#include "window.hpp"
#include "util/log.hpp"

Geez::Window::Window(const std::string &title, const glm::ivec2& size)
    : m_size(size)
{   
    // Setup OpenGL Context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    m_window = SDL_CreateWindow(title.c_str(), 
        size.x, size.y, SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (m_window == nullptr) {
        GZ_LOG(GZ_FATAL, "Window Creation...");
        m_hasError = true;
        return;
    }

    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == nullptr) {
        GZ_LOG(GZ_FATAL, "OpenGL Context Creation...");
        m_hasError = true;
        return;
    }

    glewInit();
    glViewport(0, 0, size.x, size.y);
    const U8* version            = glGetString(GL_VERSION);
    const U8* renderer           = glGetString(GL_RENDERER);
    const U8* vendor             = glGetString(GL_VENDOR);
    const U8* shading_lang_ver   = glGetString(GL_SHADING_LANGUAGE_VERSION);

    SDL_Log("========================================================");
    SDL_Log("SDL Base path:   %s", SDL_GetBasePath());
    SDL_Log("OpenGL Version:  %s", version);
    SDL_Log("Renderer:        %s", renderer);
    SDL_Log("Vendor:          %s", vendor);
    SDL_Log("GLSL Version:    %s", shading_lang_ver);
    SDL_Log("========================================================");
}

Geez::Window::~Window()
{
    SDL_DestroyWindow(m_window);
    SDL_GL_DestroyContext(m_context);
    m_window = nullptr;
    m_context = nullptr;
    GZ_LOG(GZ_DEBUG, "Window quit");
}

void Geez::Window::resize(const glm::ivec2 &size)
{
    m_size = size;
    SDL_SetWindowSize(m_window, size.x, size.y);
    glViewport(0, 0, size.x, size.y);
}

void Geez::Window::rename(const std::string &title)
{
    SDL_SetWindowTitle(m_window, title.c_str());
}

void Geez::Window::toggle_cursor_visible()
{
    set_cursor_visible(!m_cursor_visible);
}

void Geez::Window::set_cursor_visible(bool b)
{    
    if (b) {
        SDL_SetWindowMouseRect(m_window, NULL);
        SDL_WarpMouseInWindow(m_window, m_size.x/2.0f, m_size.y/2.0f);
        SDL_ShowCursor();
    }
    else {
        SDL_Rect rect = { 
            (m_size.x/2) - 20, 
            (m_size.y/2) - 20, 
            40, 40 
        };
        SDL_SetWindowMouseRect(m_window, &rect);
        SDL_HideCursor();
    }
    
    SDL_SetWindowRelativeMouseMode(m_window, !b);
    m_cursor_visible = b;
}
