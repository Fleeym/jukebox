#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelPage.hpp>
#include <Geode/binding/LevelTools.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/binding/BoomScrollLayer.hpp>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/modify/LevelSelectLayer.hpp>

#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/ui/nong_dropdown_layer.hpp>

using namespace geode::prelude;
using namespace jukebox;

class $modify(JBLevelSelectLayer, LevelSelectLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* m_jbBtn = nullptr;
    };

    void onJukebox(CCObject*) {
        if (!m_scrollLayer) {
            return;
        }

        GJGameLevel* level = nullptr;
        if (auto pageLayer = m_scrollLayer->getPage(m_scrollLayer->m_page)) {
            if (auto page = typeinfo_cast<LevelPage*>(pageLayer)) {
                level = page->m_level;
            }
        }
        if (!level && m_scrollLayer->m_page > 0) {
            if (auto pageLayer = m_scrollLayer->getPage(m_scrollLayer->m_page - 1)) {
                if (auto page = typeinfo_cast<LevelPage*>(pageLayer)) {
                    level = page->m_level;
                }
            }
        }
        if (!level) {
            return;
        }

        const bool isRobtopSong = level->m_songID == 0;
        const int rawSongID = isRobtopSong ? level->m_audioTrack : level->m_songID;
        const int adjustedID = NongManager::get().adjustSongID(rawSongID, isRobtopSong);
        if (adjustedID == 0) {
            return;
        }

        if (!NongManager::get().hasSongID(adjustedID)) {
            auto title = LevelTools::getAudioTitle(rawSongID);
            int artistID = LevelTools::artistForAudio(rawSongID);
            std::string artist = LevelTools::nameForArtist(artistID);
            
            if (rawSongID == 21) {
                artist = "MDK";
            }

            auto obj = SongInfoObject::create(rawSongID, title, artist, artistID, 0.0f,
                                              "", "", "", 0, "", false, 0, 0);
            if (!obj) {
                return;
            }

            auto res = NongManager::get().initSongID(obj, rawSongID, isRobtopSong);
            if (res.isErr()) {
                FLAlertLayer::create("Error", res.unwrapErr(), "Ok")->show();
                return;
            }
        }

        auto layer = NongDropdownLayer::create({adjustedID}, nullptr, adjustedID, std::nullopt);
        if (!layer) {
            FLAlertLayer::create("Error", "Failed to open Jukebox popup", "Ok")
                ->show();
            return;
        }
        layer->setZOrder(106);
        layer->show();
    }

    void ensureJukeboxButton() {
        if (m_fields->m_jbBtn && m_fields->m_jbBtn->getParent()) {
            return;
        }
        m_fields->m_jbBtn = nullptr;

        auto infoMenu = this->getChildByIDRecursive("info-menu");
        if (!infoMenu) {
            return;
        }

        auto menu = typeinfo_cast<CCMenu*>(infoMenu);
        if (!menu) {
            return;
        }

        auto infoBtn = menu->getChildByID("info-button");
        if (!infoBtn) {
            return;
        }

        auto spr = CCSprite::createWithSpriteFrameName("JB_PinDisc.png"_spr);
        if (!spr) {
            return;
        }
        spr->setScale(0.65f);
        spr->setID("nong-pin"_spr);

        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(JBLevelSelectLayer::onJukebox));
        btn->setID("nong-button"_spr);

        constexpr float gap = 6.f;
        auto pos = infoBtn->getPosition();
        float infoBtnWidth = infoBtn->getScaledContentSize().width;
        float x = pos.x - infoBtnWidth / 2.f - btn->getScaledContentSize().width / 2.f - gap;
        btn->setPosition({x, pos.y});

        menu->addChild(btn, infoBtn->getZOrder());
        m_fields->m_jbBtn = btn;
    }

    bool init(int page) {
        if (!LevelSelectLayer::init(page)) {
            return false;
        }
        this->ensureJukeboxButton();
        return true;
    }

    void scrollLayerMoved(cocos2d::CCPoint position) {
        LevelSelectLayer::scrollLayerMoved(position);
        this->ensureJukeboxButton();
    }
};

