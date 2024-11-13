#include "events/song_error.hpp"

namespace jukebox {

namespace event {

SongError::SongError(bool notify, std::string error)
    : m_error(error), m_notifyUser(notify) {}

std::string SongError::error() { return m_error; }
bool SongError::notifyUser() { return m_notifyUser; }

}  // namespace event

}  // namespace jukebox
