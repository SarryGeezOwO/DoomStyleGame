#include "input.hpp"
#include "util/log.hpp"
#include "imgui/imgui_impl_sdl3.h"
using namespace glm;

bool Geez::Input::poll_events()
{
    // Let past know
    for (auto& [key, record] : m_keymap) {
        record.prev = record.current;
    }

    prevMouseLeft = currMouseLeft;
    prevMouseRight = currMouseRight;

    while(SDL_PollEvent(&m_event)) {
        ImGui_ImplSDL3_ProcessEvent(&m_event);
        
        switch(m_event.type) {
            case SDL_EVENT_QUIT:
                return false;

            case SDL_EVENT_WINDOW_RESIZED:
                {
                    SDL_Window* win = SDL_GetWindowFromID(m_event.window.windowID);
                    int x, y;
                    SDL_GetWindowSize(win, &x, &y);
                    if (on_window_resize) {
                        on_window_resize(ivec2(x, y));
                    }
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    SDL_MouseButtonEvent mouseButtonEvent = m_event.button;
                    if (mouseButtonEvent.button == 1)  // Left button
                        currMouseLeft = true;
                    if (mouseButtonEvent.button == 3)  // right button
                        currMouseRight = true;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    SDL_MouseButtonEvent mouseButtonEvent = m_event.button;
                    if (mouseButtonEvent.button == 1)  // Left button
                        currMouseLeft = false;
                    if (mouseButtonEvent.button == 3)  // right button
                        currMouseRight = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                {
                    m_mouse_screen = vec2(m_event.motion.x, m_event.motion.y);
                    m_mouse_relative = vec2(m_event.motion.xrel, m_event.motion.yrel);
                }
                if (on_mouse_move) 
                    on_mouse_move(m_mouse_screen, m_mouse_relative);
                break;

            case SDL_EVENT_KEY_DOWN:
                m_keymap[m_event.key.key].current = true;
                break;

            case SDL_EVENT_KEY_UP:
                m_keymap[m_event.key.key].current = false;
                break;

            default: break;
        }
    }
    return true;
}

bool Geez::Input::check_key(SDL_Keycode key, InputState state) const
{   
    if (m_keymap.find(key) == m_keymap.end()) {
        // Key not mapped yet
        return false;
    } 

    KeyRecord rec = m_keymap.at(key);
    switch(state) {
        case GZ_HOLD:    return rec.current;
        case GZ_TAP:     return !rec.prev &&  rec.current;
        case GZ_RELEASE: return  rec.prev && !rec.current;
    }
    return false;
}

bool Geez::Input::check_mouse_left(InputState state) const
{
    switch(state) {
        case GZ_HOLD:    return currMouseLeft;
        case GZ_TAP:     return !prevMouseLeft &&  currMouseLeft;
        case GZ_RELEASE: return  prevMouseLeft && !currMouseLeft;
    }
    return false;
}

bool Geez::Input::check_mouse_right(InputState state) const
{
    switch(state) {
        case GZ_HOLD:    return currMouseRight;
        case GZ_TAP:     return !prevMouseRight &&  currMouseRight;
        case GZ_RELEASE: return  prevMouseRight && !currMouseRight;
    }
    return false;
}
