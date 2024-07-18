#pragma once


#include <filesystem>
#include <fmt/core.h>
#include <matjson.hpp>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/utils/Result.hpp"

#include "platform.hpp"

namespace jukebox {

struct JUKEBOX_DLL SongMetadata final {
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

class JUKEBOX_DLL LocalSong final {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
public:
    LocalSong(
        SongMetadata&& metadata,
        const std::filesystem::path& path
    );

    LocalSong(const LocalSong& other);

    LocalSong& operator=(const LocalSong& other);

    LocalSong(LocalSong&& other);
    LocalSong& operator=(LocalSong&& other);

    ~LocalSong();

    SongMetadata* metadata() const;
    std::filesystem::path path() const;

    // Might not be used
    static LocalSong createUnknown(int songID);
    static LocalSong fromSongObject(SongInfoObject* obj);
};

class JUKEBOX_DLL YTSong final {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;

public:
    YTSong(
        SongMetadata&& metadata,
        std::string youtubeID,
        std::optional<std::string> indexId,
        std::optional<std::filesystem::path> path = std::nullopt
    );
    YTSong(const YTSong& other);
    YTSong& operator=(const YTSong& other);

    YTSong(YTSong&& other);
    YTSong& operator=(YTSong&& other);

    ~YTSong();

    SongMetadata* metadata() const;
    std::string youtubeID() const;
    std::optional<std::string> indexId() const;
    std::optional<std::filesystem::path> path() const;
};


class JUKEBOX_DLL HostedSong final {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;

public:
    HostedSong(
        SongMetadata&& metadata,
        std::string url,
        std::optional<std::string> m_indexId,
        std::optional<std::filesystem::path> path = std::nullopt
    );
    HostedSong(const HostedSong& other);
    HostedSong& operator=(const HostedSong& other);

    HostedSong(HostedSong&& other);
    HostedSong& operator=(HostedSong&& other);

    ~HostedSong();

    SongMetadata* metadata() const;
    std::string url() const;
    std::optional<std::string> indexId() const;
    std::optional<std::filesystem::path> path() const;
};

struct JUKEBOX_DLL SongMetadataPathed final {
    SongMetadata m_metadata;
    std::filesystem::path m_path;

    SongMetadataPathed(
        SongMetadata metadata,
        std::filesystem::path path = ""
    ) : m_metadata(metadata),
        m_path(path)
    {}

    SongMetadataPathed(const LocalSong& song) : m_metadata(*song.metadata()), m_path(song.path()) {}
    SongMetadataPathed(const YTSong& song) : m_metadata(*song.metadata()), m_path(song.path().value()) {}
    SongMetadataPathed(const HostedSong& song) : m_metadata(*song.metadata()), m_path(song.path().value()) {}
};

class JUKEBOX_DLL Nongs final {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
public:
    Nongs(int songID, LocalSong&& defaultSong);
    Nongs(int songID);

    // No copies for this one
    Nongs(const Nongs&) = delete;
    Nongs& operator=(const Nongs&) = delete;

    Nongs(Nongs&&);
    Nongs& operator=(Nongs&&);

    ~Nongs();

    int songID() const;
    LocalSong* defaultSong() const;
    SongMetadataPathed* active() const;

    bool isDefaultActive() const;

    /**
     * Returns Err if there is no NONG with the given path for the song ID
     * Otherwise, returns ok
     */
    geode::Result<> setActive(const std::filesystem::path& path);
    geode::Result<> merge(Nongs&&);
    geode::Result<> deleteAllSongs();
    geode::Result<> deleteSong(const std::filesystem::path& path);
    void deleteSong(const std::string& indexId);

    std::vector<std::unique_ptr<LocalSong>>& locals();
    std::vector<std::unique_ptr<YTSong>>& youtube();
    std::vector<std::unique_ptr<HostedSong>>& hosted();

    // Remove all custom nongs and set the default song as active
    void resetToDefault();

    geode::Result<LocalSong*> add(LocalSong song);
    geode::Result<YTSong*> add(YTSong song);
    geode::Result<HostedSong*> add(HostedSong song);

    void remove(LocalSong* song);
    void remove(YTSong* song);
    void remove(HostedSong* song);
};

class JUKEBOX_DLL Manifest {
    friend class NongManager;
private:
    int m_version = s_latestVersion;
    std::unordered_map<int, std::unique_ptr<Nongs>> m_nongs = {};
public:
    constexpr static inline int s_latestVersion = 4;

    Manifest() = default;
    Manifest(const Manifest&) = delete;
    Manifest& operator=(const Manifest&) = delete;

    int version() const {
        return m_version;
    }
};

class JUKEBOX_DLL Nong final {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
public:
    Nong(const LocalSong& local);
    Nong(const YTSong& yt);
    Nong(const HostedSong& hosted);

    // No copies for this one
    Nong(const Nong&) = delete;
    Nong& operator=(const Nong&) = delete;

    Nong(Nong&&);
    Nong& operator=(Nong&&);

    ~Nong();

    SongMetadata* metadata() const;
    geode::Result<Nongs> toNongs() const;

    void visit(
        std::function<void(LocalSong*)> local,
        std::function<void(YTSong*)> yt,
        std::function<void(HostedSong*)> hosted
    ) const;
};

}
