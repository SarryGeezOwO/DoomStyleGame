#ifndef GZ_TEXTURE_HPP
#define GZ_TEXTURE_HPP

#include "util/common_types.hpp"
#include "resource/resource.hpp"
#include <string>
#include <glm/glm.hpp>
#include "util/log.hpp"
#include <gl/glew.h>

namespace Geez
{
    struct TextureFormat
    {
        GLenum internal_format;
        GLenum base_format;
    };

    inline TextureFormat get_texture_format(I32 bpp)
    {
        switch (bpp)
        {
            case 1: return { GL_R8,   GL_RED  };
            case 3: return { GL_RGB8, GL_RGB  };
            case 4: return { GL_RGBA8, GL_RGBA };
            default:
                GZ_LOG(GZ_FAIL, "Unsupported texture format: %d bpp", bpp);
                return { GL_RGB8, GL_RGB }; // fallback just in case
        }
    }

    struct Texture : IResource
    {
    private:
        U32 m_render_id;
        U32 m_channels;
        glm::ivec2 m_resolution;
        TextureFormat m_format;

    public:
        Texture(const std::string& file);
        ~Texture();

        void bind(U32 slot = 0);
        void unbind();

        inline U32 renderID() const noexcept { return m_render_id; }
        inline U32 channels() const noexcept { return m_channels; }
        inline glm::ivec2 resolution() const noexcept { return m_resolution; }
        inline TextureFormat format() const noexcept { return m_format; }
    };
}

#endif