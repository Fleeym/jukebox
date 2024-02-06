#include <Geode/loader/Loader.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/Modify.hpp>
#include <fmt/core.h>
#include <sstream>

using namespace geode::prelude;

struct JBMenuLayer : Modify<JBMenuLayer, MenuLayer> {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        if (Mod::get()->getSavedValue<bool>("failed-load", false)) {
            Mod::get()->setSavedValue("failed-load", false);
            std::stringstream ss;
            std::string content = fmt::format("Jukebox <cr>couldn't load nong_data.json</c>. This means that none of your saved songs have been loaded. Please check <cb>{}</c> for any created backups and report this to me on my Discord (fleeym)", Mod::get()->getSaveDir() / "backups");
            auto popup = FLAlertLayer::create("Jukebox", content.c_str(), "Ok");
            popup->m_scene = this;
            popup->show();
        }

        return true;
    }
};