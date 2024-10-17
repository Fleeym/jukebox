#include "events/song_download_finished.hpp"

#include "index.hpp"
#include "nong.hpp"

using namespace jukebox::index;

namespace jukebox {

namespace event {

SongDownloadFinished::SongDownloadFinished(IndexSongMetadata* song,
                                           Song* destination)
    : m_song(song), m_destination(destination) {}
IndexSongMetadata* SongDownloadFinished::song() { return m_song; }
Song* SongDownloadFinished::destination() { return m_destination; }

}  // namespace event

}  // namespace jukebox
