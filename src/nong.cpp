#include "../include/nong.hpp"

#include <Geode/binding/MusicDownloadManager.hpp>

#include <filesystem>
#include <memory>

namespace jukebox {

SongMetadata* LocalSong::metadata() const {
    return m_metadata.get();
}

std::filesystem::path LocalSong::path() const {
    return m_path;
}

LocalSong LocalSong::createUnknown(int songID) {
    return LocalSong {
        SongMetadata {
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
            obj->m_songName,
            obj->m_artistName
        },
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(obj->m_songID)
        )
    };
}

}