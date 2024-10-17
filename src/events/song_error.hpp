#pragma once

#include <string>

#include "Geode/loader/Event.hpp"

#include "managers/index_manager.hpp"

namespace jukebox {

namespace event {

class SongError final : public geode::Event {
private:
    bool m_notifyUser;
    std::string m_error;

protected:
    friend class ::jukebox::NongManager;
    friend class ::jukebox::IndexManager;

    SongError(bool notifyUser, std::string error);

public:
    std::string error();
    bool notifyUser();
};

}  // namespace event

}  // namespace jukebox
