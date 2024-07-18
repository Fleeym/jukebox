#include <Geode/loader/ModEvent.hpp>

#include "managers/nong_manager.hpp"
#include "managers/index_manager.hpp"

$on_mod(Loaded) {
    // TODO
    // jukebox::NongManager::get()->loadSongs();
    jukebox::IndexManager::get();
};

$on_mod(DataSaved) {
    // TODO
    // jukebox::NongManager::get()->writeJson();
}
