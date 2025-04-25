#pragma once

#include <filesystem>
#include <memory>
#include <optional>

#include <Geode/Result.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/Task.hpp>

#include <jukebox/events/get_song_info.hpp>
#include <jukebox/events/song_error.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox {

class NongManager {
protected:
    Manifest m_manifest;
    bool m_initialized = false;

    NongManager() = default;
    NongManager(const NongManager&) = delete;
    NongManager(NongManager&&) = delete;

    NongManager& operator=(const NongManager&) = delete;
    NongManager& operator=(NongManager&&) = delete;

    void setupManifestPath() {
        auto path = this->baseManifestPath();
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
        }
    }

    geode::Result<> saveNongs(std::optional<int> saveId = std::nullopt);
    geode::EventListener<geode::EventFilter<jukebox::event::SongError>>
        m_songErrorListener;
    geode::EventListener<geode::EventFilter<jukebox::event::GetSongInfo>>
        m_songInfoListener;
    geode::Result<std::unique_ptr<Nongs>> loadNongsFromPath(
        const std::filesystem::path& path);

    geode::Result<> migrateV2();

public:
    using MultiAssetSizeTask = geode::Task<std::string>;
    std::optional<Nongs*> m_currentlyPreparingNong;

    bool init();

    bool initialized() const { return m_initialized; }

    std::filesystem::path baseManifestPath() {
        static std::filesystem::path path =
            geode::Mod::get()->getSaveDir() / "manifest";
        return path;
    }

    std::filesystem::path baseNongsPath() {
        static std::filesystem::path path =
            geode::Mod::get()->getSaveDir() / "nongs";
        return path;
    }

    bool hasSongID(int id);

    geode::Result<Nongs*> initSongID(SongInfoObject* obj, int id, bool robtop);

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
     * Fetches all NONG data for a certain songID
     *
     * @param songID the id of the song
     * @return the data from the JSON or nullopt if it wasn't created yet
     */
    std::optional<Nongs*> getNongs(int songID);

    /**
     * Returns all the uniqueIDs of nongs that are verified for the given level ID
     *
     * @param levelID the id of the level
     * @param songIDs list of all the song ids to check their nongs
     * @return List of all uniqueIDs of nongs that are verified for the given level ID
     */
    std::vector<std::string> getVerifiedNongsForLevel(int levelID, std::vector<int> songIDs);

    /**
     * Returns whether the nong is verified for the a song in a level
     *
     * @param levelID the id of the level
     * @param songID the id of a song in the level
     * @param uniqueID the id of the nong
     * @return Boolean for whether the nong is verified 
     */
    bool isNongVerifiedForLevelSong(int levelID, int songID, const std::string& uniqueID);

    /**
     * Checks if the given level has a verified song for any of the given song IDs
     *
     * @param levelID the id of the level
     * @param songIDs list of all the song ids to check
     * @return Whether one of the songs has a verified nong for the level
     */
    bool isNongVerified(int levelID, std::vector<int> songIDs);

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
     * Runs on a separate thread. Returns a task that will resolve to the total
     * size.
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
    geode::Result<> addNongs(Nongs&& nong);

    /**
     * Set active song
     * @param gdSongID the id of the song in GD
     * @param uniqueID the unique id of the song in Jukebox
     */
    geode::Result<> setActiveSong(int gdSongID, std::string uniqueID);

    /**
     * Delete a song
     * @param gdSongID the id of the song in GD
     * @param uniqueID the unique id of the song in Jukebox
     */
    geode::Result<> deleteSong(int gdSongID, std::string uniqueID);

    /**
     * Delete a song's audio file
     * @param gdSongID the id of the song in GD
     * @param uniqueID the unique id of the song in Jukebox
     */
    geode::Result<> deleteSongAudio(int gdSongID, std::string uniqueID);

    /**
     * Delete all NONGs for a song ID
     * @param gdSongID id of the song
     */
    geode::Result<> deleteAllSongs(int gdSongID);

    /**
     * Get a path to a song file
     */
    std::filesystem::path generateSongFilePath(
        const std::string& extension,
        std::optional<std::string> filename = std::nullopt);

    static NongManager& get() {
        static NongManager instance;
        return instance;
    }
};

}  // namespace jukebox
