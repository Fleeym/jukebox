#include "Geode/binding/SongInfoObject.hpp"
#include <Geode/modify/LevelTools.hpp>
#include <Geode/binding/LevelTools.hpp>
#include <Geode/modify/Modify.hpp>

#include "../managers/nong_manager.hpp"

using namespace geode::prelude;

class $modify(LevelTools) {
    static SongInfoObject* getSongObject(int id) {
        SongInfoObject* og = LevelTools::getSongObject(id);
        if (og == nullptr) {
            return og;
        }
        int searchId = -id - 1;
        auto active = jukebox::NongManager::get()->getActiveNong(searchId);
        if (active.has_value()) {
            auto value = active.value();
            og->m_songName = value.songName;
            og->m_artistName = value.authorName;
        }
        return og;
    }

    static gd::string getAudioTitle(int id) {
        int searchID = -id - 1;
        auto active = jukebox::NongManager::get()->getActiveNong(searchID);
        if (active.has_value()) {
            return active.value().songName;
        }
        return LevelTools::getAudioTitle(id);
    }
};
