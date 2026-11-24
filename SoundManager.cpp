#include "SoundManager.hpp"

std::shared_ptr<Sound::PlayingSample> SoundManager::current_bg_music = nullptr;

SoundManager::SoundManager() :
    // load background music
    hospital_music(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("Hospital_Theme.wav"));
    }),

    soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("soul_capture.wav"));
    }),

    // load sfx
    hit_sfx(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("hit.wav"));
    }),

    miss_sfx(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("miss.wav"));
    }),

    slash_sfx(LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("scythe.wav"));
    })
{
}

SoundManager::~SoundManager(){
}

void SoundManager::play_music(Sound::Sample const &sample) {
	if (current_bg_music) {
        current_bg_music->stop(1.0f);
    }
	current_bg_music = Sound::loop(sample, 0.0f, 0.0f);
	current_bg_music->set_volume(1.0f, 1.0f);
}

void SoundManager::play_sfx(Sound::Sample const &sample) {
    Sound::play(sample, 1.0, 0.0);
}