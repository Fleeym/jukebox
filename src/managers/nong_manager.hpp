#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <optional>
#include <map>

#include "../types/song_info.hpp"
#include "../types/nong_state.hpp"
#include "../random_string.hpp"
#include "../trim.hpp"

using namespace geode::prelude;

class NongManager : public CCObject {
protected:
    inline static NongManager* m_instance = nullptr;
    NongState m_state;
    std::map<int, std::function<void(int)>> m_getSongInfoCallbacks;
public:
    /**
     * Only used once, on game launch. Reads the json and loads it into memory.
    */
    void loadSongs();

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
     * Adds a NONG to the JSON of a songID
     * 
     * @param song the song to add
     * @param songID the id of the song
    */
    void addNong(SongInfo const& song, int songID);

    /**
     * Removes a NONG from the JSON of a songID
     * 
     * @param song the song to remove
     * @param songID the id of the song
    */
    void deleteNong(SongInfo const& song, int songID);

    /**
     * Fetches all NONG data for a certain songID
     * 
     * @param songID the id of the song
     * @return the data from the JSON or nullopt if it wasn't created yet
    */
    std::optional<NongData> getNongs(int songID);

    /**
     * Fetches the active song from the songID JSON
     * 
     * @param songID the id of the song
     * @return the song data or nullopt in case of an error
    */
    std::optional<SongInfo> getActiveNong(int songID);

    /**
     * Fetches the default song from the songID JSON
     * 
     * @param songID the id of the song
     * @return the song data or nullopt in case of an error
    */
    std::optional<SongInfo> getDefaultNong(int songID);

    /**
     * Validates any local nongs that have an invalid path
     * 
     * @param songID the id of the song
     * 
     * @return an array of songs that were deleted as result of the validation
    */
    std::vector<SongInfo> validateNongs(int songID);

    /**
     * Saves NONGS to the songID JSON
     * 
     * @param data the data to save
     * @param songID the id of the song
    */
    void saveNongs(NongData const& data, int songID);

    /**
     * Writes song data to the JSON
    */
    void writeJson();

    /**
     * Removes all NONG data for a song ID
     * 
     * @param songID the id of the song
    */
    void deleteAll(int songID);

    /**
     * Formats a size in bytes to a x.xxMB string
     * 
     * @param song the song
     * 
     * @return the formatted size, with the format x.xxMB
    */
    std::string getFormattedSize(SongInfo const& song);

    /**
     * Calculates the total size of multiple assets, then writes it to a string.
     * Runs on a separate thread. Returns the result in the provided callback.
     * 
     * @param songs string of song ids, separated by commas
     * @param sfx string of sfx ids, separated by commas
     * @param callback callback that handles the computed string
    */
    void getMultiAssetSizes(std::string songs, std::string sfx, std::function<void(std::string)> callback);

    /**
     * Fetches song info for an id and creates the default entry in the json.
     * 
     * @param songID the id of the song
     * @param fromCallback used to skip fetching song info from MDM (after it has been done once)
    */
    void createDefault(int songID, bool fromCallback = false);

    /**
     * Creates a default with name unknown and artist unknown. Used for invalid song ids.
     * 
     * @param songID the id of the song
    */
    void createUnknownDefault(int songID);

    /**
     * Returns the savefile path
     * 
     * @return the path of the JSON
    */
    fs::path getJsonPath();

    static NongManager* get() {
        if (m_instance == nullptr) {
            m_instance = new NongManager;
            m_instance->retain();
        }

        return m_instance;
    }
};