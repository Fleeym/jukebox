#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <map>
#include <unordered_map>
#include <vector>

#include <Geode/utils/web.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Event.hpp>

#include "../types/nong.hpp"
#include "../types/mod_state.hpp"
#include "../events/get_song_info_event.hpp"

using namespace geode::prelude;

namespace nongd {

enum class SongInfoGetAction {
    CreateDefault,
    FixDefault   
};

class NongManager : public CCObject {
protected:
    inline static NongManager* m_instance = nullptr;
    std::unique_ptr<ModState> m_state;
    std::mutex m_state_mutex;
    std::map<int, std::function<void(int)>> m_getSongInfoCallbacks;
    std::unordered_map<int, std::vector<SongInfoGetAction>> m_getSongInfoActions;
    EventListener<EventFilter<GetSongInfoEvent>> m_songInfoListener = { this, &NongManager::onSongInfoFetched };

    std::optional<std::vector<SongInfoGetAction>> getSongIDActions(int songID);
    void addSongIDAction(int songID, SongInfoGetAction action);
    void createDefaultCallback(SongInfoObject* obj);
    void setDefaultState();
    void backupBadJson(std::filesystem::path const& json);
    std::optional<std::unique_ptr<SongData>> loadFromPath(std::filesystem::path path);
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
    void addNong(std::unique_ptr<Nong> song, int songID);

    /**
     * Removes a NONG from the JSON of a songID
     * 
     * @param song the song to remove
     * @param songID the id of the song
    */
    void deleteNong(Nong* song, int songID);

    /**
     * Fetches all NONG data for a certain songID
     * 
     * @param songID the id of the song
     * @return the data from the JSON or nullopt if it wasn't created yet
    */
    std::optional<SongData*> getNongs(int songID);

    /**
     * Validates any local nongs that have an invalid path
     * 
     * @param songID the id of the song
     * 
     * @return an array of songs that were deleted as result of the validation
    */
    std::vector<Nong*> validateNongs(int songID);

    /**
     * Writes song data to the JSON
    */
    void save(int songID);

    /**
     * Saves song data for all ids 
    */
    void saveAll();

    /**
     * Removes all NONG data for a song ID
     * 
     * @param songID the id of the song
    */
    void deleteAll(int songID);

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
    */
    void createDefault(int songID);

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
    std::filesystem::path getJsonPath(int songID);

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
