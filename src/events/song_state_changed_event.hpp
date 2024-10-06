#pragma once

#include "../managers/index_manager.hpp"
#include "../managers/nong_manager.hpp"
#include "Geode/loader/Event.hpp"

using namespace geode::prelude;

namespace jukebox {

class SongStateChangedEvent final : public Event {
private:
    int m_gdSongID;

protected:
    friend class NongManager;
    friend class IndexManager;

    SongStateChangedEvent(int gdSongID) : m_gdSongID(gdSongID) {};

public:
    int gdSongID() const { return m_gdSongID; };
};

}  // namespace jukebox