#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>

#include "../managers/nong_manager.hpp"
#include "../types/song_info.hpp"

class $modify(MusicDownloadManager) {
	gd::string pathForSong(int id) {
        auto active = NongManager::get()->getActiveNong(id);
        if (!active.has_value()) {
		    return MusicDownloadManager::pathForSong(id);
        }
        auto value = active.value();
		if (!fs::exists(value.path)) {
		    return MusicDownloadManager::pathForSong(id);
		}
		return value.path.string();
	}
    void onGetSongInfoCompleted(gd::string p1, gd::string p2) {
        MusicDownloadManager::onGetSongInfoCompleted(p1, p2);
        auto songID = std::stoi(p2);
        NongManager::get()->resolveSongInfoCallback(songID);
    }

    SongInfoObject* getSongInfoObject(int id) {
        auto og = MusicDownloadManager::getSongInfoObject(id);
        if (og == nullptr) {
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