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
#include <arc/future/Future.hpp>
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

    SongMetadata(int gdID, std::string uniqueID, std::string name, std::string artist,
                 std::optional<std::string> level = std::nullopt, int offset = 0)
        : gdID(gdID),
          uniqueID(std::move(uniqueID)),
          name(std::move(name)),
          artist(std::move(artist)),
          level(std::move(level)),
          startOffset(offset) {}

    bool operator==(const SongMetadata& other) const {
        return gdID == other.gdID && uniqueID == other.uniqueID && name == other.name && artist == other.artist &&
               level == other.level && startOffset == other.startOffset;
    }
};

enum class NongType { LOCAL, YOUTUBE, HOSTED };

class Song {
public:
    virtual ~Song() = default;
    [[nodiscard]] virtual NongType type() const = 0;
    [[nodiscard]] virtual SongMetadata* metadata() const = 0;
    [[nodiscard]] virtual std::optional<std::string> indexID() const = 0;
    virtual void setIndexID(const std::string& id) = 0;
    // For local songs, this will always have a value, otherwise do check
    [[nodiscard]] virtual std::optional<std::filesystem::path> path() const = 0;
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

    LocalSong(LocalSong&& other) noexcept;
    LocalSong& operator=(LocalSong&& other) noexcept;

    ~LocalSong() override;

    [[nodiscard]] NongType type() const override { return NongType::LOCAL; };
    [[nodiscard]] SongMetadata* metadata() const override;
    [[nodiscard]] std::optional<std::filesystem::path> path() const override;
    void setPath(std::filesystem::path p) override;
    [[nodiscard]] std::optional<std::string> indexID() const override { return std::nullopt; }
    void setIndexID(const std::string& id) override {}

    static LocalSong createUnknown(int songID);
    static LocalSong fromSongObject(const SongInfoObject* obj);
};

class YTSong final : public Song {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;

public:
    YTSong(SongMetadata&& metadata, std::string youtubeID, std::optional<std::string> m_indexID,
           std::optional<std::filesystem::path> path = std::nullopt);
    YTSong(const YTSong& other);
    YTSong& operator=(const YTSong& other);

    YTSong(YTSong&& other) noexcept;
    YTSong& operator=(YTSong&& other) noexcept;

    ~YTSong() override;

    [[nodiscard]] NongType type() const override { return NongType::YOUTUBE; };
    [[nodiscard]] SongMetadata* metadata() const override;
    [[nodiscard]] std::string youtubeID() const;
    [[nodiscard]] std::optional<std::string> indexID() const override;
    void setIndexID(const std::string& id) override;
    [[nodiscard]] std::optional<std::filesystem::path> path() const override;
    void setPath(std::filesystem::path p) override;
    [[nodiscard]] arc::Future<geode::Result<geode::ByteVector>> startDownload() const;
};

class HostedSong final : public Song {
private:
    class Impl;

    std::unique_ptr<Impl> m_impl;

public:
    HostedSong(SongMetadata&& metadata, std::string url, std::optional<std::string> m_indexID,
               std::optional<std::filesystem::path> path = std::nullopt);
    HostedSong(const HostedSong& other);
    HostedSong& operator=(const HostedSong& other);

    HostedSong(HostedSong&& other) noexcept;
    HostedSong& operator=(HostedSong&& other) noexcept;

    ~HostedSong() override;

    [[nodiscard]] NongType type() const override { return NongType::HOSTED; };
    [[nodiscard]] SongMetadata* metadata() const override;
    [[nodiscard]] std::string url() const;
    [[nodiscard]] std::optional<std::string> indexID() const override;
    void setIndexID(const std::string& id) override;
    [[nodiscard]] std::optional<std::filesystem::path> path() const override;
    void setPath(std::filesystem::path p) override;
    [[nodiscard]] arc::Future<geode::Result<geode::ByteVector>> startDownload() const;
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

    Nongs(Nongs&&) noexcept;
    Nongs& operator=(Nongs&&) noexcept;

    ~Nongs();

    [[nodiscard]] int songID() const;
    [[nodiscard]] LocalSong* defaultSong() const;
    [[nodiscard]] Song* active() const;

    [[nodiscard]] bool isDefaultActive() const;

    geode::Result<> commit();
    /**
     * Returns Err if there is no NONG with the given path for the song ID
     * Otherwise, returns ok
     */
    geode::Result<> setActive(const std::string& uniqueID);
    geode::Result<> merge(Nongs&&);
    // Remove all custom nongs and set the default song as active
    [[nodiscard]] geode::Result<> deleteAllSongs();
    [[nodiscard]] geode::Result<> deleteSong(const std::string& uniqueID, bool audio = true);
    [[nodiscard]] geode::Result<> deleteSongAudio(const std::string& uniqueID) const;
    [[nodiscard]] std::optional<Song*> findSong(const std::string& uniqueID) const;

    [[nodiscard]] std::vector<std::unique_ptr<LocalSong>>& locals() const;
    [[nodiscard]] std::vector<std::unique_ptr<YTSong>>& youtube() const;
    [[nodiscard]] std::vector<std::unique_ptr<HostedSong>>& hosted() const;
    [[nodiscard]] std::vector<index::IndexSongMetadata*>& indexSongs() const;

    geode::Result<LocalSong*> add(LocalSong&& song) const;
    geode::Result<YTSong*> add(YTSong&& song) const;
    geode::Result<HostedSong*> add(HostedSong&& song) const;
    geode::Result<> replaceSong(const std::string& id, LocalSong&& song);
    geode::Result<> replaceSong(const std::string& id, YTSong&& song);
    geode::Result<> replaceSong(const std::string& id, HostedSong&& song);

    geode::Result<> registerIndexSong(index::IndexSongMetadata* song) const;
};

class Manifest {
    friend class NongManager;

private:
    int m_version = s_latestVersion;
    std::unordered_map<int, std::unique_ptr<Nongs>> m_nongs = {};

public:
    constexpr static int s_latestVersion = 4;

    Manifest() = default;
    Manifest(Manifest&&) noexcept = default;
    Manifest& operator=(Manifest&&) = default;
    Manifest(const Manifest&) = delete;
    Manifest& operator=(const Manifest&) = delete;

    [[nodiscard]] int version() const { return m_version; }
};

}  // namespace jukebox
