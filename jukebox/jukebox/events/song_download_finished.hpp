#pragma once

#include <optional>

#include <Geode/loader/Event.hpp>

#include <jukebox/nong/index.hpp>
#include <jukebox/managers/index_manager.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox {

namespace event {

class SongDownloadFinished final : public geode::Event {
protected:
    friend class ::jukebox::IndexManager;

    std::optional<index::IndexSongMetadata*> m_indexSource = nullptr;
    Song* m_destination;

    SongDownloadFinished(std::optional<index::IndexSongMetadata*> indexSource,
                         Song* destination);

public:
    std::optional<index::IndexSongMetadata*> indexSource();
    Song* destination();
};

}  // namespace event

}  // namespace jukebox
