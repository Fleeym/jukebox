#include <jukebox/events/song_state_changed.hpp>

namespace jukebox {

namespace event {

SongStateChanged::SongStateChanged(Nongs* nongs) : m_nongs(nongs) {};

Nongs* SongStateChanged::nongs() const { return m_nongs; };

}  // namespace event

}  // namespace jukebox
