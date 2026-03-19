#include "audio.hpp"
#include "util/log.hpp"
#include <SDL3/SDL_properties.h>

// Haha, I love coding 💔💔🌹
static MIX_Mixer* speaker = nullptr;

void Geez::GZ_Audio_Init()
{   
    if (!MIX_Init()) {
        GZ_LOG(GZ_FATAL, 1, "Failed Initialization SDL3_Mixer\n%s", SDL_GetError());
        return;
    }

    speaker = MIX_CreateMixerDevice( SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!speaker) {
        GZ_LOG(GZ_FATAL, "Failed to create Audio Mixer.");
    }
}

void Geez::GZ_Audio_Quit()
{
    MIX_DestroyMixer(speaker);
    MIX_Quit();
    GZ_LOG(GZ_DEBUG, "Audio Subsystem quit");
}

bool isInit() {
    return speaker != nullptr;
}

Geez::Audio::Audio(const std::string& file)
{
    if (!isInit()) return;

    m_data = MIX_LoadAudio(speaker, file.c_str(), true);
    if (!m_data) {
        GZ_LOG(GZ_FATAL, "Failed to load Audio [%s].", file);
        MIX_DestroyAudio(m_data);
    }
}

Geez::Audio::~Audio()
{
    MIX_DestroyAudio(m_data);
    GZ_LOG(GZ_SUCCESS, "Audio [%s] Destroyed", m_resource_id.c_str());
}

void Geez::Audio::play()
{
    if (!isInit()) return;
    MIX_PlayAudio(speaker, m_data);
}

Geez::AudioPlayer::AudioPlayer(U32 mixer_channels)
{
    if (!isInit()) return;
    for (U32 i = 0; i < mixer_channels; i++) {
        auto channel = MIX_CreateTrack(speaker);
        if (channel == nullptr) {
            GZ_LOG(GZ_FAIL, "Failed to create audio mixer channel [%d]", i);
            MIX_DestroyTrack(channel);
            continue;
        }

        m_channels.push_back(std::move(channel));
        m_channel_count++;
    }
    GZ_LOG(GZ_SUCCESS, "Created [%d] Audio Mixer Channel", m_channel_count);
}

Geez::AudioPlayer::~AudioPlayer()
{
    for (MIX_Track*& channel : m_channels ) {
        MIX_DestroyTrack(channel);
    }
    GZ_LOG(GZ_DEBUG, "Destroyed [%d] Audio Mixer Channels", m_channel_count);
}

void Geez::AudioPlayer::set_max_volume_limit(F32 limit)
{
    m_max_volume = limit;
}

void Geez::AudioPlayer::set_master_volume(F32 gain)
{
    MIX_SetMasterGain(speaker, SDL_clamp(gain, 0.0f, m_max_volume));
}

void Geez::AudioPlayer::play(Audio *audio)
{
    if (audio == nullptr) {
        GZ_LOG(GZ_FAIL, "[AUDIO] attempting to play a nullptr.");
        return;
    }

    for (U32 i = 0; i < m_channel_count; i++) {
        MIX_Track*& channel = m_channels[i];
        if (MIX_GetTrackRemaining(channel) > 0) 
            continue;

        MIX_SetTrackAudio(channel, audio->m_data);
        MIX_SetTrackGain(channel, SDL_clamp(audio->prop_gain, 0.0f, m_max_volume));

        // Construct Options
        SDL_PropertiesID properties = SDL_CreateProperties();
        SDL_SetNumberProperty(properties, 
            MIX_PROP_PLAY_LOOPS_NUMBER, audio->prop_repeat); // Loop
        SDL_SetNumberProperty(properties, 
            MIX_PROP_PLAY_START_MILLISECOND_NUMBER, audio->prop_offset); // Offset (MS)
        SDL_SetNumberProperty(properties, 
            MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, audio->prop_fade_in); // Fade in (MS)
        
        MIX_PlayTrack(channel, properties);
        GZ_LOG(GZ_OK, "Track [%d] is playing: %s", i, audio->m_resource_id.c_str());
        return;
    }
    
    // No Channel
    GZ_LOG(GZ_FAIL, "No Audio Mixer channel available to play [%s]", audio->m_resource_id.c_str());
}

