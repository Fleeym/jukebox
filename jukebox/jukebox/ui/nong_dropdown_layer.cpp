#include <jukebox/ui/nong_dropdown_layer.hpp>

#include <optional>
#include <string>
#include <vector>

#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <ccTypes.h>
#include <fmt/core.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/SimpleAxisLayout.hpp>
#include <Geode/utils/web.hpp>

#include <jukebox/events/get_song_info.hpp>
#include <jukebox/managers/index_manager.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/ui/list/nong_list.hpp>
#include <jukebox/ui/nong_add_popup.hpp>

using namespace geode::prelude;

namespace jukebox {

bool NongDropdownLayer::setup(std::vector<int> ids, CustomSongWidget* parent,
                              int defaultSongID, std::optional<int> levelID) {
    m_songIDS = ids;
    m_parentWidget = parent;
    m_defaultSongID = defaultSongID;
    bool isMultiple = ids.size() > 1;
    if (ids.size() == 1) {
        m_currentSongID = ids[0];
    }
    m_levelID = levelID;

    CCSize contentSize = m_mainLayer->getContentSize();

    int manifest = NongManager::get().getCurrentManifestVersion();
    int count = NongManager::get().getStoredIDCount();
    std::string label = fmt::format("Manifest v{}, storing {} unique song IDs.",
                                    manifest, count);
    CCLabelBMFont* manifestLabel =
        CCLabelBMFont::create(label.c_str(), "chatFont.fnt");
    manifestLabel->setPosition(contentSize.width / 2, 12.f);
    manifestLabel->limitLabelWidth(140.f, 0.9f, 0.1f);
    manifestLabel->setColor(ccColor3B{220, 220, 220});
    manifestLabel->setID("manifest-label");
    m_mainLayer->addChild(manifestLabel);

    m_bottomRightMenu = CCMenu::create();
    m_bottomRightMenu->setID("bottom-right-menu");
    CCSprite* spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    spr->setScale(0.7f);
    m_addBtn = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(NongDropdownLayer::openAddPopup));

    spr = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    m_discordBtn = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(NongDropdownLayer::onDiscord));
    m_discordBtn->setID("discord-button");

    spr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
    m_deleteBtn = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(NongDropdownLayer::deleteAllNongs));

    if (isMultiple) {
        m_addBtn->setVisible(false);
        m_deleteBtn->setVisible(false);
    } else {
        m_addBtn->setVisible(true);
        m_deleteBtn->setVisible(true);
    }
    m_bottomRightMenu->addChild(m_addBtn);
    m_bottomRightMenu->addChild(m_discordBtn);
    m_bottomRightMenu->addChild(m_deleteBtn);
    SimpleAxisLayout* layout =
        SimpleColumnLayout::create()
            ->setMainAxisAlignment(MainAxisAlignment::Start)
            ->setMainAxisDirection(AxisDirection::BottomToTop)
            ->setMainAxisScaling(AxisScaling::ScaleDown)
            ->setCrossAxisScaling(AxisScaling::ScaleDown)
            ->setGap(5.0f);
    layout->ignoreInvisibleChildren(true);
    m_bottomRightMenu->setContentSize(
        {40.0f, m_mainLayer->getContentHeight() / 2 - 5.0f});
    m_bottomRightMenu->setAnchorPoint({1.0f, 0.0f});
    m_bottomRightMenu->setLayout(layout);
    m_bottomRightMenu->setZOrder(2);
    m_mainLayer->addChildAtPosition(m_bottomRightMenu, Anchor::BottomRight,
                                    CCPoint{-5.0f, 5.0f});

    CCMenu* settingsMenu = CCMenu::create();
    settingsMenu->setID("settings-menu");
    CCSprite* sprite =
        CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    sprite->setScale(0.8f);
    CCMenuItemSpriteExtra* settingsButton = CCMenuItemSpriteExtra::create(
        sprite, this, menu_selector(NongDropdownLayer::onSettings));
    settingsButton->setID("settings-button");
    settingsMenu->addChild(settingsButton);
    settingsMenu->setAnchorPoint({1.0f, 1.0f});
    settingsMenu->setLayout(SimpleColumnLayout::create()
                                ->setMainAxisAlignment(MainAxisAlignment::Start)
                                ->setMainAxisScaling(AxisScaling::Fit)
                                ->setCrossAxisScaling(AxisScaling::Fit));
    settingsMenu->setZOrder(1);
    m_mainLayer->addChildAtPosition(settingsMenu, Anchor::TopRight,
                                    CCPoint{-5.0f, -5.0f});

    CCSprite* bottomLeftArt =
        CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    bottomLeftArt->setAnchorPoint({0.0f, 0.0f});
    m_mainLayer->addChildAtPosition(bottomLeftArt, Anchor::BottomLeft);

    CCSprite* bottomRightArt =
        CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    bottomRightArt->setAnchorPoint({1.0f, 0.0f});
    bottomRightArt->setFlipX(true);
    m_mainLayer->addChildAtPosition(bottomRightArt, Anchor::BottomRight);

    CCSprite* topLeftArt =
        CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    topLeftArt->setAnchorPoint({0.0f, 1.0f});
    topLeftArt->setFlipY(true);
    m_mainLayer->addChildAtPosition(topLeftArt, Anchor::TopLeft);

    CCSprite* topRightArt =
        CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    topRightArt->setAnchorPoint({1.0f, 1.0f});
    topRightArt->setFlipY(true);
    topRightArt->setFlipX(true);
    m_mainLayer->addChildAtPosition(topRightArt, Anchor::TopRight);

    this->createList();
    CCSprite* title =
        CCSprite::createWithSpriteFrameName("JB_ListLogo.png"_spr);
    title->setPosition(ccp(contentSize.width / 2, contentSize.height - 10.f));
    title->setScale(0.75f);
    m_mainLayer->addChild(title);
    handleTouchPriority(this);

    m_songErrorListener.bind([this](event::SongError* event) {
        if (event->notifyUser()) {
            FLAlertLayer* popup =
                FLAlertLayer::create("Error", event->error(), "OK");
            popup->setZOrder(this->getZOrder() + 1);
            popup->show();
        }
        return ListenerResult::Propagate;
    });

    m_songInfoListener.bind([this](event::GetSongInfo* event) {
        if (!m_list || m_currentSongID != event->gdSongID()) {
            return ListenerResult::Propagate;
        }

        FLAlertLayer* popup = FLAlertLayer::create(
            "Song Refetched", "Successfully refetched default song data", "Ok");
        popup->setZOrder(this->getZOrder() + 1);
        popup->show();

        return ListenerResult::Propagate;
    });

    m_downloadFailedListener.bind([this](event::SongDownloadFailed* event) {
        if (!m_list || m_currentSongID != event->gdSongId()) {
            return ListenerResult::Propagate;
        }

        FLAlertLayer* popup = FLAlertLayer::create(
            "Download failed",
            fmt::format("Song download failed. Reason: {}", event->error()),
            "Ok");
        popup->setZOrder(this->getZOrder() + 1);
        popup->show();

        return ListenerResult::Propagate;
    });

    return true;
}

void NongDropdownLayer::onSettings(CCObject* sender) {
    geode::openSettingsPopup(Mod::get());
}

void NongDropdownLayer::onSelectSong(int songID) { m_currentSongID = songID; }

void NongDropdownLayer::openAddPopup(CCObject* target) {
    if (!m_currentSongID.has_value()) {
        return;
    }
    NongAddPopup::create(m_currentSongID.value())->show();
}

void NongDropdownLayer::createList() {
    if (!m_list) {
        m_list = NongList::create(
            m_songIDS, CCSize{this->getCellSize().width, 220.f}, m_levelID,
            [this](std::optional<int> currentSongID) {
                m_currentSongID = currentSongID;
                bool multiple = !currentSongID.has_value();
                if (multiple) {
                    m_addBtn->setVisible(false);
                    m_deleteBtn->setVisible(false);
                    m_bottomRightMenu->updateLayout();
                } else {
                    m_addBtn->setVisible(true);
                    m_deleteBtn->setVisible(true);
                    m_bottomRightMenu->updateLayout();
                }
            });
        m_mainLayer->addChildAtPosition(m_list, Anchor::Center);
        return;
    }

    // TODO FIX
    if (m_currentSongID.has_value()) {
        m_list->setCurrentSong(m_currentSongID.value());
    }
    m_list->build();
    handleTouchPriority(this);
}

CCSize NongDropdownLayer::getCellSize() const { return {320.f, 60.f}; }

void NongDropdownLayer::onDiscord(CCObject* target) {
    geode::createQuickPopup("Discord",
                            "Do you want to <cb>join the Song File Hub discord "
                            "server</c> to receive "
                            "updates and submit bug reports?",
                            "No", "Yes", [](FLAlertLayer* alert, bool btn2) {
                                if (btn2) {
                                    geode::utils::web::openLinkInBrowser(
                                        "https://discord.gg/maSgd4zpEF");
                                }
                            });
}

void NongDropdownLayer::addSong(Nongs&& song, bool popup) {
    if (!m_currentSongID) {
        return;
    }
    int id = m_currentSongID.value();
    if (auto err = NongManager::get().addNongs(std::move(song)); err.isErr()) {
        FLAlertLayer::create(
            "Failed", fmt::format("Failed to add song: {}", err.unwrapErr()),
            "Ok")
            ->show();
        return;
    }
    if (popup) {
        FLAlertLayer::create("Success", "The song was added!", "Ok")->show();
    }
}

void NongDropdownLayer::deleteAllNongs(CCObject*) {
    if (!m_currentSongID) {
        return;
    }
    createQuickPopup(
        "Delete all nongs",
        "Are you sure you want to <cr>delete all nongs</c> for this song?",
        "No", "Yes", [this](auto, bool btn2) {
            if (!btn2) {
                return;
            }

            if (!m_currentSongID) {
                return;
            }

            int id = m_currentSongID.value();
            if (auto err = NongManager::get().deleteAllSongs(id); err.isErr()) {
                FLAlertLayer::create(
                    "Failed",
                    fmt::format("Failed to delete nongs: {}", err.unwrapErr()),
                    "Ok")
                    ->show();
                return;
            }
            Nongs* data = NongManager::get().getNongs(id).value();
            FLAlertLayer::create("Success",
                                 "All nongs were deleted successfully!", "Ok")
                ->show();
        });
}

}  // namespace jukebox
