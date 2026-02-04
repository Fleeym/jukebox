#pragma once

#include <string>
#include <string_view>

#include <Geode/loader/Event.hpp>

namespace jukebox::event {

struct SongDownloadProgressData final {
private:
    float m_gdId;
    std::string m_uniqueID;
    float m_progress;

public:
    SongDownloadProgressData(const int gdId, std::string m_uniqueID, float progress)
        : m_gdId(gdId), m_uniqueID(std::move(m_uniqueID)), m_progress(progress) {}

    [[nodiscard]] int gdId() const noexcept { return m_gdId; }
    [[nodiscard]] std::string_view uniqueID() const noexcept { return m_uniqueID; }
    [[nodiscard]] float progress() const noexcept { return m_progress; }
};

struct SongDownloadProgress : geode::GlobalEvent<SongDownloadProgress, bool(const SongDownloadProgressData&), int> {
    using GlobalEvent::GlobalEvent;
};

}  // namespace jukebox::event
