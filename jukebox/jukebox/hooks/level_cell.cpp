#include <optional>

#include <Geode/modify/LevelCell.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelCell.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/loader/Loader.hpp>

#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/nong.hpp>

using namespace geode::prelude;
using namespace jukebox;

class $modify(JBLevelCell, LevelCell) {
    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        if (!Loader::get()->isModLoaded("geode.node-ids")) {
            return;
        }

        CCNode* main = this->getChildByID("main-layer");
        if (!main) {
            return;
        }

        CCNode* label = main->getChildByID("song-name");
        if (!label) {
            return;
        }

        CCLabelBMFont* songName = static_cast<CCLabelBMFont*>(label);
        std::optional<std::string> nongName = this->getNongSongName();
        if (nongName.has_value()) {
            songName->setString(nongName.value().c_str());
        }
    }

    std::optional<std::string> getNongSongName() {
        int id = m_level->m_songID;
        if (m_level->m_songID == 0) {
            id = (-m_level->m_audioTrack) - 1;
        }

        std::optional<Nongs*> opt = NongManager::get().getNongs(id);
        if (!opt.has_value()) {
            return std::nullopt;
        }

        Nongs* nongs = opt.value();
        return nongs->active()->metadata()->name;
    }
};
