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
    */
    JUKEBOX_DLL void setActiveSong(SongInfo const& song, int songID);
}
