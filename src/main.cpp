#include <memory>

#include <Geode/Result.hpp>
#include "Geode/DefaultInclude.hpp"
#include "Geode/loader/Mod.hpp"
#include "Geode/loader/ModEvent.hpp"

#include "managers/index_manager.hpp"
#include "managers/nong_manager.hpp"
#include "ui/indexes_setting.hpp"

$execute {
    /*(void)Mod::get()->registerCustomSettingType(*/
    /*    "indexes",*/
    /*    [](const std::string& key, const std::string& modID,*/
    /*       const matjson::Value& json)*/
    /*        -> Result<std::shared_ptr<jukebox::IndexSetting>> {*/
    /*        return jukebox::IndexSetting::parse(key, modID, json);*/
    /*    });*/
}

$on_mod(Loaded) {
    jukebox::NongManager::get().init();
    jukebox::IndexManager::get().init();
};
