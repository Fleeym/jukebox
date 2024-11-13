#pragma once

#include "Geode/loader/Event.hpp"

#include "nong.hpp"

namespace jukebox {

namespace event {

class NongDeleted : public geode::Event {
protected:
    std::string m_uniqueId;
    int m_gdId;

    friend class ::jukebox::Nongs;

    NongDeleted(const std::string& uniqueId, int gdId);

public:
    std::string uniqueId() const;
    int gdId() const;
};

}  // namespace event

}  // namespace jukebox
