#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include <fmt/core.h>
#include <Geode/Result.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/utils/general.hpp>
#include <matjson.hpp>

#include <jukebox/nong/index.hpp>

namespace jukebox {

struct SongMetadata final {
    int gdID;
    std::string uniqueID;
    std::string name;
    std::string artist;
    std::optional<std::string> level;
    int startOffset;

    SongMetadata(int gdID, std::string uniqueID, std::string name,
                 std::string artist,
                 std::optional<std::string> level = std::nullopt,
                 int offset = 0)
        : gdID(gdID),
          uniqueID(uniqueID),
          name(name),
          artist(artist),
          level(level),
          startOffset(offset) {}

    bool operator==(const SongMetadata& other) const {
        return gdID == other.gdID && uniqueID == other.uniqueID &&
               name == other.name && artist == other.artist &&
               level == other.level && startOffset == other.startOffset;
    }
};

enum class NongType { LOCAL, YOUTUBE, HOSTED };

class Song {
public:
    virtual ~Song() = default;
    virtual NongType type() const = 0;
    virtual SongMetadata* metadata() const = 0;
    virtual std::optional<std::string> indexID() const = 0;
    virtual void setIndexID(const std::string& id) = 0;
    // For local songs, this will always have a value, otherwise do check
    virtual std::optional<std::filesystem::path> path() const = 0;
    virtual void setPath(std::filesystem::path p) = 0;
};

class LocalSong final : public Song {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;

public:
    LocalSong(SongMetadata&& metadata, const std::filesystem::path& path);

    LocalSong(const LocalSong& other);
    LocalSong& operator=(const LocalSong& other);

    LocalSong(LocalSong&& other);
    LocalSong& operator=(LocalSong&& other);

    ~LocalSong();

    NongType type() const { return NongType::LOCAL; };
    SongMetadata* metadata() const;
    std::optional<std::filesystem::path> path() const;
    void setPath(std::filesystem::path p);
    std::optional<std::string> indexID() const { return std::nullopt; }
    void setIndexID(const std::string& id) {}

    static LocalSong createUnknown(int songID);
    static LocalSong fromSongObject(SongInfoObject* obj);
};

class YTSong final : public Song {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;

public:
    YTSong(SongMetadata&& metadata, std::string youtubeID,
           std::optional<std::string> m_indexID,
           std::optional<std::filesystem::path> path = std::nullopt);
    YTSong(const YTSong& other);
    YTSong& operator=(const YTSong& other);

    YTSong(YTSong&& other);
    YTSong& operator=(YTSong&& other);

    ~YTSong();

    NongType type() const { return NongType::YOUTUBE; };
    SongMetadata* metadata() const;
    std::string youtubeID() const;
    std::optional<std::string> indexID() const;
    void setIndexID(const std::string& id);
    std::optional<std::filesystem::path> path() const;
    void setPath(std::filesystem::path p);
    geode::Result<geode::Task<geode::Result<geode::ByteVector>, float>>
    startDownload();
};

class HostedSong final : public Song {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;

public:
    HostedSong(SongMetadata&& metadata, std::string url,
               std::optional<std::string> m_indexID,
               std::optional<std::filesystem::path> path = std::nullopt);
    HostedSong(const HostedSong& other);
    HostedSong& operator=(const HostedSong& other);

    HostedSong(HostedSong&& other);
    HostedSong& operator=(HostedSong&& other);

    ~HostedSong();

    NongType type() const { return NongType::HOSTED; };
    SongMetadata* metadata() const;
    std::string url() const;
    std::optional<std::string> indexID() const;
    void setIndexID(const std::string& id);
    std::optional<std::filesystem::path> path() const;
    void setPath(std::filesystem::path p);
    geode::Result<geode::Task<geode::Result<geode::ByteVector>, float>>
    startDownload();
};

class Nongs final {
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
    Song* active() const;

    bool isDefaultActive() const;

    geode::Result<> commit();
    /**
     * Returns Err if there is no NONG with the given path for the song ID
     * Otherwise, returns ok
     */
    geode::Result<> setActive(const std::string& uniqueID);
    geode::Result<> merge(Nongs&&);
    // Remove all custom nongs and set the default song as active
    geode::Result<> deleteAllSongs();
    geode::Result<> deleteSong(const std::string& uniqueID, bool audio = true);
    geode::Result<> deleteSongAudio(const std::string& uniqueID);
    std::optional<Song*> findSong(const std::string& uniqueID);

    std::vector<std::unique_ptr<LocalSong>>& locals() const;
    std::vector<std::unique_ptr<YTSong>>& youtube() const;
    std::vector<std::unique_ptr<HostedSong>>& hosted() const;
    std::vector<index::IndexSongMetadata*>& indexSongs() const;

    geode::Result<LocalSong*> add(LocalSong&& song);
    geode::Result<YTSong*> add(YTSong&& song);
    geode::Result<HostedSong*> add(HostedSong&& song);
    geode::Result<> replaceSong(const std::string& id, LocalSong&& song);
    geode::Result<> replaceSong(const std::string& id, YTSong&& song);
    geode::Result<> replaceSong(const std::string& id, HostedSong&& song);

    geode::Result<> registerIndexSong(index::IndexSongMetadata* song);
};

class Manifest {
    friend class NongManager;

private:
    int m_version = s_latestVersion;
    std::unordered_map<int, std::unique_ptr<Nongs>> m_nongs = {};

public:
    constexpr static inline int s_latestVersion = 4;

    Manifest() = default;
    Manifest(Manifest&&) = default;
    Manifest& operator=(Manifest&&) = default;
    Manifest(const Manifest&) = delete;
    Manifest& operator=(const Manifest&) = delete;

    int version() const { return m_version; }
};

}  // namespace jukebox
