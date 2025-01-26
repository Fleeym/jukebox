#pragma once

#include <Geode/loader/Event.hpp>

#include <jukebox/nong/index.hpp>

namespace jukebox {

namespace event {

class StartDownload final : public geode::Event {
protected:
    index::IndexSongMetadata* m_song = nullptr;
    int m_gdId;

public:
    StartDownload(index::IndexSongMetadata* song, int gdId);

    index::IndexSongMetadata* song();
    int gdId();
};

}  // namespace event

}  // namespace jukebox
