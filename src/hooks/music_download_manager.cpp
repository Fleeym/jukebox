#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/utils/string.hpp>

#include "../managers/nong_manager.hpp"
#include "../../include/nong.hpp"
#include "../events/get_song_info_event.hpp"

using namespace jukebox;

class $modify(MusicDownloadManager) {
	gd::string pathForSong(int id) {
        NongManager::get()->m_currentlyPreparingNong = std::nullopt;
        auto nongs = NongManager::get()->getNongs(id);
        if (!nongs.has_value()) {
            return MusicDownloadManager::pathForSong(id);
        }
        auto value = nongs.value();
        auto active = value->active();
		if (!std::filesystem::exists(active->path)) {
            return MusicDownloadManager::pathForSong(id);
		}
        NongManager::get()->m_currentlyPreparingNong = value;
        #ifdef GEODE_IS_WINDOWS
        return geode::utils::string::wideToUtf8(active->path.c_str());
        #else
        return active->path.string();
        #endif
	}

    void onGetSongInfoCompleted(gd::string p1, gd::string p2) {
        MusicDownloadManager::onGetSongInfoCompleted(p1, p2);
        auto songID = std::stoi(p2);

        auto res = NongManager::get()->getNongs(songID);
        if (res.has_value()) {
            auto nongs = res.value();
            auto obj = this->getSongInfoObject(songID);
            nongs->defaultSong()->metadata()->m_name = obj->m_songName;
            nongs->defaultSong()->metadata()->m_artist = obj->m_artistName;
        }
        GetSongInfoEvent(this->getSongInfoObject(songID), songID).post();
    }

    SongInfoObject* getSongInfoObject(int id) {
        auto og = MusicDownloadManager::getSongInfoObject(id);
        if (og == nullptr) {
            return og;
        }
        auto res = NongManager::get()->getNongs(id);
        if (res.has_value()) {
            auto active = res.value()->active();
            og->m_songName = active->metadata->m_name;
            og->m_artistName = active->metadata->m_artist;
        }
        return og;
    }
};
