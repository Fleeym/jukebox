#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/utils/web.hpp>
#include <filesystem>
#include <sstream>

#include "nong_dropdown_layer.hpp"
#include "../managers/nong_manager.hpp"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "ccTypes.h"
#include "list/nong_list.hpp"

namespace fs = std::filesystem;

namespace jukebox {

bool NongDropdownLayer::setup(std::vector<int> ids, CustomSongWidget* parent, int defaultSongID) {
    m_songIDS = ids;
    m_parentWidget = parent;
    m_defaultSongID = defaultSongID;
    for (auto const& id : m_songIDS) {
        auto result = NongManager::get()->getNongs(id);
        if (!result.has_value()) {
            NongManager::get()->createDefault(id);
            NongManager::get()->writeJson();
            result = NongManager::get()->getNongs(id);
        }
        auto value = result.value();
        m_data[id] = value;
    }
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

    auto bigMenu = CCMenu::create();
    bigMenu->setID("big-menu");
    bigMenu->ignoreAnchorPointForPosition(false);
    auto discordSpr = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    auto discordBtn = CCMenuItemSpriteExtra::create(
        discordSpr,
        this,
        menu_selector(NongDropdownLayer::onDiscord)
    );
    discordBtn->setID("discord-button");
    CCPoint position = discordBtn->getScaledContentSize() / 2;
    position += { 5.f, 5.f };
    bigMenu->addChildAtPosition(discordBtn, Anchor::BottomLeft, position);
    this->addChild(bigMenu);

    auto spr = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
    spr->setScale(0.7f);
    auto menu = CCMenu::create();
    menu->setID("bottom-right-menu");
    auto downloadBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongDropdownLayer::fetchSongFileHub)
    );
    m_downloadBtn = downloadBtn;
    spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    spr->setScale(0.7f);
    auto addBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongDropdownLayer::openAddPopup)
    );
    m_addBtn = addBtn;
    spr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
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
        m_downloadBtn->setVisible(false);
    } else {
        m_addBtn->setVisible(true);
        m_deleteBtn->setVisible(true);
        m_downloadBtn->setVisible(true);
    }
    menu->addChild(addBtn);
    menu->addChild(downloadBtn);
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
    return true;
}

void NongDropdownLayer::onSettings(CCObject* sender) {
    geode::openSettingsPopup(Mod::get());
}

void NongDropdownLayer::onSelectSong(int songID) {
    m_currentSongID = songID;
    this->createList();
}

void NongDropdownLayer::openAddPopup(CCObject* target) {
    NongAddPopup::create(this)->show();
}

void NongDropdownLayer::createList() {
    if (!m_list) {
        m_list = NongList::create(
            m_data, 
            CCSize { this->getCellSize().width, 220.f },
            [this](int id, const SongInfo& song) {
                m_currentSongID = id;
                this->setActiveSong(song);
            },
            [this](int id) {
                if (!NongManager::get()->isFixingDefault(id)) {
                    this->template addEventListener<GetSongInfoEventFilter>(
                        [this](auto song) {
                            this->refreshList();
                            FLAlertLayer::create(
                                "Success",
                                "Default song data was refetched successfully!",
                                "Ok"
                            )->show();
                            return ListenerResult::Propagate;
                        }, id
                    );
                    NongManager::get()->markAsInvalidDefault(id);
                    NongManager::get()->prepareCorrectDefault(id);
                }
            },
            [this](int id, const SongInfo& song) {
                m_currentSongID = id;
                this->deleteSong(song);
            },
            [this](bool multiple) {
                if (multiple) {
                    m_addBtn->setVisible(false);
                    m_deleteBtn->setVisible(false);
                    m_downloadBtn->setVisible(false);
                } else {
                    m_addBtn->setVisible(true);
                    m_deleteBtn->setVisible(true);
                    m_downloadBtn->setVisible(true);
                }
            }
        );
        m_mainLayer->addChildAtPosition(m_list, Anchor::Center);
        return;
    }

    m_list->setData(m_data);
    // TODO FIX
    if (m_currentSongID.has_value()) {
        m_list->setCurrentSong(m_currentSongID.value());
    }
    m_list->build();
    handleTouchPriority(this);
}

SongInfo NongDropdownLayer::getActiveSong() {
    int id = m_currentSongID.value();
    auto active = NongManager::get()->getActiveNong(id);
    if (!active.has_value()) {
        m_data[id].active = m_data[id].defaultPath;
        NongManager::get()->saveNongs(m_data[id], id);
        return NongManager::get()->getActiveNong(id).value();
    }
    return active.value();
}

CCSize NongDropdownLayer::getCellSize() const {
    return {
        320.f,
        60.f
    };
}

void NongDropdownLayer::setActiveSong(SongInfo const& song) {
    if (!m_currentSongID) {
        return;
    }
    int id = m_currentSongID.value();
    auto songs = m_data[id];
    if (!fs::exists(song.path)) {
        FLAlertLayer::create("Failed", "Failed to set song: file not found", "Ok")->show();
        return;
    }

    m_data[id].active = song.path;

    NongManager::get()->saveNongs(m_data[id], id);
    
    this->updateParentWidget(song);

    this->createList();
}

void NongDropdownLayer::refreshList() {
    if (m_currentSongID.has_value()) {
        int id = m_currentSongID.value();
        m_data[id] = NongManager::get()->getNongs(id).value();
    }
    this->createList();
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

void NongDropdownLayer::updateParentWidget(SongInfo const& song) {
    m_parentWidget->m_songInfoObject->m_artistName = song.authorName;
    m_parentWidget->m_songInfoObject->m_songName = song.songName;
    if (song.songUrl != "local") {
        m_parentWidget->m_songInfoObject->m_songUrl = song.songUrl;
    }
    m_parentWidget->updateSongObject(this->m_parentWidget->m_songInfoObject);
}

void NongDropdownLayer::deleteSong(SongInfo const& song) {
    if (!m_currentSongID) {
        return;
    }
    int id = m_currentSongID.value();
    NongManager::get()->deleteNong(song, id);
    auto active = NongManager::get()->getActiveNong(id).value();
    this->updateParentWidget(active);
    FLAlertLayer::create("Success", "The song was deleted!", "Ok")->show();
    m_data[id] = NongManager::get()->getNongs(id).value();
    this->createList();
}

void NongDropdownLayer::addSong(SongInfo const& song) {
    if (!m_currentSongID) {
        return;
    }
    int id = m_currentSongID.value();
    auto data = m_data[id];
    for (auto savedSong : data.songs) {
        if (song.path.string() == savedSong.path.string()) {
            FLAlertLayer::create("Error", "This NONG already exists! (<cy>" + savedSong.songName + "</c>)", "Ok")->show();
            return;
        }
    }
    NongManager::get()->addNong(song, id);
    FLAlertLayer::create("Success", "The song was added!", "Ok")->show();
    m_data[id] = NongManager::get()->getNongs(id).value();
    this->createList();
}

void NongDropdownLayer::fetchSongFileHub(CCObject*) {
    // TODO FIX
    if (!m_currentSongID) {
        return;
    }
    int id = m_currentSongID.value();
    std::stringstream ss;
    ss << "Do you want to open <cb>Song File Hub</c>? The song ID (" << id << ") will be copied to your <cr>clipboard</c>.";
    createQuickPopup(
        "Song File Hub",
        ss.str(),
        "No",
        "Yes",
        [this, id](FLAlertLayer* alert, bool btn2) {
            if (!btn2) {
                return;
            }
            std::stringstream ss;
            ss << id;
            geode::utils::clipboard::write(ss.str());
            geode::utils::web::openLinkInBrowser("https://songfilehub.com");
        }
    );
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
            NongManager::get()->deleteAll(id);
            auto data = NongManager::get()->getNongs(id).value();
            auto active = NongManager::get()->getActiveNong(id).value();
            m_data[id] = data;
            this->updateParentWidget(active);
            this->createList();
            FLAlertLayer::create("Success", "All nongs were deleted successfully!", "Ok")->show();
        }
    );
}

}
