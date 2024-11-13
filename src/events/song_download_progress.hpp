#pragma once

#include <string>

#include "Geode/loader/Event.hpp"

#include "managers/index_manager.hpp"
#include "managers/nong_manager.hpp"

namespace jukebox {

namespace event {

class SongDownloadProgress final : public geode::Event {
private:
    int m_gdSongID;
    std::string m_uniqueID;
    float m_progress;

protected:
    friend class ::jukebox::NongManager;
    friend class ::jukebox::IndexManager;

    SongDownloadProgress(int gdSongID, std::string m_uniqueID, float progress);

public:
    int gdSongID();
    std::string uniqueID();
    float progress();
};

}  // namespace event

}  // namespace jukebox
