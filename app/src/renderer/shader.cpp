#include "shader.hpp"
#include "util/log.hpp"
#include "util/utility.hpp"

#include <GL/glew.h>
#include <SDL3/SDL.h>

Geez::U32 compile_shader(Geez::U32 type, const char* source)
{
    GL(Geez::U32 id = glCreateShader(type));
    GL(glShaderSource(id, 1, &source, nullptr));
    GL(glCompileShader(id));

    Geez::I32 result;
    GL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE) {
        Geez::I32 len;
        GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len));

        char* msg = (char*)_alloca(len * sizeof(char));
        GL(glGetShaderInfoLog(id, len, &len, msg));
        GZ_LOG(
            Geez::GZ_FATAL, 
            "Failed to compile shader: {%s}\n%s", 
            (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), msg);
        GL(glDeleteShader(id));
        return 0;
    }
    return id;
}

Geez::Shader::Shader(const ShaderSource& source)
    : m_source(source)
{
    // Get source
    const char* vs_src = source.vertex.c_str();
    const char* fs_src = source.fragment.c_str();

    U32 vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    U32 fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);

    GL(m_render_id = glCreateProgram());
    GL(glAttachShader(m_render_id, vs));
    GL(glAttachShader(m_render_id, fs));
    GL(glLinkProgram(m_render_id));
    GL(glValidateProgram(m_render_id));

    // Check for error PROGRAN
    I32 success;
    char infoLog[512];
    GL(glGetProgramiv(m_render_id, GL_LINK_STATUS, &success));
    if (!success) {
        GL(glGetProgramInfoLog(m_render_id, 512, NULL, infoLog));
        GZ_LOG(GZ_FATAL, "Program Linking Failed.\n%s", infoLog);
    }

    // Clean up
    GL(glDeleteShader(vs));
    GL(glDeleteShader(fs));

    // Get Shader uniforms
    std::string merged_source = source.vertex + "\n" + source.fragment;
    std::istringstream stream(merged_source);
    std::string line;

    while (std::getline(stream, line)) {
        trim(line);

        if (line.substr(0, 7) != "uniform") {
            continue;
        }

        size_t start = find_str_nth_occurance(line, " ", 2)+1;
        size_t end   = find_str_nth_occurance(line, " ", 3);
        auto uniform_name = line.substr(start, end - start);

        // Trim off semicolon if it has one
        size_t last = uniform_name.length()-1;
        if (uniform_name[last] == ';') {
            uniform_name.erase(last);
        }

        GL(m_uniforms[uniform_name] = glGetUniformLocation(m_render_id, uniform_name.c_str()));
    }
}

Geez::Shader::~Shader()
{
    GL(glDeleteProgram(m_render_id));
    GZ_LOG(GZ_SUCCESS, "Shader [%s] Destroyed", m_resource_id.c_str());
}

void Geez::Shader::bind()
{
    GL(glUseProgram(m_render_id));
}

void Geez::Shader::unbind()
{
    GL(glUseProgram(0));
}

unsigned int Geez::Shader::get_uniform(const std::string& name)
{
    if (has_uniform(name)) {
        return m_uniforms.at(name);
    }
    
    // No cache yet, set one
    GL(m_uniforms[name] = glGetUniformLocation(m_render_id, name.c_str()));
    return m_uniforms[name];
}

bool Geez::Shader::has_uniform(const std::string& name) const
{
    return m_uniforms.find(name) != m_uniforms.end(); 
}
