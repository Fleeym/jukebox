#include <Geode/loader/ModEvent.hpp>

#include "managers/nong_manager.hpp"

$on_mod(Loaded) {
    NongManager::get()->loadSongs();
};

$on_mod(DataSaved) {
    NongManager::get()->writeJson();
}