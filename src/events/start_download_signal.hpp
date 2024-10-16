#pragma once

#include "Geode/loader/Event.hpp"
#include "index.hpp"

using namespace geode::prelude;
using namespace jukebox::index;

namespace jukebox {

class StartDownloadSignal : public Event {
protected:
    IndexSongMetadata* m_song = nullptr;
    int m_gdId;
public:
    StartDownloadSignal(IndexSongMetadata* song, int gdId);

    IndexSongMetadata* song();
    int gdId();
};

}  // namespace jukebox
