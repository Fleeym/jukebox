#pragma once

#include <vector>

#include "jukebox/nong/nong.hpp"

namespace jukebox::compat {

struct CompatManifest {
    int id;
    LocalSong defaultSong;
    LocalSong active;
    std::vector<LocalSong> songs;
};

}  // namespace jukebox::compat
