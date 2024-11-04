#pragma once

#include <vector>

#include "nong.hpp"

namespace jukebox {

namespace compat {

struct CompatManifest {
    int id;
    LocalSong defaultSong;
    LocalSong active;
    std::vector<LocalSong> songs;
};

}  // namespace compat

}  // namespace jukebox
