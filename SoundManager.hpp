#pragma once

#include "Sound.hpp"
#include "Load.hpp"
#include "data_path.hpp"

// Redf https://www.embeddedrelated.com/showarticle/1598.php
struct SoundManager{
    // current bg music slot
    static std::shared_ptr< Sound::PlayingSample > current_bg_music;

    // all background music
    static Load <Sound::Sample> hospital_music;
    static Load <Sound::Sample> soul_capture;

    // all sfx
    static Load <Sound::Sample> hit_sfx;
    static Load <Sound::Sample> miss_sfx;
    static Load <Sound::Sample> slash_sfx;

    // swap the background track, fading the old one out and the new one in:
    static void play_music(Sound::Sample const &sample);

    // play sound on top of all others
    static void play_sfx(Sound::Sample const &sample);
};