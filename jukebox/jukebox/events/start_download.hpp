#pragma once

#include <string>
#include <string_view>

#include <Geode/loader/Event.hpp>

namespace jukebox::event {

struct StartDownloadData final {
protected:
    int m_gdId;
    std::string m_uniqueID;

public:
    StartDownloadData(const int gdId, std::string uniqueID) noexcept : m_gdId(gdId), m_uniqueID(std::move(uniqueID)) {}

    [[nodiscard]] int gdId() const noexcept { return m_gdId; }
    [[nodiscard]] std::string_view uniqueID() const noexcept { return m_uniqueID; }
};

struct StartDownload : geode::Event<StartDownload, bool(const StartDownloadData&)> {
    using Event::Event;
};

}  // namespace jukebox::event
