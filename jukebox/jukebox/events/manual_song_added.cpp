#include <jukebox/events/manual_song_added.hpp>

namespace jukebox {

namespace event {

ManualSongAdded::ManualSongAdded(Nongs* n, Song* s) : m_nongs(n), m_new(s) {}
Nongs* ManualSongAdded::nongs() const { return m_nongs; }
Song* ManualSongAdded::song() const { return m_new; }

}  // namespace event

}  // namespace jukebox
