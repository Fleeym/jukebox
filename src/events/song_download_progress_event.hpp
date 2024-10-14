#pragma once

#include <string>

#include "Geode/loader/Event.hpp"

#include "managers/index_manager.hpp"
#include "managers/nong_manager.hpp"

using namespace geode::prelude;

namespace jukebox {

class SongDownloadProgressEvent final : public Event {
private:
    int m_gdSongID;
    std::string m_uniqueID;
    float m_progress;

protected:
    friend class NongManager;
    friend class IndexManager;

    SongDownloadProgressEvent(int gdSongID, std::string m_uniqueID,
                              float progress)
        : m_gdSongID(gdSongID), m_uniqueID(m_uniqueID), m_progress(progress) {};

public:
    int gdSongID() { return m_gdSongID; }
    std::string uniqueID() { return m_uniqueID; }
    float progress() { return m_progress; }
};

using SongDownloadProgressFilter = EventFilter<SongDownloadProgressEvent>;

}  // namespace jukebox
