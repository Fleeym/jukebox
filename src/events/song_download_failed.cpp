#include "events/song_download_failed.hpp"

namespace jukebox {

namespace event {

SongDownloadFailed::SongDownloadFailed(int gdSongId, std::string uniqueId,
                                       std::string error)
    : m_gdSongId(gdSongId), m_uniqueId(uniqueId), m_error(error) {}
int SongDownloadFailed::gdSongId() const { return m_gdSongId; }
std::string SongDownloadFailed::uniqueId() const { return m_uniqueId; }
std::string SongDownloadFailed::error() const { return m_error; }

}  // namespace event

}  // namespace jukebox
