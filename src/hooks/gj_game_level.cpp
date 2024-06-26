#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/utils/string.hpp>

#include <filesystem>

#include "../managers/nong_manager.hpp"
#include "../types/song_info.hpp"

using namespace geode::prelude;

class $modify(GJGameLevel) {
    gd::string getAudioFileName() {
        jukebox::NongManager::get()->m_currentlyPreparingNong = std::nullopt;
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
        jukebox::NongManager::get()->m_currentlyPreparingNong = value;
        #ifdef GEODE_IS_WINDOWS
        return geode::utils::string::wideToUtf8(value.path.c_str());
        #else
        return value.path.string();
        #endif
    }
};