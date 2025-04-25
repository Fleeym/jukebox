#include <jukebox/events/start_download.hpp>

#include <jukebox/nong/index.hpp>

using namespace jukebox::index;

namespace jukebox {

namespace event {

StartDownload::StartDownload(int gdSongID, std::string uniqueID)
    : m_gdSongID(gdSongID), m_uniqueID(uniqueID) {}

int StartDownload::gdSongID() { return m_gdSongID; }
std::string StartDownload::uniqueID() { return m_uniqueID; }

}  // namespace event

}  // namespace jukebox
