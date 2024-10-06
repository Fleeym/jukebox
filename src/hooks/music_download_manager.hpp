#pragma once

#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>

struct JBMusicDownloadManager
    : geode::Modify<JBMusicDownloadManager, MusicDownloadManager> {
    gd::string pathForSong(int id);
    void onGetSongInfoCompleted(gd::string p1, gd::string p2);
    SongInfoObject* getSongInfoObject(int id);
};
