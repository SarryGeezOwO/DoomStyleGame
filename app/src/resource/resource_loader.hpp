#ifndef GZ_RESOURCE_LOADER_HPP
#define GZ_RESOURCE_LOADER_HPP

#include "core/audio.hpp"
#include "gmp/gmp.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "resource.hpp"


#include <filesystem>
#include <memory>
#include <vector>

namespace Geez
{
    struct IResourceLoader
    {
    protected:
        std::vector<std::string> loaded; // Optional shenanigas, idk man
    public:
        virtual ~IResourceLoader() = default;

        // Required overrides
        virtual bool valid_ext(const std::string& extension) const = 0;
        virtual std::unique_ptr<IResource> load(const std::filesystem::path& file) = 0;
        virtual const char* name() const = 0;

        // override for something more specific
        virtual bool can_load(const std::filesystem::path& file) const {
            return valid_ext(file.extension().string());
        }

        // Optional, for resources that loads simultaneous files
        virtual bool skip(const std::filesystem::path& file) const { 
            return false; 
        };
    };

    // ================================ TEXTURE =============================================
    
    struct TextureLoader : IResourceLoader
    {
        bool valid_ext(const std::string& extension) const override;
        std::unique_ptr<IResource> load(const std::filesystem::path& file) override;
        inline const char* name() const override { return "Texture"; }
    };

    // ================================ SHADER =============================================

    struct ShaderLoader : IResourceLoader
    {
        bool valid_ext(const std::string& extension) const override;
        bool skip(const std::filesystem::path& file) const override;
        std::unique_ptr<IResource> load(const std::filesystem::path& file) override;
        inline const char* name() const override { return "Shader"; }
    };

    // ================================ AUDIO =============================================

    struct AudioLoader : IResourceLoader
    {
        bool valid_ext(const std::string& extension) const override;
        std::unique_ptr<IResource> load(const std::filesystem::path& file) override;
        inline const char* name() const override { return "Audio"; }
    };

    // ================================ GMP =============================================

    struct GeezMapLoader : IResourceLoader
    {
        bool valid_ext(const std::string& extension) const override;
        std::unique_ptr<IResource> load(const std::filesystem::path& file) override;
        inline const char* name() const override { return "GeezMap"; }
    };
}

#endif