#include <jukebox/events/start_download.hpp>

#include <jukebox/nong/index.hpp>

using namespace jukebox::index;

namespace jukebox {

namespace event {

StartDownload::StartDownload(IndexSongMetadata* song, int gdId)
    : m_song(song), m_gdId(gdId) {}

IndexSongMetadata* StartDownload::song() { return m_song; }
int StartDownload::gdId() { return m_gdId; }

}  // namespace event

}  // namespace jukebox
