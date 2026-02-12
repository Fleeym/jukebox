#pragma once

#include <string>
#include <string_view>
#include <utility>

#include <Geode/loader/Event.hpp>

namespace jukebox::event {

struct SongDownloadFailedData final {
private:
    int m_gdId;
    std::string m_uniqueId;
    std::string m_error;

public:
    SongDownloadFailedData(const int gdId, std::string uniqueId, std::string error) noexcept
        : m_gdId(gdId), m_uniqueId(std::move(uniqueId)), m_error(std::move(error)) {}

    [[nodiscard]] int gdId() const noexcept { return m_gdId; }
    [[nodiscard]] std::string_view uniqueId() const noexcept { return m_uniqueId; }
    [[nodiscard]] std::string_view error() const noexcept { return m_error; }
};

struct SongDownloadFailed : geode::GlobalEvent<SongDownloadFailed, bool(const SongDownloadFailedData&), int> {
    using GlobalEvent::GlobalEvent;
};

}  // namespace jukebox::event
