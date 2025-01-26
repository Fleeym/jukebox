#include <jukebox/events/song_download_finished.hpp>

#include <optional>

#include <jukebox/nong/index.hpp>
#include <jukebox/nong/nong.hpp>

using namespace jukebox::index;

namespace jukebox {

namespace event {

SongDownloadFinished::SongDownloadFinished(
    std::optional<index::IndexSongMetadata*> song, Song* destination)
    : m_indexSource(song), m_destination(destination) {}
std::optional<IndexSongMetadata*> SongDownloadFinished::indexSource() {
    return m_indexSource;
}
Song* SongDownloadFinished::destination() { return m_destination; }

}  // namespace event

}  // namespace jukebox
