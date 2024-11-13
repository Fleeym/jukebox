#include "events/get_song_info.hpp"
#include <string>

namespace jukebox::event {

GetSongInfo::GetSongInfo(std::string songName, std::string artistName,
                         int songId)
    : m_songName(songName), m_artistName(artistName), m_gdSongID(songId) {}
std::string GetSongInfo::songName() { return m_songName; }
std::string GetSongInfo::artistName() { return m_artistName; }
int GetSongInfo::gdSongID() { return m_gdSongID; }

}  // namespace jukebox::event
