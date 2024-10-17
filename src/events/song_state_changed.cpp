#include "events/song_state_changed.hpp"

namespace jukebox {

namespace event {

SongStateChanged::SongStateChanged(int gdSongID) : m_gdSongID(gdSongID) {};

int SongStateChanged::gdSongID() const { return m_gdSongID; };

}  // namespace event

}  // namespace jukebox
