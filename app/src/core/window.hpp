#ifndef GZ_WINDOW_HPP
#define GZ_WINDOW_HPP

#include "util/common_types.hpp"
#include <gl/glew.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <SDL3/SDL_opengl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace Geez
{
    struct Window
    {
    private:
        SDL_Window* m_window = nullptr;
        SDL_GLContext m_context = nullptr;
        glm::ivec2 m_size;
        bool m_hasError = false;
        bool m_cursor_visible = false;

    public:
        Window(const std::string& title, const glm::ivec2& size);
        ~Window();

        void resize(const glm::ivec2& size);
        void rename(const std::string& title);
        void toggle_cursor_visible();
        void set_cursor_visible(bool b);

        inline glm::ivec2 get_size() const { return m_size; }
        inline bool is_cursor_shown() const { return m_cursor_visible; }
        inline bool error() const { return m_hasError; }
        inline SDL_Window* handle() const { return m_window; }

        inline F32 width()  const { return static_cast<F32>(m_size[0]); }
        inline F32 height() const { return static_cast<F32>(m_size[1]); }
    };
}

#endif