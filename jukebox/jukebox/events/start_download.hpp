#pragma once

#include <Geode/loader/Event.hpp>

#include <jukebox/nong/index.hpp>

namespace jukebox {

namespace event {

class StartDownload final : public geode::Event {
protected:
    int m_gdSongID;
    std::string m_uniqueID;

public:
    StartDownload(int gdSongID, std::string uniqueID);

    int gdSongID();
    std::string uniqueID();
};

}  // namespace event

}  // namespace jukebox
