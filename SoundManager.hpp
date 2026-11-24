#pragma once

#include "Sound.hpp"
#include "Load.hpp"
#include "data_path.hpp"

struct SoundManager{
    SoundManager();
    ~SoundManager();

    // current bg music slot
    static std::shared_ptr< Sound::PlayingSample > current_bg_music;

    // all background music
    Load <Sound::Sample> hospital_music;
    Load <Sound::Sample> soul_capture;

    // all sfx
    Load <Sound::Sample> hit_sfx;
    Load <Sound::Sample> miss_sfx;
    Load <Sound::Sample> slash_sfx;

    // swap the background track, fading the old one out and the new one in:
    void play_music(Sound::Sample const &sample);

    // play sound on top of all others
    void play_sfx(Sound::Sample const &sample);
};