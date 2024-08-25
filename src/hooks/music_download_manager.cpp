#include <Geode/utils/string.hpp>

#include "music_download_manager.hpp"
#include "../managers/nong_manager.hpp"
#include "../../include/nong.hpp"
#include "../events/get_song_info_event.hpp"
#include "Geode/binding/GameLevelManager.hpp"
#include "Geode/binding/MusicDownloadManager.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/cocos/CCDirector.h"

using namespace jukebox;

gd::string JBMusicDownloadManager::pathForSong(int id) {
    NongManager::get()->m_currentlyPreparingNong = std::nullopt;
    auto nongs = NongManager::get()->getNongs(id);
    if (!nongs.has_value()) {
        return MusicDownloadManager::pathForSong(id);
    }
    auto value = nongs.value();
    auto active = value->activeNong();
  		if (!std::filesystem::exists(active.path().value())) {
        return MusicDownloadManager::pathForSong(id);
  		}
    NongManager::get()->m_currentlyPreparingNong = value;
    #ifdef GEODE_IS_WINDOWS
    return geode::utils::string::wideToUtf8(active.path().value().c_str());
    #else
    return active.path().value().string();
    #endif
}

void JBMusicDownloadManager::onGetSongInfoCompleted(gd::string p1, gd::string p2) {
    MusicDownloadManager::onGetSongInfoCompleted(p1, p2);
    auto songID = std::stoi(p2);

    // CCArray* keys = GameLevelManager::sharedState()->responseToDict(p1, true)->allKeys();
    // // Example of p1: 1~|~945695~|~2~|~Tennobyte - Fly Away~|~3~|~51164~|~4~|~Tennobyte~|~5~|~6.47~|~6~|~~|~10~|~https%3A%2F%2Faudio.ngfiles.com%2F945000%2F945695_Tennobyte---Fly-Away.mp3%3Ff1593523523~|~7~|~~|~11~|~0~|~12~|~~|~13~|~~|~14~|~0
    // // Values are the keys in the Dictionary. For some reason..
    // // It also seems like only sometimes empty values are ommited.
    CCDictionary* dict = GameLevelManager::sharedState()->responseToDict(p1, true);

    // for (int i = 0; i < keys->count(); i++)  {
    //     log::info("{}", keys->objectAtIndex(i));
    //     log::info("{}", static_cast<CCString*>(keys->objectAtIndex(i))->getCString());
    // }
    // GetSongInfoEvent(this->getSongInfoObject(songID), songID).post();
    GetSongInfoEvent(
        static_cast<CCString*>(dict->allKeys()->objectAtIndex(3))->getCString(),
        static_cast<CCString*>(dict->allKeys()->objectAtIndex(7))->getCString(),
        songID
    ).post();
}

SongInfoObject* JBMusicDownloadManager::getSongInfoObject(int id) {
    auto og = MusicDownloadManager::getSongInfoObject(id);
    if (og == nullptr) {
        return og;
    }
    auto res = NongManager::get()->getNongs(id);
    if (res.has_value()) {
        auto active = res.value()->activeNong();
        og->m_songName = active.metadata()->m_name;
        og->m_artistName = active.metadata()->m_artist;
    }
    return og;
}
