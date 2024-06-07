#pragma once

#include "../src/types/song_info.hpp"
#include <optional>

#ifdef GEODE_IS_WINDOWS
    #ifdef FLEYM_JUKEBOX_EXPORTING
        #define JUKEBOX_DLL __declspec(dllexport)
    #else
        #define JUKEBOX_DLL __declspec(dllimport)
    #endif
#else
    #define JUKEBOX_DLL __attribute__((visibility("default")))
#endif

namespace jukebox {
    /**
     * Adds a NONG to the JSON of a songID
     * 
     * @param song the song to add
     * @param songID the id of the song
    */
    JUKEBOX_DLL void addNong(SongInfo const& song, int songID);

    /**
     * Sets the active song of a songID
     * 
     * @param song the song to set as active
     * @param songID the id of the song
     * @param customSongWidget optional custom song widget to update
    */
    JUKEBOX_DLL void setActiveNong(SongInfo const& song, int songID, const std::optional<geode::Ref<CustomSongWidget>>& customSongWidget);

    /**
     * Gets the active song of a songID
     * 
     * @param songID the id of the song
    */
    JUKEBOX_DLL std::optional<SongInfo> getActiveNong(int songID);

    /**
     * Deletes a NONG from the JSON of a songID
     * 
     * @param song the song to delete
     * @param songID the id of the replaced song
     * @param deleteFile whether to delete the corresponding audio file created by Jukebox
    */
    JUKEBOX_DLL void deleteNong(SongInfo const& song, int songID, bool deleteFile = true);

    /**
     * Sets the song of the songID as the default song provided by GD
     * 
     * @param songID the id of the song
    */
    JUKEBOX_DLL std::optional<SongInfo> getDefaultNong(int songID);
}
