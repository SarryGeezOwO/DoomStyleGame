#include "texture.hpp"
#include "util/error.hpp"
#include "util/utility.hpp"
#include <stb_image.h>

Geez::Texture::Texture(const std::string& file)
{   
    // Load
    stbi_set_flip_vertically_on_load(true);
    I32 width, height, bpp;
    U8* img = stbi_load(file.c_str(), &width, &height, &bpp, 0);
    if (img == nullptr) {
        GZ_LOG(GZ_FAIL, "Failed to load image [%s]", file.c_str());
        return;
    }

    GL(glGenTextures(1, &m_render_id));
    GL(glBindTexture(GL_TEXTURE_2D, m_render_id));

    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    m_resolution = glm::ivec2(width, height);
    m_format = get_texture_format(bpp);
    m_channels = bpp;

    GL(glTexImage2D(
        GL_TEXTURE_2D, 0, 
        m_format.internal_format,
        width, height, 0,
        m_format.base_format,
        GL_UNSIGNED_BYTE, img
    ));

    GL(glGenerateMipmap(GL_TEXTURE_2D));
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    if (img) stbi_image_free(img);
}

Geez::Texture::~Texture()
{
    GL(glDeleteTextures(1, &m_render_id));
    GZ_LOG(GZ_SUCCESS, "Texture [%s] Destroyed", m_resource_id.c_str());
}

void Geez::Texture::bind(U32 slot)
{
    GL(glActiveTexture(GL_TEXTURE0 + slot));
    GL(glBindTexture(GL_TEXTURE_2D, m_render_id));
}

void Geez::Texture::unbind()
{
    GL(glBindTexture(GL_TEXTURE_2D, 0));
}
