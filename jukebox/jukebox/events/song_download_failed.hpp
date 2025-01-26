#pragma once

#include <string>

#include <Geode/loader/Event.hpp>

namespace jukebox {

namespace event {

class SongDownloadFailed final : public geode::Event {
private:
    int m_gdSongId;
    std::string m_uniqueId;
    std::string m_error;

public:
    SongDownloadFailed(int gdSongId, std::string uniqueId, std::string error);
    int gdSongId() const;
    std::string uniqueId() const;
    std::string error() const;
};

}  // namespace event

}  // namespace jukebox
