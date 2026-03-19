#ifndef GZ_INPUT_HPP
#define GZ_INPUT_HPP

#include <SDL3/SDL_events.h>
#include <unordered_map>
#include <glm/glm.hpp>

/**
 * @brief Input handling system based on SDL3 events
 * 
 * Provides a simplified interface for:
 * - Keyboard input detection
 * - Mouse button states
 * - Window resize events
 * - Mouse movement tracking
 */

namespace Geez
{
    using Input_WindowResizeCallback = void (*)(const glm::ivec2& newsize);
    using Input_MouseMoveCallback = void(*)(const glm::vec2& screen, const glm::vec2& relative);

    /**
     * @brief Input state types for keys and buttons
     * 
     * GZ_HOLD    - Button/key is currently being held down
     * GZ_TAP     - Button/key was just pressed this frame
     * GZ_RELEASE - Button/key was just released this frame
     */
    enum InputState {
        GZ_HOLD,
        GZ_TAP,
        GZ_RELEASE
    };

    /**
     * @brief Tracks the previous and current state of a key
     */
    struct KeyRecord
    {
        bool prev;     ///< State in previous frame
        bool current;  ///< State in current frame
    };

    /**
     * @brief Main input handling class
     * 
     * Processes SDL events and maintains the state of:
     * - Keyboard keys
     * - Mouse buttons
     * - Window events
     * - Mouse movement
     */
    struct Input
    {
    private:
        SDL_Event m_event;
        std::unordered_map<SDL_Keycode, KeyRecord> m_keymap;
        Input_WindowResizeCallback on_window_resize = nullptr;
        Input_MouseMoveCallback on_mouse_move = nullptr;
        glm::vec2 m_mouse_screen;
        glm::vec2 m_mouse_relative;

        bool prevMouseLeft = false;   ///< Left mouse button state last frame
        bool currMouseLeft = false;   ///< Left mouse button current state
        bool prevMouseRight = false;  ///< Right mouse button state last frame
        bool currMouseRight = false;  ///< Right mouse button current state
        
    public:
        /**
         * @brief Process all pending SDL events
         * @return false if quit event received, true otherwise
         */
        bool poll_events();

        /**
         * @brief Check the state of a keyboard key
         * @param key SDL keycode to check
         * @param state Desired state to check for (HOLD/TAP/RELEASE)
         * @return true if key is in specified state
         */
        bool check_key(SDL_Keycode key, InputState state) const;

        /**
         * @brief Check left mouse button state
         * @param state Desired state to check for (HOLD/TAP/RELEASE)
         * @return true if button is in specified state
         */
        bool check_mouse_left(InputState state) const;

        /**
         * @brief Check right mouse button state
         * @param state Desired state to check for (HOLD/TAP/RELEASE)
         * @return true if button is in specified state
         */
        bool check_mouse_right(InputState state) const;

        /**
         * @brief Register callback for window resize events
         * @param cb Function taking new window size as vec2
         */
        inline void AddWindowResizeCallback(Input_WindowResizeCallback cb) {
            on_window_resize = cb;
        }

        /**
         * @brief Register callback for mouse movement events
         * @param cb Function taking the new mouse position [Screen, Relative]
         */
        inline void AddMouseMoveCallback(Input_MouseMoveCallback cb) {
            on_mouse_move = cb;
        }

        inline glm::vec2 mouse_screen() const { return m_mouse_screen; }
        inline glm::vec2 mouse_relative() const { return m_mouse_relative; }
    };

}

#endif