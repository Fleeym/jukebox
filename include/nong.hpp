#pragma once

#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/cocos/cocoa/CCObject.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

#include "platform.hpp"

namespace jukebox {

struct JUKEBOX_DLL SongMetadata {
    int m_id;
    std::string m_name;
    std::string m_artist;
    std::optional<std::string> m_level;
    uint32_t m_startOffset;

    SongMetadata(
        int id,
        std::string name,
        std::string artist,
        std::optional<std::string> level = std::nullopt,
        uint32_t offset = 0
    ) : m_id(id),
        m_name(name),
        m_artist(artist),
        m_level(level),
        m_startOffset(offset)
    {}
};

class JUKEBOX_DLL LocalSong {
private:
    std::unique_ptr<SongMetadata> m_metadata;
    std::filesystem::path m_path;
public:
    LocalSong(
        SongMetadata&& metadata,
        const std::filesystem::path& path
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_path(path) 
    {}

    LocalSong(const LocalSong& other) 
        : m_path(other.m_path) 
    {
        m_metadata = std::make_unique<SongMetadata>(SongMetadata {
            other.m_metadata->m_id,
            other.m_metadata->m_name,
            other.m_metadata->m_artist,
            other.m_metadata->m_level,
            other.m_metadata->m_startOffset,
        });
    }

    LocalSong& operator=(const LocalSong& other) {
        m_path = other.m_path;
        m_metadata = std::make_unique<SongMetadata>(SongMetadata {
            other.m_metadata->m_id,
            other.m_metadata->m_name,
            other.m_metadata->m_artist,
            other.m_metadata->m_level,
            other.m_metadata->m_startOffset,
        });

        return *this;
    }

    LocalSong(LocalSong&& other)
        : m_path(std::move(other.m_path)),
        m_metadata(std::move(other.m_metadata))
    {}

    LocalSong& operator=(LocalSong&& other) {
        m_path = std::move(other.m_path);
        m_metadata = std::move(other.m_metadata);

        return *this;
    }

    SongMetadata* metadata() const {
        return m_metadata.get();
    }
    std::filesystem::path path() const {
        return m_path;
    }

    // Might not be used
    static LocalSong createUnknown(int songID);
    static LocalSong fromSongObject(SongInfoObject* obj);
};

class JUKEBOX_DLL YTSong {
private:
    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_youtubeID;
    std::optional<std::filesystem::path> m_path;
public:
    YTSong(
        SongMetadata&& metadata,
        std::string&& youtubeID,
        std::optional<std::filesystem::path>&& path = std::nullopt
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_youtubeID(youtubeID),
        m_path(path)
    {}

    YTSong(const YTSong& other)
        : m_youtubeID(other.m_youtubeID),
        m_path(other.m_path)
    {
        m_metadata = std::make_unique<SongMetadata>(SongMetadata {
            other.m_metadata->m_id,
            other.m_metadata->m_name,
            other.m_metadata->m_artist,
            other.m_metadata->m_level,
            other.m_metadata->m_startOffset,
        });
    }

    YTSong& operator=(const YTSong& other) {
        m_youtubeID = other.m_youtubeID;
        m_path = other.m_path;
        m_metadata = std::make_unique<SongMetadata>(SongMetadata {
            other.m_metadata->m_id,
            other.m_metadata->m_name,
            other.m_metadata->m_artist,
            other.m_metadata->m_level,
            other.m_metadata->m_startOffset,
        });

        return *this;
    }

    YTSong(YTSong&& other)
        : m_metadata(std::move(other.m_metadata)),
        m_path(std::move(other.m_path)),
        m_youtubeID(std::move(other.m_youtubeID))
    {}

    YTSong& operator=(YTSong&& other) {
        m_youtubeID = std::move(other.m_youtubeID);
        m_path = std::move(other.m_path);
        m_metadata = std::move(other.m_metadata);

        return *this;
    }

    SongMetadata* metadata() const {
        return m_metadata.get();
    }
    std::string youtubeID() const {
        return m_youtubeID;
    }
    std::optional<std::filesystem::path> path() const {
        return m_path;
    }
};

}
