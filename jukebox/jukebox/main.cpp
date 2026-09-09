#include <Geode/DefaultInclude.hpp>
#include <Geode/Result.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/Popup.hpp>

#include <jukebox/managers/index_manager.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/ui/indexes_setting.hpp>

using namespace geode::prelude;

$execute { (void)Mod::get()->registerCustomSettingType("indexes", &jukebox::IndexSetting::parse); }

$on_mod(Loaded) {
    jukebox::IndexManager::get().init();
    jukebox::NongManager::get().init();

    ButtonSettingPressedEventV3(Mod::get(), "clear-cache")
        .listen([](auto buttonKey) {
            if (buttonKey == "clear-index-cache") {
                createQuickPopup(
                    "Clear cache",
                    "Are you sure you want to clear <cr>ALL</c> index caches? Only do this if you are having trouble "
                    "with "
                    "index songs.",
                    "No", "Yes", [](FLAlertLayer*, bool btn2) {
                        if (!btn2) {
                            return;
                        }
                        jukebox::IndexManager::get().clearCaches();
                        geode::Notification::create("Caches cleared successfully", NotificationIcon::Success)->show();
                    });
            }  // ... + others
        })
        .leak();
};
