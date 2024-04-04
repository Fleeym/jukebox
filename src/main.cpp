#include <Geode/loader/ModEvent.hpp>

#include "managers/nong_manager.hpp"

$on_mod(Loaded) {
    jukebox::NongManager::get()->loadSongs();
};

$on_mod(DataSaved) {
    jukebox::NongManager::get()->writeJson();
}