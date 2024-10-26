#include "ui/list/nong_cell.hpp"

#include <numeric>

#include <fmt/core.h>
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/binding/FLAlertLayer.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/base_nodes/Layout.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/Popup.hpp"

#include "managers/index_manager.hpp"
#include "nong.hpp"

namespace jukebox {

bool NongCell::init(int songID, Song* info, bool isDefault, bool selected,
                    CCSize const& size, std::function<void()> onSelect,
                    std::function<void()> onFixDefault,
                    std::function<void()> onDelete,
                    std::function<void()> onDownload,
                    std::function<void()> onEdit) {
    if (!CCNode::init()) {
        return false;
    }

    m_songID = songID;
    m_songInfo = info;
    m_isDefault = isDefault;
    m_isActive = selected;
    m_isDownloaded = m_songInfo->path().has_value() &&
                     std::filesystem::exists(m_songInfo->path().value());
    m_isDownloadable = m_songInfo->type() != NongType::LOCAL;
    m_onSelect = onSelect;
    m_onFixDefault = onFixDefault;
    m_onDelete = onDelete;
    m_onDownload = onDownload;
    m_onEdit = onEdit;

    this->setContentSize(size);
    this->setAnchorPoint({0.5f, 0.5f});

    constexpr float PADDING_X = 12.0f;
    constexpr float PADDING_Y = 6.0f;
    const CCSize maxSize = {size.width - 2 * PADDING_X,
                            size.height - 2 * PADDING_Y};
    const float songInfoWidth = maxSize.width * (2.0f / 3.0f);
    const float buttonsWidth = maxSize.width - songInfoWidth;

    CCScale9Sprite* bg = CCScale9Sprite::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(75);
    bg->setScale(0.3f);
    bg->setContentSize(size / bg->getScale());
    this->addChildAtPosition(bg, Anchor::Center);

    CCMenu* menu = CCMenu::create();

    if (m_isDownloaded) {
        const char* selectSprName =
            selected ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png";

        CCSprite* selectSpr =
            CCSprite::createWithSpriteFrameName(selectSprName);
        selectSpr->setScale(0.7f);

        CCMenuItemSpriteExtra* selectButton = CCMenuItemSpriteExtra::create(
            selectSpr, this, menu_selector(NongCell::onSet));
        selectButton->setAnchorPoint({0.5f, 0.5f});
        selectButton->setID("set-button");
        menu->addChild(selectButton);
    }

    menu->setID("buttons-menu");
    menu->setAnchorPoint({1.0f, 0.5f});
    menu->setContentSize({buttonsWidth, maxSize.height});
    menu->setLayout(
        RowLayout::create()->setGap(5.f)->setAxisAlignment(AxisAlignment::End));

    if (!m_isDefault && !m_songInfo->indexID().has_value()) {
        CCSprite* spr = CCSprite::create("JB_Edit.png"_spr);
        spr->setScale(0.7f);
        CCMenuItemSpriteExtra* editButton = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(NongCell::onEdit));
        editButton->setID("edit-button");
        editButton->setAnchorPoint({0.5f, 0.5f});
        menu->addChild(editButton);
    }

    if (isDefault) {
        CCSprite* sprite =
            CCSprite::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
        sprite->setScale(0.8f);
        if (!selected) {
            sprite->setColor({0x80, 0x80, 0x80});
        }
        CCMenuItemSpriteExtra* fixButton = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NongCell::onFixDefault));
        fixButton->setID("fix-button");
        menu->addChild(fixButton);
    } else if (m_isDownloadable && !m_isDownloaded) {
        CCSprite* sprite =
            CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        sprite->setScale(0.7f);
        m_downloadButton = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NongCell::onDownload));
        m_downloadButton->setID("download-button");
        menu->addChild(m_downloadButton);

        CCSprite* progressBarBack =
            CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
        progressBarBack->setColor({50, 50, 50});
        progressBarBack->setScale(0.62f);

        CCSprite* spr =
            CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
        spr->setColor({0, 255, 0});

        m_downloadProgress = CCProgressTimer::create(spr);
        m_downloadProgress->setType(
            CCProgressTimerType::kCCProgressTimerTypeRadial);
        m_downloadProgress->setPercentage(50.f);
        m_downloadProgress->setID("progress-bar");
        m_downloadProgress->setScale(0.66f);

        m_downloadProgressContainer = CCMenu::create();

        m_downloadProgressContainer->addChildAtPosition(m_downloadProgress,
                                                        Anchor::Center);
        m_downloadProgressContainer->addChildAtPosition(progressBarBack,
                                                        Anchor::Center);

        m_downloadProgressContainer->setZOrder(-1);
        m_downloadProgressContainer->setVisible(false);

        m_downloadButton->addChildAtPosition(m_downloadProgressContainer,
                                             Anchor::Center);
    }

    if (!m_isDefault) {
        CCSprite* sprite =
            CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        sprite->setScale(0.6475f);
        CCMenuItemSpriteExtra* deleteButton = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NongCell::onDelete));
        deleteButton->setID("delete-button");
        menu->addChild(deleteButton);
    }

    menu->updateLayout();
    menu->setID("button-menu");
    this->addChildAtPosition(menu, Anchor::Right, {-PADDING_X, 0.0f});

    SongMetadata* songMetadata = m_songInfo->metadata();

    std::vector<std::string> metadataList = {};

    switch (m_songInfo->type()) {
        case NongType::YOUTUBE:
            metadataList.push_back("youtube");
            break;
        case NongType::HOSTED:
            metadataList.push_back("hosted");
            break;
        default:
            break;
    }

    if (m_songInfo->indexID().has_value()) {
        auto indexID = m_songInfo->indexID().value();
        auto indexName = IndexManager::get().getIndexName(indexID);
        metadataList.push_back(indexName.has_value() ? indexName.value()
                                                     : indexID);
    }

    if (m_songInfo->metadata()->level.has_value()) {
        metadataList.push_back(m_songInfo->metadata()->level.value());
    }

    if (metadataList.size() > 0) {
        m_metadataLabel = CCLabelBMFont::create(
            metadataList.size() >= 1
                ? std::accumulate(
                      std::next(metadataList.begin()), metadataList.end(),
                      metadataList[0],
                      [](const std::string& a, const std::string& b) {
                          return a + ": " + b;
                      })
                      .c_str()
                : "",
            "bigFont.fnt");
        m_metadataLabel->limitLabelWidth(songInfoWidth, 0.4f, 0.1f);
        m_metadataLabel->setColor({.r = 162, .g = 191, .b = 255});
        m_metadataLabel->setID("metadata");
    }

    m_songNameLabel =
        CCLabelBMFont::create(songMetadata->name.c_str(), "bigFont.fnt");
    m_songNameLabel->limitLabelWidth(songInfoWidth, 0.7f, 0.1f);

    if (selected) {
        m_songNameLabel->setColor({188, 254, 206});
    }

    m_authorNameLabel =
        CCLabelBMFont::create(songMetadata->artist.c_str(), "goldFont.fnt");
    m_authorNameLabel->limitLabelWidth(songInfoWidth, 0.5f, 0.1f);
    m_authorNameLabel->setID("author-name");
    m_songNameLabel->setID("song-name");

    m_songInfoNode = CCNode::create();
    m_songInfoNode->setID("song-info-node");
    m_songInfoNode->setAnchorPoint({0.0f, 0.5f});
    m_songInfoNode->setContentSize({songInfoWidth, maxSize.height});

    m_songInfoNode->addChild(m_songNameLabel);
    m_songInfoNode->addChild(m_authorNameLabel);
    if (m_metadataLabel != nullptr) {
        m_songInfoNode->addChild(m_metadataLabel);
    }
    m_songInfoNode->setLayout(
        ColumnLayout::create()
            ->setAutoScale(false)
            ->setAxisReverse(true)
            ->setCrossAxisOverflow(false)
            ->setAxisAlignment(AxisAlignment::Even)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::Start));

    this->addChildAtPosition(m_songInfoNode, Anchor::Left, {PADDING_X, 0.0f});
    return true;
}

void NongCell::onFixDefault(CCObject* target) {
    if (!m_isActive) {
        FLAlertLayer::create("Error", "Set this NONG as <cr>active</c> first",
                             "Ok")
            ->show();
        return;
    }
    createQuickPopup(
        "Fix default",
        "Do you want to refetch song info <cb>for the default NONG</c>? Use "
        "this <cr>ONLY</c> if it gets renamed by accident!",
        "No", "Yes", [this](FLAlertLayer* alert, bool btn2) {
            if (btn2) {
                m_onFixDefault();
            }
        });
}

void NongCell::setDownloadProgress(float progress) {
    m_downloadProgressContainer->setVisible(true);
    auto sprite =
        CCSprite::createWithSpriteFrameName("GJ_cancelDownloadBtn_001.png");
    sprite->setScale(0.7f);
    m_downloadButton->setSprite(sprite);
    m_downloadButton->setColor(ccc3(105, 105, 105));
    m_downloadProgress->setPercentage(progress * 100.f);
}

void NongCell::onSet(CCObject* target) { m_onSelect(); }

void NongCell::onDownload(CCObject* target) { m_onDownload(); }

void NongCell::onEdit(CCObject* target) { m_onEdit(); }

void NongCell::onDelete(CCObject* target) { m_onDelete(); }

NongCell* NongCell::create(int songID, Song* song, bool isDefault,
                           bool selected, CCSize const& size,
                           std::function<void()> onSelect,
                           std::function<void()> onFixDefault,
                           std::function<void()> onDelete,
                           std::function<void()> onDownload,
                           std::function<void()> onEdit) {
    auto ret = new NongCell();
    if (ret && ret->init(songID, song, isDefault, selected, size, onSelect,
                         onFixDefault, onDelete, onDownload, onEdit)) {
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

}  // namespace jukebox
