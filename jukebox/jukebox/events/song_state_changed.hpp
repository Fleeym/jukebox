#pragma once

#include <Geode/loader/Event.hpp>

#include <jukebox/nong/nong.hpp>

namespace jukebox {

namespace event {

class SongStateChanged final : public geode::Event {
private:
    Nongs* m_nongs;

public:
    SongStateChanged(Nongs* nongs);
    Nongs* nongs() const;
};

}  // namespace event

}  // namespace jukebox
