#include "events/song_download_progress.hpp"

namespace jukebox {

namespace event {

SongDownloadProgress::SongDownloadProgress(int gdSongID, std::string m_uniqueID,
                                           float progress)
    : m_gdSongID(gdSongID), m_uniqueID(m_uniqueID), m_progress(progress) {};

int SongDownloadProgress::gdSongID() { return m_gdSongID; }
std::string SongDownloadProgress::uniqueID() { return m_uniqueID; }
float SongDownloadProgress::progress() { return m_progress; }

}  // namespace event

}  // namespace jukebox
