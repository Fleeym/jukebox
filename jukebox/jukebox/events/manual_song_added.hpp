#pragma once

#include <Geode/loader/Event.hpp>

#include <jukebox/nong/nong.hpp>

namespace jukebox::event {

struct ManualSongAddedData {
private:
    Nongs* m_nongs;
    Song* m_new;

public:
    explicit ManualSongAddedData(Nongs* nongs, Song* newSong) noexcept
        : m_nongs(nongs), m_new(newSong) {}

    [[nodiscard]] Nongs* nongs() const noexcept { return m_nongs; }
    [[nodiscard]] Song* song() const noexcept { return m_new; }
};

struct ManualSongAdded final
    : geode::Event<ManualSongAdded, bool(const ManualSongAddedData&)> {
    using Event::Event;
};

}  // namespace jukebox::event
