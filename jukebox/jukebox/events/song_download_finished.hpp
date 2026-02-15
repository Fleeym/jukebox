#pragma once

#include <optional>

#include <Geode/loader/Event.hpp>

#include <jukebox/managers/index_manager.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox::event {

struct SongDownloadFinishedData final {
private:
    std::optional<index::IndexSongMetadata*> m_indexSource;
    Song* m_destination;

public:
    SongDownloadFinishedData(const std::optional<index::IndexSongMetadata*> indexSource, Song* destination) noexcept
        : m_indexSource(indexSource), m_destination(destination) {}

    [[nodiscard]] std::optional<index::IndexSongMetadata*> indexSource() const noexcept { return m_indexSource; }
    [[nodiscard]] Song* destination() const noexcept { return m_destination; }
};

struct SongDownloadFinished final : geode::Event<SongDownloadFinished, bool(const SongDownloadFinishedData&)> {
    using Event::Event;
};

}  // namespace jukebox::event
