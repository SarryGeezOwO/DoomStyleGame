#ifndef GZ_RESOURCE_MANAGER
#define GZ_RESOURCE_MANAGER

#include "util/common_types.hpp"
#include "resource.hpp"
#include "resource_loader.hpp"

#include "core/audio.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"

#include "gmp/gmp.hpp"

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <queue>
#include <mutex>
#include <type_traits>

namespace Geez
{
        /**
     * @brief Central manager for game/application assets.
     *
     * The ResourceManager is responsible for discovering, loading, storing,
     * and hot-reloading resources such as Shaders, Textures and Audio.
     *
     * Responsibilities:
     *  - Ensure a single copy of each resource is loaded (by ResourceID).
     *  - Provide typed accessors to loaded resources.
     *  - Support bulk loading from the project's asset directory.
     *  - Provide a background watcher thread that detects file changes and
     *    queues resources for reload.
     *
     * Thread-safety:
     *  - Public read operations (e.g., get/find) acquire a shared lock.
     *  - Mutating operations (load, free, free_all, process_pending_reload)
     *    acquire a unique lock.
     *  - The internal watcher thread uses shared/unique locks when scanning
     *    and enqueuing reloads.
     *
     * Usage:
     *  - Call load_all() at startup to populate the manager.
     *  - Optionally call start_watching() to enable hot-reload; poll
     *    process_pending_reload() from the main thread to perform reloads.
     *
     * Note:
     *  - Resources are indexed by ResourceID (string alias).
     *  - ResourceID is case sensitive.
     */
    struct ResourceManager
    {
    private:
        std::unordered_map<ResourceID, std::unique_ptr<IResource>> m_resource_map;
        std::vector<std::unique_ptr<IResourceLoader>> m_loaders;
        std::queue<std::filesystem::path> m_reload_queue;

        std::unique_ptr<std::thread> m_resource_watcher;
        std::shared_mutex m_resource_mutex;
        std::atomic<U32> m_watch_interval = 3000; // MS
        std::atomic<bool> m_is_watching;
        U32 m_loaded_count = 0;

        void watch_resources();

    public:
        /** @brief Constructs an empty ResourceManager. */
        ResourceManager();

        /** @brief Cleans up and releases all resources automatically. */
        ~ResourceManager();

        /**
         * @brief Set the polling interval used by the watcher thread.
         * @param milliseconds Interval in milliseconds (clamped between 500 and 6000).
         *
         * This sets how frequently the watcher thread checks file modification times.
         */
        void set_watch_interval(U32 milliseconds);

        /**
         * @brief Starts the background watcher thread.
         *
         * Thread will scan known resources periodically and enqueue changed files
         * for reload. The actual reloads should be performed on the main thread
         * by calling process_pending_reload() (to avoid OpenGL / audio context issues
         * inside the watcher thread).
         */
        void start_watching();

        /**
         * @brief Requests the watcher thread to stop and joins it.
         *
         * This will block until the watcher thread exits. Safe to call from shutdown.
         */
        void stop_watching();

        /**
         * @brief Process queued file changes and reload corresponding resources.
         *
         * Should be called from the main thread periodically (e.g., each frame).
         * This takes ownership of the pending reload queue and calls load(..., true)
         * for each queued path.
         */
        void process_pending_reload();

        inline bool is_watching() const noexcept { return m_is_watching; }
        
        /**
         * @brief Loads a specific resource file into memory.
         *
         * This function chooses an appropriate loader based on file extension
         * and loader capability. If overwrite is true, an existing resource
         * with the same ID will be replaced.
         *
         * @param file Path to the resource file to load.
         * @param overwrite If true, replace an existing resource with the same ID.
         */
        void load(const std::filesystem::path& file, bool overwrite);

        /**
         * @brief Recursively scans the project's asset directory and attempts
         * to load every file found with a suitable loader.
         *
         * Recommended to call before enabling the watcher thread.
         */
        void load_all();

        /**
         * @brief Unload a specific resource from memory.
         * @param id The ID of the resource to remove.
         *
         * If the resource is not present, this is a no-op.
         */
        void unload(const ResourceID& id);

        /**
         * @brief Unloads all loaded resources and clears all registered loaders.
         *
         * This is used during shutdown to ensure resources are released in a
         * deterministic order.
         */
        void unload_all();

        /**
         * @brief Checks whether a resource with the given ID exists.
         * @param id The ID of the resource to check.
         * @return true if the resource exists, false otherwise.
         */
        bool find(const ResourceID& id);

        /**
         * @brief Retrieve a typed pointer to a loaded resource by ResourceID.
         *
         * This templated getter performs a thread-safe lookup (acquires a shared lock)
         * and returns a pointer to the stored IResource instance cast to the requested
         * concrete type T. If the resource is not found, nullptr is returned.
         *
         * Behavior and safety:
         *  - A compile-time check (static_assert) restricts which concrete types
         *    are permitted. Add allowed types to the static_assert list when new
         *    resource kinds are introduced.
         *  - The function uses static_cast<T*> on the stored IResource pointer. That
         *    cast is safe only if the stored object is actually of type T (or a
         *    type publicly derived from T). Use exact matching ResourceID / loader
         *    conventions to ensure the stored type matches the requested T.
         *  - Thread-safety: a std::shared_lock<std::shared_mutex> is used so multiple
         *    concurrent readers are supported while writers (load/free) must acquire
         *    exclusive locks.
         *
         * Notes:
         *  - If you need special per-type behavior (different casting rules or
         *    additional validation), provide a specialization or overload for that type.
         *
         * Example:
         *   Shader* s = resourceManager.get<Shader>("my_shader");
         *
         * @tparam T Concrete resource type to retrieve (e.g., Shader, Texture, Audio)
         * @param id ResourceID of the resource to fetch
         * @return Pointer to T on success, or nullptr if not found
         */
        template <typename T>
        T* get(const ResourceID& id) {
            static_assert(
                std::is_same_v<T, Shader>       ||
                std::is_same_v<T, Texture>      ||
                std::is_same_v<T, Audio>        ||
                std::is_same_v<T, GeezMapData>
                ,
                "ResourceManager::get<T> unknown resource type."
            );

            std::shared_lock<std::shared_mutex> lock(m_resource_mutex);
            auto it = m_resource_map.find(id);
            if (it != m_resource_map.end()) return static_cast<T*>(it->second.get());

            GZ_LOG(GZ_WARNING, "Resource [%s] doesn't exist.", id.c_str());
            return nullptr;
        }
    };
}
#endif
