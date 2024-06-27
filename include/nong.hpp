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
    int m_gdID;
    std::string m_name;
    std::string m_artist;
    std::optional<std::string> m_level;
    uint32_t m_startOffset;

    SongMetadata(
        int gdID,
        std::string name,
        std::string artist,
        std::optional<std::string> level = std::nullopt,
        uint32_t offset = 0
    ) : m_gdID(gdID),
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
        : m_path(other.m_path),
        m_metadata(std::make_unique<SongMetadata>(*other.m_metadata))
    {}

    LocalSong& operator=(const LocalSong& other) {
        m_path = other.m_path;
        m_metadata = std::make_unique<SongMetadata>(*other.m_metadata);

        return *this;
    }

    LocalSong(LocalSong&& other) = default;
    LocalSong& operator=(LocalSong&& other) = default;

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
        m_path(other.m_path),
        m_metadata(std::make_unique<SongMetadata>(*other.m_metadata))
    {}

    YTSong& operator=(const YTSong& other) {
        m_youtubeID = other.m_youtubeID;
        m_path = other.m_path;
        m_metadata = std::make_unique<SongMetadata>(*other.m_metadata);

        return *this;
    }

    YTSong(YTSong&& other) = default;
    YTSong& operator=(YTSong&& other) = default;

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

class JUKEBOX_DLL HostedSong {
private:
    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_url;
    std::optional<std::filesystem::path> m_path;
public:
    HostedSong(
        SongMetadata&& metadata,
        std::string&& url,
        std::optional<std::filesystem::path>&& path = std::nullopt
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_url(std::move(url)),
        m_path(std::move(path))
    {}

    HostedSong(const HostedSong& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
        m_path(other.m_path),
        m_url(other.m_url)
    {}

    HostedSong& operator=(const HostedSong& other) {
        m_metadata = std::make_unique<SongMetadata>(*other.m_metadata);
        m_path = other.m_path;
        m_url = other.m_url;

        return *this;
    }

    HostedSong(HostedSong&& other) = default;
    HostedSong& operator=(HostedSong&& other) = default;

    SongMetadata* metadata() const {
        return m_metadata.get();
    }
    std::string url() const {
        return m_url;
    }
    std::optional<std::filesystem::path> path() const {
        return m_path;
    }
};

}
