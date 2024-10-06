#include <Geode/loader/ModEvent.hpp>

#include "Geode/DefaultInclude.hpp"
#include "Geode/loader/Mod.hpp"
#include "managers/nong_manager.hpp"
#include "managers/index_manager.hpp"
#include "ui/indexes_setting.hpp"

$execute {
    (void)Mod::get()->registerCustomSettingType(
        "indexes",
        &jukebox::IndexSetting::parse
    );
}

$on_mod(Loaded) {
    jukebox::NongManager::get();
    jukebox::IndexManager::get();
};
