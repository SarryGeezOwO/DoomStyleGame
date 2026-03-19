#include "resource_manager.hpp"
#include "util/utility.hpp"
#include "util/log.hpp"
#include <fstream>
#include <cstring>

namespace fs =  std::filesystem;

/**
 * Implementation notes:
 *  - The watcher thread only detects file modification and enqueues paths to
 *    m_reload_queue. Actual reloads are performed by the main thread via
 *    process_pending_reload() to ensure resource-specific context safety
 *    (e.g., OpenGL calls happen on the main thread).
 *
 *  - Access to m_resource_map and m_reload_queue is synchronized using a
 *    std::shared_mutex. Readers acquire a shared_lock; writers use unique_lock.
 */
Geez::ResourceManager::ResourceManager()
{;
    m_loaders.push_back(std::make_unique<ShaderLoader>());
    m_loaders.push_back(std::make_unique<TextureLoader>());
    m_loaders.push_back(std::make_unique<AudioLoader>());
    m_loaders.push_back(std::make_unique<GeezMapLoader>());
}

Geez::ResourceManager::~ResourceManager()
{
    stop_watching();
    unload_all();
    GZ_LOG(GZ_DEBUG, "Resource manager quit");
}

// ======================================================= //
// ================ HOT RELOADING ======================== //

void Geez::ResourceManager::set_watch_interval(U32 milliseconds)
{
    m_watch_interval = SDL_clamp(milliseconds, 500, 6000);
}

/**
 * @brief Scan loaded resources for file time changes and enqueue changed files.
 *
 * This method is called repeatedly by the watcher thread. It uses a shared lock
 * while iterating and upgrades to a unique lock only when a change is detected
 * and a path must be pushed onto m_reload_queue.
 */
void Geez::ResourceManager::watch_resources()
{
    std::shared_lock lock(m_resource_mutex);
    for (auto& resource : m_resource_map) {
        for (auto& contributer : resource.second->m_contributers) {
            fs::file_time_type last = contributer.last_modified;
            fs::file_time_type current = fs::last_write_time(contributer.path);

            if (last == current) {
                continue;
            }

            lock.unlock();
            {
                std::unique_lock write_lock(m_resource_mutex);
                contributer.last_modified = current;
                m_reload_queue.push(contributer.path);
            }
            lock.lock();
        }
    }
}

/**
 * @brief Start the background watcher thread if not already running.
 *
 * The thread loops while m_is_watching is true and sleeps for m_watch_interval
 * between scan passes.
 */
void Geez::ResourceManager::start_watching()
{   
    if (m_resource_watcher) return;
    m_is_watching = true;

    m_resource_watcher = std::make_unique<std::thread>([this]() {
        while (m_is_watching) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_watch_interval));
            watch_resources();
        }
    });
}

void Geez::ResourceManager::stop_watching()
{
    m_is_watching = false;
    if (m_resource_watcher && m_resource_watcher->joinable()) {
        m_resource_watcher->join();
        m_resource_watcher.reset();
        GZ_LOG(GZ_DEBUG, "Resource watcher thread quit");
    }
}

/**
 * @brief Move pending reloads out of the internal queue and perform reloads.
 *
 * This function moves the internal queue to a local copy under lock and then
 * calls load(path, true) for each queued entry. Designed to be run on the main
 * thread to perform actual resource updates.
 */
void Geez::ResourceManager::process_pending_reload()
{
    if (m_reload_queue.empty()) return;
    m_resource_mutex.lock();
    std::queue<fs::path> copy = std::move(m_reload_queue);
    m_resource_mutex.unlock();

    while (!copy.empty()) {
        load(copy.front(), true);
        copy.pop();
    }
}

// ======================================================= //
// ======================================================= //

void Geez::ResourceManager::load(const std::filesystem::path &file, bool overwrite)
{
    std::string name = filename_no_ext(file);
    std::string ext = to_lower(file.extension().string());

    std::unique_ptr<IResource> resource;
    const char* chosen_loader = "";

    bool no_loader = true;
    for (std::unique_ptr<IResourceLoader>& loader: m_loaders) {
        bool valid = loader->valid_ext(ext);
        no_loader = min(no_loader, !valid);

        if (loader->skip(file) && valid && !overwrite) return;
        if (!loader->can_load(file)) continue;
        
        resource = loader->load(file);
        chosen_loader = loader->name();
        break;
    }

    if (no_loader) {
        GZ_LOG(GZ_FAIL, "Resource [%s] not loaded, no loader capable of type [%s]", 
            name.c_str(), ext.c_str());
        return;
    }

    if (resource == nullptr) {
        GZ_LOG(GZ_FAIL, "Resource [%s] couldn't be loaded by [%s]", name.c_str(), chosen_loader);
        return;
    }

    if (find(name) && !overwrite) {
        GZ_LOG(GZ_WARNING, "Possible resource conflict: [%s]. File Ignored [%s]",
            name.c_str(), file.filename().string().c_str());
        return;
    }

    std::unique_lock lock(m_resource_mutex);
    resource->m_resource_id   = name;

    m_resource_map[name] = std::move(resource);
    GZ_LOG(GZ_SUCCESS, "Resource %s [%s] [%s]", 
        (overwrite ? "Overwritten" : "Loaded"),
        chosen_loader, name.c_str());
    m_loaded_count++;
}

void Geez::ResourceManager::load_all()
{
    const fs::path asset_path{GetResourcePath()};
    GZ_LOG(GZ_DEBUG, "Loading all discoverable resources at [%s]", asset_path.string().c_str());
    
    Internal::Logger::increment_tab_level(1);
    for (auto const& entry : fs::recursive_directory_iterator{asset_path}) 
    {
        if (entry.is_directory()) 
            continue;
        load(entry.path(), false);
    }
    Internal::Logger::decrement_tab_level(1);
}

void Geez::ResourceManager::unload(const ResourceID &id)
{
    std::unique_lock lock(m_resource_mutex);
    m_resource_map.erase(id);
    m_loaded_count--;
}

void Geez::ResourceManager::unload_all()
{
    GZ_LOG(GZ_DEBUG, "Unloading [%d] resources", m_loaded_count);
    Internal::Logger::increment_tab_level(1);
    std::unique_lock lock(m_resource_mutex);
    m_loaders.clear();
    m_resource_map.clear();
    m_loaded_count = 0;
    Internal::Logger::decrement_tab_level(1);
}

bool Geez::ResourceManager::find(const ResourceID &id)
{
    std::shared_lock lock(m_resource_mutex);
    return m_resource_map.find(id) != m_resource_map.end();
}
