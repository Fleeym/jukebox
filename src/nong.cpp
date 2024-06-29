#include "../include/nong.hpp"

#include <Geode/binding/MusicDownloadManager.hpp>

#include <filesystem>

using namespace geode::prelude;

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

Result<> Nongs::setActive(const std::filesystem::path& path) {
    if (m_default->path() == path) {
        m_active->path = m_default->path();
        m_active->metadata = m_default->metadata();
        return Ok();
    }

    for (const auto& i: m_locals) {
        if (i->path() == path) {
            m_active->path = i->path();
            m_active->metadata = i->metadata();
            return Ok();
        }
    }

    for (const auto& i: m_youtube) {
        if (i->path().has_value() && i->path() == path) {
            m_active->path = i->path().value();
            m_active->metadata = i->metadata();
            return Ok();
        }
    }

    return Err("No song found with given path for song ID");
}

}