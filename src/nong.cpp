#include "../include/nong.hpp"

#include <Geode/binding/MusicDownloadManager.hpp>

#include <filesystem>

namespace jukebox {

LocalSong LocalSong::createUnknown(int songID) {
    return LocalSong {
        SongMetadata {
            songID,
            "Unknown",
            ""
        },
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(songID)
        )
    };
}

LocalSong LocalSong::fromSongObject(SongInfoObject* obj) {
    return LocalSong {
        SongMetadata {
            obj->m_songID,
            obj->m_songName,
            obj->m_artistName
        },
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(obj->m_songID)
        )
    };
}

}