#pragma once

#include <string>

#include "Geode/loader/Event.hpp"

#include "managers/index_manager.hpp"
#include "managers/nong_manager.hpp"

using namespace geode::prelude;

namespace jukebox {

class SongErrorEvent final : public Event {
private:
    bool m_notifyUser;
    std::string m_error;

protected:
    friend class NongManager;
    friend class IndexManager;

    template <typename... Args>
    SongErrorEvent(bool notifyUser, fmt::format_string<Args...> format,
                   Args&&... args)
        : m_error(fmt::format(format, std::forward<Args>(args)...)),
          m_notifyUser(notifyUser){};

public:
    std::string error() { return m_error; }
    bool notifyUser() { return m_notifyUser; }
};

}  // namespace jukebox
