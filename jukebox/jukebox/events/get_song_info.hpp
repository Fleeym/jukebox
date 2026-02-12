#pragma once

#include <string>
#include <string_view>

#include <Geode/loader/Event.hpp>

namespace jukebox::event {

struct GetSongInfoData final {
private:
    int m_gdId;
    std::string m_songName;
    std::string m_artistName;

public:
    GetSongInfoData(const int gdId, std::string songName, std::string artistName) noexcept
        : m_gdId(gdId), m_songName(std::move(songName)), m_artistName(std::move(artistName)) {}

    [[nodiscard]] int gdId() const noexcept { return m_gdId; }
    [[nodiscard]] std::string_view songName() const noexcept { return m_songName; }
    [[nodiscard]] std::string_view artistName() const noexcept { return m_artistName; }
};

struct GetSongInfo final : geode::GlobalEvent<GetSongInfo, bool(const GetSongInfoData&), int> {
    using GlobalEvent::GlobalEvent;
};

}  // namespace jukebox::event
