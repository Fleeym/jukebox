#pragma once

#include <string>

#include <Geode/loader/Event.hpp>

#include <jukebox/hooks/music_download_manager.hpp>

namespace jukebox {

namespace event {

class GetSongInfo : public geode::Event {
private:
    std::string m_songName;
    std::string m_artistName;
    int m_gdSongID;

protected:
    friend class ::JBMusicDownloadManager;

    GetSongInfo(std::string songName, std::string artistName, int gdSongID);

public:
    std::string songName();
    std::string artistName();
    int gdSongID();
};

}  // namespace event

}  // namespace jukebox
