#pragma once

#include "Geode/loader/Event.hpp"
#include "../managers/nong_manager.hpp"
#include "../managers/index_manager.hpp"
#include "../hooks/music_download_manager.hpp"

using namespace geode::prelude;

namespace jukebox {

class GetSongInfoEvent : public Event {
private:
    std::string m_songName;
    std::string m_artistName;
    int m_gdSongID;

protected:
    friend class ::JBMusicDownloadManager;

    GetSongInfoEvent(std::string songName, std::string artistName, int gdSongID)
        : m_songName(songName), m_artistName(artistName), m_gdSongID(gdSongID) {}

public:
    std::string songName() { return m_songName; }
    std::string artistName() { return m_artistName; }
    int gdSongID() { return m_gdSongID; }
};

using GetSongInfoFilter = EventFilter<GetSongInfoEvent>;

}
