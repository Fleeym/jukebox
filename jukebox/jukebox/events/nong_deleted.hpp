#pragma once

#include <string>
#include <string_view>
#include <utility>

#include <Geode/loader/Event.hpp>

namespace jukebox::event {

struct NongDeletedData final {
protected:
    int m_gdId;
    std::string m_uniqueId;

public:
    NongDeletedData(const int gdId, std::string uniqueId) noexcept : m_gdId(gdId), m_uniqueId(std::move(uniqueId)) {}

    [[nodiscard]] int gdId() const noexcept { return m_gdId; }
    [[nodiscard]] std::string_view uniqueId() const noexcept { return m_uniqueId; }
};

struct NongDeleted : geode::GlobalEvent<NongDeleted, bool(const NongDeletedData&), int> {
    using GlobalEvent::GlobalEvent;
};

}  // namespace jukebox::event
