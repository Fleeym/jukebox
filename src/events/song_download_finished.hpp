#pragma once

#include "Geode/loader/Event.hpp"

#include "index.hpp"
#include "managers/index_manager.hpp"
#include "nong.hpp"

namespace jukebox {

namespace event {

class SongDownloadFinished final : public geode::Event {
protected:
    friend class ::jukebox::IndexManager;

    index::IndexSongMetadata* m_song = nullptr;
    Song* m_destination;

    SongDownloadFinished(index::IndexSongMetadata* song, Song* destination);

public:
    index::IndexSongMetadata* song();
    Song* destination();
};

}  // namespace event

}  // namespace jukebox
