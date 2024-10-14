#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>

#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Event.hpp"
#include "Geode/loader/Mod.hpp"
#include "Geode/utils/Task.hpp"

#include "../../include/nong.hpp"
#include "../events/get_song_info_event.hpp"
#include "../events/song_error_event.hpp"

using namespace geode::prelude;

namespace jukebox {

class SongErrorEvent;

class NongManager {
protected:
    Manifest m_manifest;
    std::unordered_map<std::string, Song*> m_bigmap;
    bool m_initialized = false;

    NongManager() {
        this->init();
    }

    void setupManifestPath() {
        auto path = this->baseManifestPath();
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
        }
    }

    bool init();
    Result<> saveNongs(std::optional<int> saveId = std::nullopt);
    EventListener<EventFilter<SongErrorEvent>> m_songErrorListener;
    EventListener<EventFilter<GetSongInfoEvent>> m_songInfoListener;
    Result<std::unique_ptr<Nongs>> loadNongsFromPath(
        const std::filesystem::path& path);

public:
    using MultiAssetSizeTask = Task<std::string>;
    std::optional<Nongs*> m_currentlyPreparingNong;

    bool initialized() const { return m_initialized; }

    std::filesystem::path baseManifestPath() {
        static std::filesystem::path path =
            Mod::get()->getSaveDir() / "manifest";
        return path;
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
    std::filesystem::path generateSongFilePath(
        const std::string& extension,
        std::optional<std::string> filename = std::nullopt);

    static NongManager& get() {
        static NongManager instance;
        return instance;
    }
};

}  // namespace jukebox
