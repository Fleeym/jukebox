#pragma once

#include <Geode/loader/Event.hpp>

#include <jukebox/nong/nong.hpp>

namespace jukebox {

namespace event {

class ManualSongAdded final : public geode::Event {
private:
    Nongs* m_nongs;
    Song* m_new;

public:
    ManualSongAdded(Nongs* nongs, Song* newSong);
    Nongs* nongs() const;
    Song* song() const;
};

}  // namespace event

}  // namespace jukebox
