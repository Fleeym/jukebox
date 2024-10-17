#pragma once

#include "Geode/loader/Event.hpp"

#include "managers/index_manager.hpp"
#include "managers/nong_manager.hpp"

namespace jukebox {

namespace event {

class SongStateChanged final : public geode::Event {
private:
    int m_gdSongID;

protected:
    friend class ::jukebox::NongManager;
    friend class ::jukebox::IndexManager;

    SongStateChanged(int gdSongID);

public:
    int gdSongID() const;
};

}  // namespace event

}  // namespace jukebox
