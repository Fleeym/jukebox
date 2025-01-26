#include <jukebox/hooks/music_download_manager.hpp>

#include <optional>

#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/c++stl/string.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/string.hpp>

#include <jukebox/events/get_song_info.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/nong.hpp>

using namespace jukebox;

gd::string JBMusicDownloadManager::pathForSong(int id) {
    NongManager::get().m_currentlyPreparingNong = std::nullopt;
    std::optional<Nongs*> nongs = NongManager::get().getNongs(id);
    if (!nongs.has_value()) {
        return MusicDownloadManager::pathForSong(id);
    }
    Nongs* value = nongs.value();
    Song* active = value->active();
    if (!std::filesystem::exists(active->path().value())) {
        return MusicDownloadManager::pathForSong(id);
    }
    NongManager::get().m_currentlyPreparingNong = value;
#ifdef GEODE_IS_WINDOWS
    return geode::utils::string::wideToUtf8(active->path().value().c_str());
#else
    return active->path().value().string();
#endif
}

void JBMusicDownloadManager::onGetSongInfoCompleted(gd::string p1,
                                                    gd::string p2) {
    m_fields->overrideSongInfo = true;
    MusicDownloadManager::onGetSongInfoCompleted(p1, p2);
    int songID = std::stoi(p2);

    SongInfoObject* obj = this->getSongInfoObject(songID);
    m_fields->overrideSongInfo = false;

    if (obj == nullptr) {
        return;
    }

    event::GetSongInfo(obj->m_songName, obj->m_artistName, songID)
        .post();
}

SongInfoObject* JBMusicDownloadManager::getSongInfoObject(int id) {
    auto og = MusicDownloadManager::getSongInfoObject(id);
    if (og == nullptr) {
        return og;
    }

    if (m_fields->overrideSongInfo) {
        return og;
    }

    std::optional<Nongs*> opt = NongManager::get().getNongs(id);

    if (!opt) {
        return og;
    }

    Nongs* res = opt.value();
    Song* active = res->active();

    og->m_songName = active->metadata()->name;
    og->m_artistName = active->metadata()->artist;
    return og;
}
