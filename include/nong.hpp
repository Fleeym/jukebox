#pragma once

#include "Geode/binding/SongInfoObject.hpp"
#include <Geode/cocos/cocoa/CCObject.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

namespace jukebox {

struct SongMetadata {
    std::string m_name;
    std::string m_artist;
    std::optional<std::string> m_level;
    uint32_t m_startOffset;

    SongMetadata(
        std::string name,
        std::string artist,
        std::optional<std::string> level = std::nullopt,
        uint32_t offset = 0
    ) : m_name(name),
        m_artist(artist),
        m_level(level),
        m_startOffset(offset)
    {}
};

class LocalNong {
private:
    std::unique_ptr<SongMetadata> m_metadata;
    std::filesystem::path m_path;
public:
    LocalNong(
        SongMetadata&& metadata,
        const std::filesystem::path& path
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_path(path) 
    {}

    LocalNong(const LocalNong& other) 
        : m_path(other.m_path) {
        m_metadata = std::make_unique<SongMetadata>(SongMetadata {
            other.m_metadata->m_name,
            other.m_metadata->m_artist,
            other.m_metadata->m_level,
            other.m_metadata->m_startOffset,
        });
    }

    SongMetadata* metadata() const;
    std::filesystem::path path() const;

    // Might not be used
    static LocalNong createUnknown(int songID);

    static LocalNong fromSongObject(SongInfoObject* obj);
};

}
