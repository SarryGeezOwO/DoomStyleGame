#ifndef GZ_AUDIO_HPP
#define GZ_AUDIO_HPP

#include "util/common_types.hpp"
#include "resource/resource.hpp"
#include <SDL3_mixer/SDL_mixer.h>
#include <vector>
#include <memory>

/**
 * @brief Audio System based on SDL3_Mixer
 * 
 * The audio system consists of three main components:
 * 
 * 1. MIX_Audio - Raw sound data (e.g., MP3, WAV)
 *    - Represents loaded audio files in memory
 *    - Managed by the Audio class
 * 
 * 2. MIX_Track - Audio playback channel
 *    - Handles individual instances of playing sounds
 *    - Managed by the AudioPlayer class
 *    - Each track can have its own volume, offset, and effects
 * 
 * 3. MIX_Mixer - Global audio mixing engine
 *    - Combines all tracks into final output
 *    - Single instance managed by GZ_Audio_Init/Quit
 *    - Controls master volume and device settings
 */

namespace Geez {

/**
 * @brief Initialize the audio system and create the global mixer
 * @note Must be called before using any audio functionality
 */
void GZ_Audio_Init();

/**
 * @brief Cleanup and shutdown the audio system
 */
void GZ_Audio_Quit();

/**
 * @brief Represents a loaded audio resource
 * 
 * Manages the lifetime of loaded audio data and provides playback properties
 */
struct Audio : IResource {
    friend struct AudioPlayer;

private:
    MIX_Audio* m_data = nullptr;

public:
    F32 prop_gain       = 1.0f; ///< Volume multiplier [0.0 - max_volume]
    I32 prop_offset     = 0;    ///< Starting position in milliseconds
    I32 prop_fade_in    = 0;    ///< Fade-in duration in milliseconds
    I32 prop_repeat     = 0;    ///< Repeat count (-1: infinite, 0: none, n: n repeats)

    /**
     * @brief Load audio from file
     * @param file Path to audio file
     * @throws Fatal error if file loading fails
     */
    Audio(const std::string& file);
    ~Audio();

    /**
     * @brief Simple playback ignoring properties
     * @note For more control, use AudioPlayer
     */
    void play();
};

/**
 * @brief Manages multiple audio playback channels
 * 
 * Handles concurrent playback of multiple audio sources with individual
 * volume control and effects. Automatically manages channel allocation.
 */
struct AudioPlayer {
private:
    std::vector<MIX_Track*> m_channels;
    U32 m_channel_count = 0;
    F32 m_max_volume = 2.0f;

public:
    /**
     * @brief Create audio player with specified channel count
     * @param mixer_channels Number of simultaneous sounds supported
     */
    explicit AudioPlayer(U32 mixer_channels);
    ~AudioPlayer();

    /**
     * @brief Set maximum allowed volume multiplier
     * @param limit Upper boundary for volume settings
     */
    void set_max_volume_limit(F32 limit);

    /**
     * @brief Set global volume for all channels
     * @param gain Volume multiplier [0.0 - max_volume]
     */
    void set_master_volume(F32 gain);
    
    /**
     * @brief Play audio with its properties on first available channel
     * @param audio Audio resource to play
     * @note Fails silently if no channels are available
     */
    void play(Audio* audio);
};

}

#endif