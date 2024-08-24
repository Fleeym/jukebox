#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/utils/web.hpp>
#include <sstream>

#include "nong_dropdown_layer.hpp"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "ccTypes.h"
#include "list/nong_list.hpp"
#include "../managers/index_manager.hpp"

namespace jukebox {

bool NongDropdownLayer::setup(std::vector<int> ids, CustomSongWidget* parent, int defaultSongID) {
    m_songIDS = ids;
    m_parentWidget = parent;
    m_defaultSongID = defaultSongID;
    // for (auto const& id : m_songIDS) {
    //     auto result = NongManager::get()->getNongs(id);
    //     auto value = result.value();
    //     m_data[id] = value;
    // }
    bool isMultiple = ids.size() > 1;
    if (ids.size() == 1) {
        m_currentSongID = ids[0];
    }

    auto contentSize = m_mainLayer->getContentSize();

    int manifest = NongManager::get()->getCurrentManifestVersion();
    int count = NongManager::get()->getStoredIDCount();
    std::stringstream ss;
    ss << "Manifest v" << manifest << ", storing " << count << " unique song IDs.";

    auto manifestLabel = CCLabelBMFont::create(ss.str().c_str(), "chatFont.fnt");
    manifestLabel->setPosition(contentSize.width / 2, 12.f);
    manifestLabel->limitLabelWidth(140.f, 0.9f, 0.1f);
    manifestLabel->setColor(ccColor3B { 220, 220, 220 });
    manifestLabel->setID("manifest-label");
    m_mainLayer->addChild(manifestLabel);

    auto menu = CCMenu::create();
    menu->setID("bottom-right-menu");
    auto spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    spr->setScale(0.7f);
    auto addBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongDropdownLayer::openAddPopup)
    );
    m_addBtn = addBtn;
    spr = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    auto discordBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongDropdownLayer::onDiscord)
    );
    discordBtn->setID("discord-button");
    spr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
    spr->setScale(0.7f);
    auto removeBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongDropdownLayer::deleteAllNongs)
    );
    m_deleteBtn = removeBtn;
    if (isMultiple) {
        m_addBtn->setVisible(false);
        m_deleteBtn->setVisible(false);
    } else {
        m_addBtn->setVisible(true);
        m_deleteBtn->setVisible(true);
    }
    menu->addChild(addBtn);
    menu->addChild(discordBtn);
    menu->addChild(removeBtn);
    auto layout = ColumnLayout::create();
    layout->setAxisAlignment(AxisAlignment::Start);
    menu->setContentSize({addBtn->getScaledContentSize().width, 200.f});
    menu->setAnchorPoint(ccp(1.0f, 0.0f));
    menu->setLayout(layout);
    menu->setPosition(contentSize.width - 7.f, 7.f);
    menu->setZOrder(2);
    m_mainLayer->addChild(menu);

    menu = CCMenu::create();
    menu->setID("settings-menu");
    auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    sprite->setScale(0.8f);
    auto settingsButton = CCMenuItemSpriteExtra::create(
        sprite,
        this,
        menu_selector(NongDropdownLayer::onSettings)
    );
    settingsButton->setID("settings-button");
    menu->addChild(settingsButton);
    menu->setAnchorPoint({0.5f, 1.0f});
    menu->setContentSize(settingsButton->getScaledContentSize());
    auto settingsLayout = ColumnLayout::create();
    settingsLayout->setAxisAlignment(AxisAlignment::End);
    settingsLayout->setAxisReverse(true);
    menu->setLayout(settingsLayout);
    menu->setZOrder(1);
    m_mainLayer->addChildAtPosition(menu, Anchor::TopRight, CCPoint { -25.0f, -5.0f  });

    auto bottomLeftArt = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    bottomLeftArt->setAnchorPoint({ 0.0f, 0.0f });
    m_mainLayer->addChildAtPosition(bottomLeftArt, Anchor::BottomLeft);

    auto bottomRightArt = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    bottomRightArt->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightArt->setFlipX(true);
    m_mainLayer->addChildAtPosition(bottomRightArt, Anchor::BottomRight);

    auto topLeftArt = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    topLeftArt->setAnchorPoint({ 0.0f, 1.0f });
    topLeftArt->setFlipY(true);
    m_mainLayer->addChildAtPosition(topLeftArt, Anchor::TopLeft);

    auto topRightArt = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    topRightArt->setAnchorPoint({ 1.0f, 1.0f });
    topRightArt->setFlipY(true);
    topRightArt->setFlipX(true);
    m_mainLayer->addChildAtPosition(topRightArt, Anchor::TopRight);

    this->createList();
    auto title = CCSprite::create("JB_ListLogo.png"_spr);
    title->setPosition(ccp(contentSize.width / 2, contentSize.height - 10.f));
    title->setScale(0.75f);
    m_mainLayer->addChild(title);
    handleTouchPriority(this);

    m_songErrorListener.bind([](SongErrorEvent* event){
        if (event->notifyUser()) {
            FLAlertLayer::create(
                "Error",
                event->error(),
                "OK"
            )->show();
        }
        return ListenerResult::Propagate;
    });

    m_songStateListener.bind([this](SongStateChangedEvent* event){
        if (!m_list || m_currentSongID != event->gdSongID()) return ListenerResult::Propagate;

        log::info("song state changed");
        createList();

        return ListenerResult::Propagate;
    });

    m_downloadListener.bind([this](SongDownloadProgressEvent* event){
        if (!m_list || m_currentSongID != event->gdSongID()) return ListenerResult::Propagate;

        m_list->setDownloadProgress(event->uniqueID(), event->progress());

        return ListenerResult::Propagate;
    });

    return true;
}

void NongDropdownLayer::onSettings(CCObject* sender) {
    geode::openSettingsPopup(Mod::get());
}

void NongDropdownLayer::onSelectSong(int songID) {
    m_currentSongID = songID;
}

void NongDropdownLayer::openAddPopup(CCObject* target) {
    if (!m_currentSongID.has_value()) return;
    NongAddPopup::create(this, m_currentSongID.value())->show();
}

void NongDropdownLayer::createList() {
    if (!m_list) {
        m_list = NongList::create(
            m_songIDS,
            CCSize { this->getCellSize().width, 220.f },
            [this](int gdSongID, const std::string& uniqueID) {
                this->setActiveSong(gdSongID, uniqueID);
            },
            [this](int gdSongID) {
                // TODO
                // if (!NongManager::get()->isFixingDefault(id)) {
                //     this->template addEventListener<GetSongInfoEventFilter>(
                //         [this](auto song) {
                //             this->refreshList();
                //             FLAlertLayer::create(
                //                 "Success",
                //                 "Default song data was refetched successfully!",
                //                 "Ok"
                //             )->show();
                //             return ListenerResult::Propagate;
                //         }, id
                //     );
                //     NongManager::get()->markAsInvalidDefault(id);
                //     NongManager::get()->prepareCorrectDefault(id);
                // }
                MusicDownloadManager::sharedState()->clearSong(gdSongID);
                MusicDownloadManager::sharedState()->getSongInfo(gdSongID, true);
            },
            [this](int gdSongID, const std::string& uniqueID, bool onlyAudio, bool confirm) {
                this->deleteSong(gdSongID, uniqueID, onlyAudio, confirm);
            },
            [this](int gdSongID, const std::string& uniqueID) {
                this->downloadSong(gdSongID, uniqueID);
            },
            [this](int gdSongID, const std::string& uniqueID) {
                auto nongs = NongManager::get()->getNongs(gdSongID);
                if (!nongs.has_value()) {
                    FLAlertLayer::create(
                        "Error",
                        "Song is not initialized",
                        "Ok"
                    )->show();
                    return;
                }
                auto nong = nongs.value()->getNongFromID(uniqueID);
                NongAddPopup::create(this, gdSongID, std::move(nong))->show();
            },
            [this](std::optional<int> currentSongID) {
                m_currentSongID = currentSongID;
                bool multiple = !currentSongID.has_value();
                if (multiple) {
                    m_addBtn->setVisible(false);
                    m_deleteBtn->setVisible(false);
                } else {
                    m_addBtn->setVisible(true);
                    m_deleteBtn->setVisible(true);
                }
            }
        );
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

CCSize NongDropdownLayer::getCellSize() const {
    return {
        320.f,
        60.f
    };
}

void NongDropdownLayer::setActiveSong(int gdSongID, const std::string& uniqueID) {
    if (auto err = NongManager::get()->setActiveSong(gdSongID, uniqueID); err.isErr()) {
        FLAlertLayer::create("Failed", fmt::format("Failed to set song: {}", err.error()), "Ok")->show();
        return;
    }
}

void NongDropdownLayer::onDiscord(CCObject* target) {
    geode::createQuickPopup(
        "Discord",
        "Do you want to <cb>join the Jukebox discord server</c> to receive updates and submit bug reports?",
        "No",
        "Yes",
        [](FLAlertLayer* alert, bool btn2) {
            if (btn2) {
                geode::utils::web::openLinkInBrowser("https://discord.gg/SFE7qxYFyU");
            }
        }
    );
}

void NongDropdownLayer::deleteSong(int gdSongID, const std::string& uniqueID, bool onlyAudio, bool confirm) {
    auto func = [gdSongID, uniqueID, onlyAudio, confirm](){
        if (onlyAudio) {
            if (auto err = NongManager::get()->deleteSongAudio(gdSongID, uniqueID); err.isErr()) {
                FLAlertLayer::create("Failed", fmt::format("Failed to delete song: {}", err.error()), "Ok")->show();
                return;
            }
        } else {
            if (auto err = NongManager::get()->deleteSong(gdSongID, uniqueID); err.isErr()) {
                FLAlertLayer::create("Failed", fmt::format("Failed to delete song: {}", err.error()), "Ok")->show();
                return;
            }
        }

        if (confirm) {
          FLAlertLayer::create("Success", "The song was deleted!", "Ok")->show();
        }
    };

    if (!confirm) {
        func();
        return;
    }

    createQuickPopup(
        "Are you sure?",
        fmt::format("Are you sure you want to delete the song from your NONGs?"),
        "No",
        "Yes",
        [this, func] (FLAlertLayer* self, bool btn2) {
            if (!btn2) return;
            func();
        }
    );
}

void NongDropdownLayer::downloadSong(int gdSongID, const std::string& uniqueID) {
    if (auto err = IndexManager::get()->downloadSong(gdSongID, uniqueID); err.isErr()) {
        FLAlertLayer::create("Failed", fmt::format("Failed to start/stop downloading song: {}", err.error()), "Ok")->show();
        return;
    }
}

void NongDropdownLayer::addSong(Nongs&& song, bool popup) {
    if (!m_currentSongID) {
        return;
    }
    int id = m_currentSongID.value();
    if (auto err = NongManager::get()->addNongs(std::move(song)); err.isErr()) {
        FLAlertLayer::create("Failed", fmt::format("Failed to add song: {}", err.error()), "Ok")->show();
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
    createQuickPopup("Delete all nongs",
        "Are you sure you want to <cr>delete all nongs</c> for this song?",
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (!btn2) {
                return;
            }

            if (!m_currentSongID) {
                return;
            }

            int id = m_currentSongID.value();
            if (auto err = NongManager::get()->deleteAllSongs(id); err.isErr()) {
                FLAlertLayer::create("Failed", fmt::format("Failed to delete nongs: {}", err.error()), "Ok")->show();
                return;
            }
            auto data = NongManager::get()->getNongs(id).value();
            FLAlertLayer::create("Success", "All nongs were deleted successfully!", "Ok")->show();
        }
    );
}

}
