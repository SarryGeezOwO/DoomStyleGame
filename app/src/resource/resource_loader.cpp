#include "resource_loader.hpp"
#include "util/utility.hpp"
#include "util/log.hpp"
#include <fstream>
#include <algorithm>

// ================================ TEXTURE ================================ //

bool Geez::TextureLoader::valid_ext(const std::string &extension) const
{
    return extension == ".png" 
        || extension == ".jpg"
        || extension == ".jpeg";
}

std::unique_ptr<Geez::IResource> Geez::TextureLoader::load(const std::filesystem::path &file)
{
    auto ptr = std::make_unique<Texture>(file.string());
    ptr->add_contributer({file, std::filesystem::last_write_time(file)});
    return ptr;
}

// ================================ SHADER ================================ //

bool Geez::ShaderLoader::valid_ext(const std::string &extension) const
{
    return extension == ".glsl";
}

bool Geez::ShaderLoader::skip(const std::filesystem::path &file) const
{
    // If loaded already, skip
    return std::find(loaded.begin(), loaded.end(), filename_no_ext(file)) != loaded.end();
}

std::unique_ptr<Geez::IResource> Geez::ShaderLoader::load(const std::filesystem::path &file)
{
    std::string name = filename_no_ext(file);
    std::string file_vs = GetShaderPath(name)+".vs.glsl"; // Haha arbitrary GLSL extension, iz fine
    std::string file_fs = GetShaderPath(name)+".fs.glsl";
    ShaderSource source;

    std::ifstream vs(file_vs, std::ios::binary);
    std::ifstream fs(file_fs, std::ios::binary);
    if (vs && fs) {
        loaded.push_back(name);
        source = {
            std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>()),
            std::string((std::istreambuf_iterator<char>(vs)), std::istreambuf_iterator<char>())
        };

        auto ptr = std::make_unique<Shader>(source);
        ptr->add_contributer({file_vs, std::filesystem::last_write_time(file_vs)});
        ptr->add_contributer({file_fs, std::filesystem::last_write_time(file_fs)});
        return ptr;
    } 

    bool vs_missing = !vs.is_open();
    bool fs_missing = !fs.is_open();

    if (vs_missing && fs_missing) {
        GZ_LOG(GZ_FAIL, "Unable to open both vertex and fragment shaders for {%s}", name.c_str());
    } else if (vs_missing) {
        GZ_LOG(GZ_FAIL, "Unable to open vertex shader for {%s}", name.c_str());
    } else if (fs_missing) {
        GZ_LOG(GZ_FAIL, "Unable to open fragment shader for {%s}", name.c_str());
    }
    return nullptr;
}

// ================================ AUDIO ================================ //

bool Geez::AudioLoader::valid_ext(const std::string &extension) const
{
    return extension == ".mp3"
        || extension == ".wav";
}

std::unique_ptr<Geez::IResource> Geez::AudioLoader::load(const std::filesystem::path &file)
{
    auto ptr = std::make_unique<Audio>(file.string());
    ptr->add_contributer({file, std::filesystem::last_write_time(file)});
    return ptr;
}

// ================================ GMP ================================ //

bool Geez::GeezMapLoader::valid_ext(const std::string &extension) const
{
    return extension == ".gmp";
}

std::unique_ptr<Geez::IResource> Geez::GeezMapLoader::load(const std::filesystem::path &file)
{
    auto ptr = std::make_unique<GeezMapData>(file.string());
    ptr->make_mesh();
    
    ptr->add_contributer({file, std::filesystem::last_write_time(file)});
    return ptr;
}
