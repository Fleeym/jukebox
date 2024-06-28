#pragma once

#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Event.hpp>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <Geode/utils/Task.hpp>

#include "../types/song_info.hpp"
#include "../types/nong_state.hpp"
#include "../types/index.hpp"
#include "../events/get_song_info_event.hpp"

using namespace geode::prelude;

namespace jukebox {

enum class SongInfoGetAction {
    CreateDefault,
    FixDefault   
};

class NongManager : public CCObject {
protected:
    inline static NongManager* m_instance = nullptr;
    NongState m_state;
    std::unordered_map<int, std::function<void(int)>> m_getSongInfoCallbacks;
    std::unordered_map<int, std::unordered_set<SongInfoGetAction>> m_getSongInfoActions;
    EventListener<EventFilter<GetSongInfoEvent>> m_songInfoListener = { this, &NongManager::onSongInfoFetched };
    std::vector<Index> m_indexes;
    bool m_initialized = false;

    std::optional<std::unordered_set<SongInfoGetAction>> getSongIDActions(int songID);
    void addSongIDAction(int songID, SongInfoGetAction action);
    void createDefaultCallback(SongInfoObject* obj, int songID = 0);
    void setDefaultState();
    void backupCurrentJSON();
public:
    using MultiAssetSizeTask = Task<std::string>;
    std::optional<SongInfo> m_currentlyPreparingNong;

    bool initialized() { return m_initialized; }

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
     * @param deleteFile whether to delete the corresponding audio file created by Jukebox
    */
    void deleteNong(SongInfo const& song, int songID, bool deleteFile = true);

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
     * Runs on a separate thread. Returns a task that will resolve to the total size.
     * 
     * @param songs string of song ids, separated by commas
     * @param sfx string of sfx ids, separated by commas
    */
    MultiAssetSizeTask getMultiAssetSizes(std::string songs, std::string sfx);

    /**
     * Fetches song info for an id and creates the default entry in the json.
     * 
     * @param songID the id of the song
    */
    void createDefault(SongInfoObject* object, int songID, bool robtop);

    /**
     * Creates a default with name unknown and artist unknown. Used for invalid song ids.
     * 
     * @param songID the id of the song
    */
    void createUnknownDefault(int songID);

    /**
     * Checks if a song ID has actions associated to it 
     * 
     * @param songID the id of the song
    */
    bool hasActions(int songID);

    /**
     * Checks if the default song is being fixed for a song ID 
     *
     * @param songID the id of the song
    */
    bool isFixingDefault(int songID);

    /**
     * Returns the savefile path
     * 
     * @return the path of the JSON
    */
    fs::path getJsonPath();

    /**
     * Add actions needed to fix a broken song default
     * @param songID id of the song
    */
    void prepareCorrectDefault(int songID);
    /**
     * Callback that runs when a fix song default action runs
     * @param songID id of the song
    */
    void fixDefault(SongInfoObject* obj);

    /**
     * Marks a song ID as having an invalid default
     * @param songID id of the song
    */
    void markAsInvalidDefault(int songID);

    // /**
    //  * Get the indexes used by Jukebox
    // */
    // std::vector<Index> getIndexes();

    ListenerResult onSongInfoFetched(GetSongInfoEvent* event);

    static NongManager* get() {
        if (m_instance == nullptr) {
            m_instance = new NongManager;
            m_instance->retain();
        }

        return m_instance;
    }
};

}
