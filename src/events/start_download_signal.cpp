#include "events/start_download_signal.hpp"

#include "index.hpp"

using namespace jukebox::index;

namespace jukebox {

StartDownloadSignal::StartDownloadSignal(IndexSongMetadata* song, int gdId)
    : m_song(song), m_gdId(gdId) {}

IndexSongMetadata* StartDownloadSignal::song() { return m_song; }
int StartDownloadSignal::gdId() { return m_gdId; }

}  // namespace jukebox
