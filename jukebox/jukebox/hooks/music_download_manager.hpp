#pragma once

#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/c++stl/string.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>  // IWYU pragma: keep

struct JBMusicDownloadManager
    : geode::Modify<JBMusicDownloadManager, MusicDownloadManager> {
    struct Fields {
        bool overrideSongInfo = false;
    };
    
    gd::string pathForSong(int id);
    void onGetSongInfoCompleted(gd::string p1, gd::string p2);
    SongInfoObject* getSongInfoObject(int id);
};
