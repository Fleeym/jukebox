#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/utils/string.hpp>

#include <filesystem>

#include "../managers/nong_manager.hpp"

using namespace geode::prelude;

class $modify(GJGameLevel) {
    gd::string getAudioFileName() {
        jukebox::NongManager::get()->m_currentlyPreparingNong = std::nullopt;
        if (m_songID != 0) {
            return GJGameLevel::getAudioFileName();
        }
        int id = (-m_audioTrack) - 1;
        auto res = jukebox::NongManager::get()->getNongs(id);
        if (!res.has_value()) {
            return GJGameLevel::getAudioFileName();
        }
        auto active = res.value()->activeNong();
        if (!std::filesystem::exists(active.path().value())) {
            return GJGameLevel::getAudioFileName();
        }
        jukebox::NongManager::get()->m_currentlyPreparingNong = res.value();
        #ifdef GEODE_IS_WINDOWS
        return geode::utils::string::wideToUtf8(active.path().value().c_str());
        #else
        return active.path().value().string();
        #endif
    }
};
