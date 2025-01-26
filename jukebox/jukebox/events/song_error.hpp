#pragma once

#include <string>

#include <Geode/loader/Event.hpp>

namespace jukebox {

class NongManager;

namespace event {

class SongError final : public geode::Event {
private:
    bool m_notifyUser;
    std::string m_error;

public:
    SongError(bool notifyUser, std::string error);

    std::string error();
    bool notifyUser();
};

}  // namespace event

}  // namespace jukebox
