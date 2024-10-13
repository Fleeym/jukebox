#include "Geode/binding/GJGameLevel.hpp"
#include "Geode/modify/GJGameLevel.hpp"  // IWYU pragma: keep
#include "Geode/modify/Modify.hpp"
#include "Geode/utils/string.hpp"

#include <filesystem>
#include <optional>

#include "../managers/nong_manager.hpp"

using namespace geode::prelude;
using namespace jukebox;

class $modify(GJGameLevel) {
    gd::string getAudioFileName() {
        NongManager::get()->m_currentlyPreparingNong = std::nullopt;
        if (m_songID != 0) {
            return GJGameLevel::getAudioFileName();
        }
        int id = (-m_audioTrack) - 1;
        std::optional<Nongs*> res = NongManager::get()->getNongs(id);
        if (!res.has_value()) {
            return GJGameLevel::getAudioFileName();
        }
        Song* active = res.value()->active();
        if (!std::filesystem::exists(active->path().value())) {
            return GJGameLevel::getAudioFileName();
        }
        jukebox::NongManager::get()->m_currentlyPreparingNong = res.value();
#ifdef GEODE_IS_WINDOWS
        return geode::utils::string::wideToUtf8(active->path().value().c_str());
#else
        return active.path().value().string();
#endif
    }
};
