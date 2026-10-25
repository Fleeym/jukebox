#pragma once

#include <Geode/loader/Event.hpp>

namespace jukebox::event {

struct IndexesLoaded final : geode::Event<IndexesLoaded, bool()> {
    using Event::Event;
};

}  // namespace jukebox::event
