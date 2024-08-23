#pragma once

#include <filesystem>
#include <optional>

#include "Geode/utils/Task.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Event.hpp"
#include "Geode/loader/Mod.hpp"

#include "../../include/nong.hpp"

using namespace geode::prelude;

namespace jukebox {

class SongError final : public Event {
private:
    friend class NongManager;
    friend class IndexManager;

    template<typename... Args>
    SongError(
        bool notifyUser,
        fmt::format_string<Args...> format,
        Args&&... args
    ) : m_error(fmt::format(format, std::forward<Args>(args)...)),
        m_notifyUser(notifyUser)
    {};

public:
    bool m_notifyUser;
    std::string m_error;
};

class SongStateChanged final : public Event {
private:
    friend class NongManager;
    friend class IndexManager;

    SongStateChanged(
        int gdSongID
    ) : m_gdSongID(gdSongID)
    {};
public:
    int m_gdSongID;
};

class SongDownloadProgress final : public Event {
private:
    friend class NongManager;
    friend class IndexManager;

    SongDownloadProgress(
        int gdSongID,
        std::string m_uniqueID,
        float progress
    ) : m_gdSongID(gdSongID),
        m_uniqueID(m_uniqueID),
        m_progress(progress)
    {};
public:
    int m_gdSongID;
    std::string m_uniqueID;
    float m_progress;
};

enum class SongInfoGetAction {
    CreateDefault,
    FixDefault
};

class NongManager : public CCObject {
protected:
    inline static NongManager* m_instance = nullptr;
    Manifest m_manifest;
    bool m_initialized = false;

    void setupManifestPath() {
        auto path = this->baseManifestPath();
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
        }
    }

    std::filesystem::path baseManifestPath() {
        static std::filesystem::path path = Mod::get()->getSaveDir() / "manifest";
        return path;
    }

    bool init();
    Result<> saveNongs(std::optional<int> saveId = std::nullopt);
    std::unique_ptr<EventListener<EventFilter<SongError>>> m_songErrorListener;
    Result<std::unique_ptr<Nongs>> loadNongsFromPath(const std::filesystem::path& path);
public:
    using MultiAssetSizeTask = Task<std::string>;
    std::optional<Nongs*> m_currentlyPreparingNong;

    bool initialized() const {
        return m_initialized;
    }

    void initSongID(SongInfoObject* obj, int id, bool robtop);

    /**
     * Adjusts a song ID with respect to Robtop songs
    */
    int adjustSongID(int id, bool robtop);

    /**
     * Execute callbacks stored for getSongInfo for a songID, if they exist
    */
    void resolveSongInfoCallback(int id);

    /**
     * Gets the current manifest version stored in state
    */
    int getCurrentManifestVersion();

    /**
     * Gets the current number of song IDs that have been added to the manifest
    */
    int getStoredIDCount();

    /**
     * Get Nong from manifest
     * @param gdSongID the id of the song in GD
     * @param uniqueID the unique id of the song in Jukebox
     */
    Result<Nong> getNongFromManifest(int gdSongID, std::string uniqueID);

    /**
     * Fetches all NONG data for a certain songID
     *
     * @param songID the id of the song
     * @return the data from the JSON or nullopt if it wasn't created yet
    */
    std::optional<Nongs*> getNongs(int songID);

    /**
     * Formats a size in bytes to a x.xxMB string
     *
     * @param path the path to calculate the filesize of
     *
     * @return the formatted size, with the format x.xxMB
    */
    std::string getFormattedSize(const std::filesystem::path& path);

    /**
     * Calculates the total size of multiple assets, then writes it to a string.
     * Runs on a separate thread. Returns a task that will resolve to the total size.
     *
     * @param songs string of song ids, separated by commas
     * @param sfx string of sfx ids, separated by commas
    */
    MultiAssetSizeTask getMultiAssetSizes(std::string songs, std::string sfx);

    /**
     * Add actions needed to fix a broken song default
     * @param songID id of the song
    */
    void refetchDefault(int songID);

    /**
     * Add NONGs
     * @param nong NONG to add
    */
    Result<> addNongs(Nongs&& nong);

    /**
     * Set active song
     * @param gdSongID the id of the song in GD
     * @param uniqueID the unique id of the song in Jukebox
    */
    Result<> setActiveSong(int gdSongID, std::string uniqueID);

    /**
     * Delete a song
     * @param gdSongID the id of the song in GD
     * @param uniqueID the unique id of the song in Jukebox
    */
    Result<> deleteSong(int gdSongID, std::string uniqueID);

    /**
     * Delete a song's audio file
     * @param gdSongID the id of the song in GD
     * @param uniqueID the unique id of the song in Jukebox
    */
    Result<> deleteSongAudio(int gdSongID, std::string uniqueID);

    /**
     * Delete all NONGs for a song ID
     * @param gdSongID id of the song
    */
    Result<> deleteAllSongs(int gdSongID);

    /**
     * Get a path to a song file
    */
    std::filesystem::path generateSongFilePath(const std::string& extension, std::optional<std::string> filename = std::nullopt);

    static NongManager* get() {
        if (m_instance == nullptr) {
            m_instance = new NongManager();
            m_instance->retain();
            m_instance->init();
        }

        return m_instance;
    }
};

}
