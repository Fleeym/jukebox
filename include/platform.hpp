#pragma once

#ifdef GEODE_IS_WINDOWS
    #ifdef FLEYM_JUKEBOX_EXPORTING
        #define JUKEBOX_DLL __declspec(dllexport)
    #else
        #define JUKEBOX_DLL __declspec(dllimport)
    #endif
#else
    #define JUKEBOX_DLL __attribute__((visibility("default")))
#endif