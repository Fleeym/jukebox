#include "../../include/jukebox.hpp"
#include "../managers/nong_manager.hpp"

namespace fs = std::filesystem;

namespace jukebox {
  JUKEBOX_DLL void addNong(SongInfo const& song, int songID) {
    NongManager::get()->addNong(song, songID);
  }

  JUKEBOX_DLL void setActiveNong(SongInfo const& song, int songID, const std::optional<Ref<CustomSongWidget>>& customSongWidget) {
    NongData data = NongManager::get()->getNongs(songID).value();

    if (!fs::exists(song.path)) {
        return;
    }

    data.active = song.path;

    NongManager::get()->saveNongs(data, songID);

    if (customSongWidget.has_value()) {
      const Ref<CustomSongWidget> widget = customSongWidget.value();

      widget->m_songInfoObject->m_artistName = song.authorName;
      widget->m_songInfoObject->m_songName = song.songName;
      if (song.songUrl != "local") {
          widget->m_songInfoObject->m_songUrl = song.songUrl;
      }
      widget->updateSongObject(widget->m_songInfoObject);
    }
  }

  JUKEBOX_DLL std::optional<SongInfo> getActiveNong(int songID) {
    return NongManager::get()->getActiveNong(songID);
  }

  JUKEBOX_DLL void deleteNong(SongInfo const& song, int songID) {
    NongManager::get()->deleteNong(song, songID);
  }

  JUKEBOX_DLL std::optional<SongInfo> getDefaultNong(int songID) {
    return NongManager::get()->getDefaultNong(songID);
  }
}
