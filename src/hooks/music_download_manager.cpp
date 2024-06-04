#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/utils/string.hpp>

#include "../managers/nong_manager.hpp"
#include "../types/song_info.hpp"
#include "../events/get_song_info_event.hpp"

using namespace jukebox;

class $modify(MusicDownloadManager) {
	gd::string pathForSong(int id) {
        NongManager::get()->m_currentlyPreparingNong = std::nullopt;
        auto active = NongManager::get()->getActiveNong(id);
        if (!active.has_value()) {
            return MusicDownloadManager::pathForSong(id);
        }
        auto value = active.value();
		if (!fs::exists(value.path)) {
            return MusicDownloadManager::pathForSong(id);
		}
        NongManager::get()->m_currentlyPreparingNong = active;
        #ifdef GEODE_IS_WINDOWS
        return geode::utils::string::wideToUtf8(value.path.c_str());
        #else
        return value.path.string();
        #endif
	}
    void onGetSongInfoCompleted(gd::string p1, gd::string p2) {
        MusicDownloadManager::onGetSongInfoCompleted(p1, p2);
        auto songID = std::stoi(p2);
        GetSongInfoEvent(this->getSongInfoObject(songID), songID).post();
    }

    SongInfoObject* getSongInfoObject(int id) {
        auto og = MusicDownloadManager::getSongInfoObject(id);
        if (og == nullptr) {
            return og;
        }
        if (NongManager::get()->hasActions(id)) {
            return og;
        }
        auto active = NongManager::get()->getActiveNong(id);
        if (active.has_value()) {
            auto value = active.value();
            og->m_songName = value.songName;
            og->m_artistName = value.authorName;
        }
        return og;
    }
};