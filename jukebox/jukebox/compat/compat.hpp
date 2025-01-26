#pragma once

#include <vector>

#include <jukebox/nong/nong.hpp>

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
