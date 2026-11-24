#include "SoundManager.hpp"

SoundManager::SoundManager(){
    // load background music
    Load< Sound::Sample > hospital_music(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("Hospital_Theme.wav"));
    });

    Load< Sound::Sample > soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("soul_capture.wav"));
    });

    // load sfx
    Load< Sound::Sample > hit_sfx(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("hit.wav"));
    });

    Load< Sound::Sample > miss_sfx(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("miss.wav"));
    });

    Load< Sound::Sample > slice_sfx(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("scythe.wav"));
    });
}

SoundManager::~SoundManager(){
}

void SoundManager::play_music(Sound::Sample const &sample) {
	current_bg_music->stop(1.0f);
	current_bg_music = Sound::loop(sample, 0.0f, 0.0f);
	current_bg_music->set_volume(1.0f, 1.0f);
}

void SoundManager::play_sfx(Sound::Sample const &sample) {
    std::shared_ptr< Sound::PlayingSample > slot;
    Sound::play(sample, 1.0, 0.0);
}