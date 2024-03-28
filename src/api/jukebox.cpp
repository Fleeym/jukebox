#include "../../include/jukebox.hpp"
#include "../managers/nong_manager.hpp"

namespace fs = std::filesystem;

namespace jukebox {
  JUKEBOX_DLL void addNong(SongInfo const& song, int songID) {
    NongManager::get()->addNong(song, songID);
  }

  JUKEBOX_DLL void setActiveSong(SongInfo const& song, int songID) {
    NongData data = NongManager::get()->getNongs(songID).value();

    if (!fs::exists(song.path)) {
        return;
    }

    data.active = song.path;

    NongManager::get()->saveNongs(data, songID);
  }
}
