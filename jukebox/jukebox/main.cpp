#include <Geode/DefaultInclude.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>

#include <jukebox/managers/index_manager.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/ui/indexes_setting.hpp>

$execute {
    (void)Mod::get()->registerCustomSettingType("indexes",
                                                &jukebox::IndexSetting::parse);
}

$on_mod(Loaded) {
    jukebox::NongManager::get().init();
    jukebox::IndexManager::get().init();
};
