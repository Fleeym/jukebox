#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/utils/web.hpp>
#include <filesystem>
#include <sstream>

#include "nong_dropdown_layer.hpp"
#include "../managers/nong_manager.hpp"

namespace fs = std::filesystem;

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
    if (ids.size() == 1) {
        m_currentListType = NongListType::Single;
        m_currentSongID = ids[0];
    } else {
        m_currentListType = NongListType::Multiple;
    }
    auto winsize = CCDirector::sharedDirector()->getWinSize();

    int manifest = NongManager::get()->getCurrentManifestVersion();
    int count = NongManager::get()->getStoredIDCount();
    std::stringstream ss;
    ss << "Manifest v" << manifest << ", storing " << count << " unique song IDs.";

    auto manifestLabel = CCLabelBMFont::create(ss.str().c_str(), "chatFont.fnt");
    manifestLabel->setPosition(winsize.width / 2, winsize.height / 2 - 125.f);
    manifestLabel->limitLabelWidth(140.f, 0.9f, 0.1f);
    manifestLabel->setColor(cc3x(0xc2c2c2));
    manifestLabel->setID("manifest-label");
    m_mainLayer->addChild(manifestLabel);

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
    downloadBtn->setPositionY(35.f);
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
    removeBtn->setPositionY(67.f);
    m_deleteBtn = removeBtn;
    spr = CCSprite::createWithSpriteFrameName("backArrowPlain_01_001.png");
    auto backBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongDropdownLayer::onBack)
    );
    m_backBtn = backBtn;
    backBtn->setPosition(ccp(-370.f, 100.f));

    if (m_currentListType == NongListType::Multiple) {
        m_addBtn->setVisible(false);
        m_deleteBtn->setVisible(false);
        m_downloadBtn->setVisible(false);
        m_backBtn->setVisible(false);
    }
    if (m_data.size() == 1) {
        m_backBtn->setVisible(false);
    }
    menu->addChild(backBtn);
    menu->addChild(addBtn);
    menu->addChild(downloadBtn);
    menu->addChild(removeBtn);
    menu->setPosition(winsize.width / 2 + 185.f, winsize.height / 2 - 105.f);
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
    menu->setPosition(winsize.width - 30.f, winsize.height - 31.f);
    m_mainLayer->addChild(menu);

    this->createList();
    auto listpos = m_listView->getPosition();
    auto leftspr = CCSprite::createWithSpriteFrameName("GJ_commentSide2_001.png");
    leftspr->setPosition(ccp(listpos.x - 162.f, listpos.y));
    leftspr->setScaleY(6.8f);
    leftspr->setZOrder(1);
    m_mainLayer->addChild(leftspr);
    auto rightspr = CCSprite::createWithSpriteFrameName("GJ_commentSide2_001.png");
    rightspr->setPosition(ccp(listpos.x + 162.f, listpos.y));
    rightspr->setScaleY(6.8f);
    rightspr->setFlipX(true);
    rightspr->setZOrder(1);
    m_mainLayer->addChild(rightspr);
    auto bottomspr = CCSprite::createWithSpriteFrameName("GJ_commentTop2_001.png");
    bottomspr->setPosition(ccp(listpos.x, listpos.y - 95.f));
    bottomspr->setFlipY(true);
    bottomspr->setScaleX(0.934f);
    bottomspr->setZOrder(1);
    m_mainLayer->addChild(bottomspr);
    auto topspr = CCSprite::createWithSpriteFrameName("GJ_commentTop2_001.png");
    topspr->setPosition(ccp(listpos.x, listpos.y + 95.f));
    topspr->setScaleX(0.934f);
    topspr->setZOrder(1);
    m_mainLayer->addChild(topspr);
    auto title = CCSprite::create("JB_ListLogo.png"_spr);
    title->setPosition(ccp(winsize.width / 2, winsize.height / 2 + 125.f));
    title->setScale(0.75f);
    m_mainLayer->addChild(title);
    handleTouchPriority(this);
    return true;
}

void NongDropdownLayer::onSettings(CCObject* sender) {
    geode::openSettingsPopup(Mod::get());
}

void NongDropdownLayer::onSelectSong(int songID) {
    if (m_currentListType == NongListType::Single) {
        return;
    }

    m_currentSongID = songID;
    m_currentListType = NongListType::Single;
    this->createList();
    m_addBtn->setVisible(true);
    m_deleteBtn->setVisible(true);
    m_downloadBtn->setVisible(true);
    if (m_data.size() > 1) {
        m_backBtn->setVisible(true);
    }
}

void NongDropdownLayer::onBack(CCObject*) {
    if (m_currentListType == NongListType::Multiple || m_data.size() == 1) {
        return;
    }

    m_currentSongID = -1;
    m_currentListType = NongListType::Multiple;
    this->createList();
    m_addBtn->setVisible(false);
    m_deleteBtn->setVisible(false);
    m_downloadBtn->setVisible(false);
    m_backBtn->setVisible(false);
}

void NongDropdownLayer::openAddPopup(CCObject* target) {
    NongAddPopup::create(this)->show();
}

void NongDropdownLayer::createList() {
    switch (m_currentListType) {
        case NongListType::Single: {
            auto songs = CCArray::create();
            auto activeSong = this->getActiveSong();
            NongData songData = m_data[m_currentSongID];

            songs->addObject(NongCell::create(activeSong, this, this->getCellSize(), true, activeSong.path == songData.defaultPath));

            for (auto song : songData.songs) {
                if (songData.active == song.path) {
                    continue;
                }
                songs->addObject(NongCell::create(song, this, this->getCellSize(), false, song.path == songData.defaultPath));
            }
            if (m_listView) {
                m_listView->removeFromParent();
            }

            auto list = ListView::create(songs, this->getCellSize().height, this->getCellSize().width, 200.f);
            m_mainLayer->addChild(list);
            auto winsize = CCDirector::sharedDirector()->getWinSize();
            list->setPosition(winsize.width / 2, winsize.height / 2 - 15.f);
            list->ignoreAnchorPointForPosition(false);
            m_listView = list;
            break;
        }
        case NongListType::Multiple: {
            auto cells = CCArray::create();
            for (auto const& kv : m_data) {
                cells->addObject(JBSongCell::create(kv.second, kv.first, this, this->getCellSize()));
            }
            if (m_listView) {
                m_listView->removeFromParent();
            }

            ListView* list = ListView::create(cells, this->getCellSize().height, this->getCellSize().width, 200.f);
            m_mainLayer->addChild(list);
            auto winsize = CCDirector::sharedDirector()->getWinSize();
            list->setPosition(winsize.width / 2, winsize.height / 2 - 15.f);
            list->ignoreAnchorPointForPosition(false);
            m_listView = list;
            break;
        }
    }
    handleTouchPriority(this);
}

SongInfo NongDropdownLayer::getActiveSong() {
    auto active = NongManager::get()->getActiveNong(m_currentSongID);
    if (!active.has_value()) {
        m_data[m_currentSongID].active = m_data[m_currentSongID].defaultPath;
        NongManager::get()->saveNongs(m_data[m_currentSongID], m_currentSongID);
        return NongManager::get()->getActiveNong(m_currentSongID).value();
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
    auto songs = m_data[m_currentSongID];
    if (!fs::exists(song.path)) {
        FLAlertLayer::create("Failed", "Failed to set song: file not found", "Ok")->show();
        return;
    }

    m_data[m_currentSongID].active = song.path;

    NongManager::get()->saveNongs(m_data[m_currentSongID], m_currentSongID);
    
    this->updateParentWidget(song);

    this->createList();
}

void NongDropdownLayer::refreshList() {
    m_data[m_currentSongID] = NongManager::get()->getNongs(m_currentSongID).value();
    this->createList();
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
    NongManager::get()->deleteNong(song, m_currentSongID);
    auto active = NongManager::get()->getActiveNong(m_currentSongID).value();
    this->updateParentWidget(active);
    FLAlertLayer::create("Success", "The song was deleted!", "Ok")->show();
    m_data[m_currentSongID] = NongManager::get()->getNongs(m_currentSongID).value();
    this->createList();
}

void NongDropdownLayer::addSong(SongInfo const& song) {
    auto data = m_data[m_currentSongID];
    for (auto savedSong : data.songs) {
        if (song.path.string() == savedSong.path.string()) {
            FLAlertLayer::create("Error", "This NONG already exists! (<cy>" + savedSong.songName + "</c>)", "Ok")->show();
            return;
        }
    }
    NongManager::get()->addNong(song, m_currentSongID);
    FLAlertLayer::create("Success", "The song was added!", "Ok")->show();
    m_data[m_currentSongID] = NongManager::get()->getNongs(m_currentSongID).value();
    this->createList();
}

void NongDropdownLayer::fetchSongFileHub(CCObject*) {
    if (m_currentListType == NongListType::Multiple) {
        return;
    }
    std::stringstream ss;
    ss << "Do you want to open <cb>Song File Hub</c>? The song ID (" << m_currentSongID << ") will be copied to your <cr>clipboard</c>.";
    createQuickPopup(
        "Song File Hub",
        ss.str(),
        "No",
        "Yes",
        [this](FLAlertLayer* alert, bool btn2) {
            if (!btn2) {
                return;
            }
            std::stringstream ss;
            ss << m_currentSongID;
            geode::utils::clipboard::write(ss.str());
            geode::utils::web::openLinkInBrowser("https://songfilehub.com");
        }
    );
}

void NongDropdownLayer::deleteAllNongs(CCObject*) {
    createQuickPopup("Delete all nongs", 
        "Are you sure you want to <cr>delete all nongs</c> for this song?", 
        "No", 
        "Yes",
        [this](auto, bool btn2) {
            if (!btn2) {
                return;
            }

            NongManager::get()->deleteAll(m_currentSongID);
            auto data = NongManager::get()->getNongs(m_currentSongID).value();
            m_data[m_currentSongID] = data;
            this->updateParentWidget(this->getActiveSong());
            this->createList();
            FLAlertLayer::create("Success", "All nongs were deleted successfully!", "Ok")->show();
        }
    );
}

int NongDropdownLayer::getSongID() {
    return m_currentSongID;
}