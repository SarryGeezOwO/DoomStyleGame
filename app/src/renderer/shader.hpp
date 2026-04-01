#ifndef GZ_SHADER_HPP
#define GZ_SHADER_HPP

#include "util/common_types.hpp"
#include "util/error.hpp"
#include "resource/resource.hpp"

#include <string>
#include <GL/GLew.h>
#include <GLm/GLm.hpp>
#include <unordered_map>
using namespace glm;

namespace Geez
{
    struct ShaderSource
    {
        std::string fragment;
        std::string vertex;
    };

    // ==================== Shader ============================

    /*
        TODO: Shader initializes every uniforms found in source
              by calling get_uniform() each
    */

    struct Shader : IResource
    {
    private:
        U32 m_render_id;
        ShaderSource m_source;
        std::unordered_map<std::string, U32> m_uniforms;  // <- name, location

    public:
        Shader(const ShaderSource& source);
        ~Shader();

        template <typename T>
        Shader& set_uniform(const std::string& name, const T& value) {
            // unknown type
            static_assert(false, "uniform type not supported...");
            return *this;
        }

        void bind();
        void unbind();
        bool has_uniform(const std::string& name) const;
        U32 get_uniform(const std::string& name);

        inline U32 renderID() const noexcept { return m_render_id; }
        inline ShaderSource source() const noexcept { return m_source; }
    };

    // ==================== set_uniform ============================

    template<>
    inline Shader& Shader::set_uniform<bool>(const std::string& name, const bool& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform1f(location, value));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<float>(const std::string& name, const F32& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform1f(location, value));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<int>(const std::string& name, const I32& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform1i(location, value));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<unsigned int>(const std::string& name, const U32& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform1ui(location, value));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<vec2>(const std::string& name, const vec2& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform2f(location, value.x, value.y));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<ivec2>(const std::string& name, const ivec2& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform2i(location, value.x, value.y));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<vec3>(const std::string& name, const vec3& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform3f(location, value.x, value.y, value.z));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<ivec3>(const std::string& name, const ivec3& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform3i(location, value.x, value.y, value.z));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<vec4>(const std::string& name, const vec4& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform4f(location, value.x, value.y, value.z, value.w));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<ivec4>(const std::string& name, const ivec4& value)
    {
        U32 location = get_uniform(name);
        GL(glUniform4i(location, value.x, value.y, value.z, value.w));
        return *this;
    }

    template<>
    inline Shader& Shader::set_uniform<mat4>(const std::string& name, const mat4& value)
    {
        U32 location = get_uniform(name);
        GL(glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]));
        return *this;
    }

}

#endif
