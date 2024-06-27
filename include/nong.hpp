#pragma once


#include <filesystem>
#include <fmt/core.h>
#include <matjson.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "Geode/utils/Result.hpp"
#include "Geode/utils/string.hpp"
#include "Geode/binding/SongInfoObject.hpp"

#include "platform.hpp"

namespace jukebox {

struct JUKEBOX_DLL SongMetadata {
    int m_gdID;
    std::string m_name;
    std::string m_artist;
    std::optional<std::string> m_level;
    int m_startOffset;

    SongMetadata(
        int gdID,
        std::string name,
        std::string artist,
        std::optional<std::string> level = std::nullopt,
        int offset = 0
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

class JUKEBOX_DLL NongData {
private:
    int m_songID;
    std::unique_ptr<LocalSong> m_default;
    std::vector<std::unique_ptr<LocalSong>> m_locals {};
    std::vector<std::unique_ptr<YTSong>> m_youtube {};
    std::vector<std::unique_ptr<HostedSong>> m_hosted {};
public:
    NongData(int songID, LocalSong defaultSong)
        : m_songID(songID),
        m_default(std::make_unique<LocalSong>(defaultSong))
    {}

    int songID() const {
        return m_songID;
    }

    LocalSong* defaultSong() const {
        return m_default.get();
    }

    std::vector<std::unique_ptr<LocalSong>>& locals() {
        return m_locals;
    }
    std::vector<std::unique_ptr<YTSong>>& youtube() {
        return m_youtube;
    }
    std::vector<std::unique_ptr<HostedSong>>& hosted() {
        return m_hosted;
    }
};

}

template<> 
struct matjson::Serialize<jukebox::SongMetadata> {
    static geode::Result<jukebox::SongMetadata> from_json(
        const matjson::Value& value,
        int songID
    ) {
        if (!(value.contains("name") || !value["name"].is_string())) {
            return geode::Err("Invalid JSON key name");
        }
        if (!value.contains("artist") || !value["artist"].is_string()) {
            return geode::Err("Invalid JSON key artist");
        }

        return geode::Ok(jukebox::SongMetadata {
            songID,
            value["name"].as_string(),
            value["artist"].as_string(),
            value.contains("level")
                && value["level"].is_string()
                    ? std::optional(value["level"].as_string())
                    : std::nullopt,
            value.contains("offset") 
                && value["offset"].is_number()
                    ? value["offset"].as_int()
                    : 0
        });
    }
};

template<>
struct matjson::Serialize<jukebox::LocalSong> {
    static geode::Result<jukebox::LocalSong> from_json(
        const matjson::Value& value,
        int songID
    ) {
        auto metadata = matjson::Serialize<jukebox::SongMetadata>
            ::from_json(value, songID);
        
        if (metadata.isErr()) {
            return geode::Err(
                fmt::format(
                    "Local Song {} is invalid. Reason: {}",
                    value.dump(matjson::NO_INDENTATION),
                    metadata.unwrapErr()
                )
            );
        }

        if (!value.contains("path") || !value["path"].is_string()) {
            return geode::Err(
                "Local Song {} is invalid. Reason: invalid path",
                value.dump(matjson::NO_INDENTATION)
            );
        }

        return geode::Ok(jukebox::LocalSong {
            std::move(metadata.unwrap()),
            value["path"].as_string()
        });
    }

    static matjson::Value to_json(const jukebox::LocalSong& value) {
        matjson::Object ret = {};
        ret["name"] = value.metadata()->m_name;
        ret["artist"] = value.metadata()->m_artist;
        #ifdef GEODE_IS_WINDOWS
        ret["path"] = geode::utils::string::wideToUtf8(value.path().c_str());
        ret["offset"] = value.metadata()->m_startOffset;
        #else
        ret["path"] = value.path().string();
        #endif
        if (value.metadata()->m_level.has_value()) {
            ret["level"] = value.metadata()->m_level.value();
        }
        return ret;
    }
};

template<>
struct matjson::Serialize<jukebox::YTSong> {
    static geode::Result<jukebox::YTSong> from_json(
        const matjson::Value& value,
        int songID
    ) {
        auto metadata = matjson::Serialize<jukebox::SongMetadata>
            ::from_json(value, songID);
        
        if (metadata.isErr()) {
            return geode::Err(
                fmt::format(
                    "Local Song {} is invalid. Reason: {}",
                    value.dump(matjson::NO_INDENTATION),
                    metadata.unwrapErr()
                )
            );
        }

        if (!value.contains("path") || !value["path"].is_string()) {
            return geode::Err(
                "YT Song {} is invalid. Reason: invalid path",
                value.dump(matjson::NO_INDENTATION)
            );
        }

        if (!value.contains("youtube_id") || !value["youtube_id"].is_string()) {
            return geode::Err(
                "YT Song {} is invalid. Reason: invalid youtube ID",
                value.dump(matjson::NO_INDENTATION)
            );
        }

        return geode::Ok(jukebox::YTSong {
            std::move(metadata.unwrap()),
            value["youtube_id"].as_string(),
            value["path"].as_string(),
        });
    }

    static geode::Result<matjson::Value> to_json(const jukebox::YTSong& value) {
        if (!value.path().has_value()) {
            return geode::Err("YT song has no local path specified");
        }
        matjson::Object ret = {};
        ret["name"] = value.metadata()->m_name;
        ret["artist"] = value.metadata()->m_artist;
        ret["offset"] = value.metadata()->m_startOffset;
        ret["youtube_id"] = value.youtubeID();

        #ifdef GEODE_IS_WINDOWS
        ret["path"] = geode::utils::string::wideToUtf8(value.path().value().c_str());
        #else
        ret["path"] = value.path().value().string();
        #endif

        if (value.metadata()->m_level.has_value()) {
            ret["level"] = value.metadata()->m_level.value();
        }
        return geode::Ok(ret);
    }
};

template<>
struct matjson::Serialize<jukebox::HostedSong> {
    static geode::Result<jukebox::HostedSong> from_json(
        const matjson::Value& value,
        int songID
    ) {
        auto metadata = matjson::Serialize<jukebox::SongMetadata>
            ::from_json(value, songID);
        
        if (metadata.isErr()) {
            return geode::Err(
                fmt::format(
                    "Local Song {} is invalid. Reason: {}",
                    value.dump(matjson::NO_INDENTATION),
                    metadata.unwrapErr()
                )
            );
        }

        if (!value.contains("path") || !value["path"].is_string()) {
            return geode::Err(
                "Hosted Song {} is invalid. Reason: invalid path",
                value.dump(matjson::NO_INDENTATION)
            );
        }

        if (!value.contains("url") || !value["url"].is_string()) {
            return geode::Err(
                "Hosted Song {} is invalid. Reason: invalid url",
                value.dump(matjson::NO_INDENTATION)
            );
        }

        return geode::Ok(jukebox::HostedSong{
            std::move(metadata.unwrap()),
            value["url"].as_string(),
            value["path"].as_string()
        });
    }
};