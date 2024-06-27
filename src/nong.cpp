#include "../include/nong.hpp"
#include "Geode/binding/MusicDownloadManager.hpp"

#include <filesystem>
#include <memory>

namespace jukebox {

SongMetadata* LocalNong::metadata() const {
    return m_metadata.get();
}

std::filesystem::path LocalNong::path() const {
    return m_path;
}

LocalNong LocalNong::createUnknown(int songID) {
    return LocalNong {
        SongMetadata {
            "Unknown",
            ""
        },
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(songID)
        )
    };
}

LocalNong LocalNong::fromSongObject(SongInfoObject* obj) {
    return LocalNong {
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