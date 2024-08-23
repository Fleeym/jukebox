#include <Geode/loader/ModEvent.hpp>

#include "managers/nong_manager.hpp"
#include "managers/index_manager.hpp"

$on_mod(Loaded) {
    jukebox::IndexManager::get();
};
