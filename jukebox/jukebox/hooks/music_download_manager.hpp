#pragma once

#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <string>
#include <Geode/modify/MusicDownloadManager.hpp>  // IWYU pragma: keep

struct JBMusicDownloadManager
    : geode::Modify<JBMusicDownloadManager, MusicDownloadManager> {
    struct Fields {
        bool overrideSongInfo = false;
    };
    
    std::string pathForSong(int id);
    void onGetSongInfoCompleted(std::string p1, std::string p2);
    SongInfoObject* getSongInfoObject(int id);
};
