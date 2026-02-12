#pragma once

#include <Geode/loader/Event.hpp>

#include <jukebox/nong/nong.hpp>

namespace jukebox::event {

struct SongStateChangedData final {
private:
    Nongs* m_nongs;

public:
    explicit SongStateChangedData(Nongs* nongs) noexcept : m_nongs(nongs) {}

    [[nodiscard]] Nongs* nongs() const noexcept { return m_nongs; }
};

struct SongStateChanged : geode::SimpleEvent<SongStateChanged, const SongStateChangedData&> {
    using SimpleEvent::SimpleEvent;
};

}  // namespace jukebox::event
