#pragma once

#include <string>
#include <string_view>

#include <Geode/loader/Event.hpp>

namespace jukebox::event {

struct SongErrorData final {
private:
    bool m_notifyUser;
    std::string m_error;

public:
    SongErrorData(const bool notifyUser, std::string error) noexcept
        : m_notifyUser(notifyUser), m_error(std::move(error)) {}

    [[nodiscard]] std::string_view error() const noexcept { return m_error; }
    [[nodiscard]] bool notifyUser() const noexcept { return m_notifyUser; }
};

struct SongError : geode::Event<SongError, bool(const SongErrorData&)> {
    using Event::Event;
};

}  // namespace jukebox::event
