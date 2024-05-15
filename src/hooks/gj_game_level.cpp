#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>

#include <filesystem>

#include "../managers/nong_manager.hpp"
#include "../types/song_info.hpp"

using namespace geode::prelude;

class $modify(GJGameLevel) {
    gd::string getAudioFileName() {
        if (m_songID != 0) {
            return GJGameLevel::getAudioFileName();
        }
        int id = (-m_audioTrack) - 1;
        auto active = jukebox::NongManager::get()->getActiveNong(id);
        if (!active.has_value()) {
            return GJGameLevel::getAudioFileName();
        }
        jukebox::SongInfo value = active.value();
        if (!std::filesystem::exists(value.path)) {
            return GJGameLevel::getAudioFileName();
        }
        return value.path.string();
    }
};