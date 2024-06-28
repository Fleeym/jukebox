#include <Geode/loader/ModEvent.hpp>

#include "managers/nong_manager.hpp"
#include "ui/indexes_setting.hpp"

$on_mod(Loaded) {
    jukebox::NongManager::get()->loadSongs();
    Mod::get()->addCustomSetting<IndexesSettingValue>(
        "indexes", Mod::get()
                       ->getSettingDefinition("indexes")
                       ->get<CustomSetting>()
                       ->json->get<std::vector<Index>>("default"));
};

$on_mod(DataSaved) {
    jukebox::NongManager::get()->writeJson();
}