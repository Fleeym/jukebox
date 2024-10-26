#pragma once

#include "Geode/loader/Event.hpp"

namespace jukebox {

namespace event {

class SongStateChanged final : public geode::Event {
private:
    int m_gdSongID;

public:
    SongStateChanged(int gdSongID);
    int gdSongID() const;
};

}  // namespace event

}  // namespace jukebox
