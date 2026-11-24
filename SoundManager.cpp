#include "SoundManager.hpp"

Load<Sound::Sample> SoundManager::hospital_music(
    LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("Hospital_Theme.wav"));
    }
);

Load<Sound::Sample> SoundManager::soul_capture(
    LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("soul_capture.wav"));
    }
);

Load<Sound::Sample> SoundManager::hit_sfx(
    LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("hit.wav"));
    }
);

Load<Sound::Sample> SoundManager::miss_sfx(
    LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("miss.wav"));
    }
);

Load<Sound::Sample> SoundManager::slash_sfx(
    LoadTagDefault, []() -> Sound::Sample const * {
        return new Sound::Sample(data_path("scythe.wav"));
    }
);

std::shared_ptr<Sound::PlayingSample> SoundManager::current_bg_music = nullptr;

void SoundManager::play_music(Sound::Sample const &sample) {
    if (current_bg_music) {
        current_bg_music->stop(1.0f);
    }

    current_bg_music = Sound::loop(sample, 1.0f, 0.0f);
}

void SoundManager::play_sfx(Sound::Sample const &sample) {
    Sound::play(sample, 1.0f, 0.0f);
}