#pragma once

#include <string>
#include <string_view>

#include <Geode/loader/Event.hpp>

namespace jukebox::events {

struct FileDownloadProgressData final {
private:
    float m_progress;
    std::string m_url;

public:
    FileDownloadProgressData(std::string url, const float progress) noexcept
        : m_progress{progress}, m_url(std::move(url)) {}

    [[nodiscard]] float progress() const noexcept { return m_progress; }
    [[nodiscard]] std::string_view url() const noexcept { return m_url; }
};

struct FileDownloadProgress
    : geode::GlobalEvent<FileDownloadProgress, bool(const FileDownloadProgressData&), std::string> {
    using GlobalEvent::GlobalEvent;
};

}  // namespace jukebox::events
