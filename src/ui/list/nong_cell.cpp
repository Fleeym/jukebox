#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <fmt/format.h>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>
#include <numeric>

#include "../../managers/index_manager.hpp"
#include "nong_cell.hpp"

namespace jukebox {

bool NongCell::init(int songID, Nong info, bool isDefault, bool selected,
                    CCSize const& size, std::function<void()> onSelect,
                    std::function<void()> onFixDefault,
                    std::function<void()> onDelete,
                    std::function<void()> onDownload,
                    std::function<void()> onEdit) {
    if (!CCNode::init()) {
        return false;
    }

    m_songID = songID;
    m_songInfo = std::move(info);
    m_isDefault = isDefault;
    m_isActive = selected;
    m_isDownloaded = m_songInfo.path().has_value() &&
                     std::filesystem::exists(m_songInfo.path().value());
    m_isDownloadable = m_songInfo.visit<bool>([](auto _) { return false; },
                                              [](auto _) { return true; },
                                              [](auto _) { return true; });
    m_onSelect = onSelect;
    m_onFixDefault = onFixDefault;
    m_onDelete = onDelete;
    m_onDownload = onDownload;
    m_onEdit = onEdit;

    this->setContentSize(size);
    this->setAnchorPoint(CCPoint{0.5f, 0.5f});

    CCMenuItemSpriteExtra* button;

    auto bg = CCScale9Sprite::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(75);
    bg->setScale(0.3f);
    bg->setContentSize(size / bg->getScale());
    this->addChildAtPosition(bg, Anchor::Center);

    if (selected) {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        sprite->setScale(0.7f);
        button = CCMenuItemSpriteExtra::create(sprite, this, nullptr);
    } else {
        auto sprite =
            CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        sprite->setScale(0.7f);
        button = CCMenuItemSpriteExtra::create(sprite, this,
                                               menu_selector(NongCell::onSet));
    }
    button->setAnchorPoint(ccp(0.5f, 0.5f));
    button->setID("set-button");

    auto menu = CCMenu::create();
    if (m_isDownloaded) {
        menu->addChild(button);
    }
    menu->setAnchorPoint(CCPoint{1.0f, 0.5f});
    menu->setContentSize(CCSize{this->getContentSize().width, 200.f});

    menu->setLayout(
        RowLayout::create()->setGap(5.f)->setAxisAlignment(AxisAlignment::End));

    if (!m_isDefault && !m_songInfo.indexID().has_value()) {
        auto spr = CCSprite::create("JB_Edit.png"_spr);
        spr->setScale(0.7f);
        auto editButton = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(NongCell::onEdit));
        editButton->setID("edit-button");
        editButton->setAnchorPoint(ccp(0.5f, 0.5f));
        menu->addChild(editButton);
    }

    if (isDefault) {
        auto sprite =
            CCSprite::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
        sprite->setScale(0.8f);
        if (!selected) {
            sprite->setColor({0x80, 0x80, 0x80});
        }
        auto fixButton = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NongCell::onFixDefault));
        fixButton->setID("fix-button");
        menu->addChild(fixButton);
    } else if (m_isDownloadable && !m_isDownloaded) {
        auto sprite =
            CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        sprite->setScale(0.7f);
        m_downloadButton = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NongCell::onDownload));
        m_downloadButton->setID("download-button");
        menu->addChild(m_downloadButton);

        auto progressBarBack =
            CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
        progressBarBack->setColor(ccc3(50, 50, 50));
        progressBarBack->setScale(0.62f);

        auto spr = CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
        spr->setColor(ccc3(0, 255, 0));

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

    if (!m_isDefault &&
        !(m_songInfo.indexID().has_value() && !m_isDownloaded)) {
        bool trashSprite = m_isDownloadable && m_isDownloaded;
        auto sprite = CCSprite::createWithSpriteFrameName(
            trashSprite ? "GJ_trashBtn_001.png" : "GJ_deleteIcon_001.png");
        sprite->setScale(trashSprite ? 0.6475 : 0.7f);
        auto deleteButton = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NongCell::onDelete));
        deleteButton->setID("delete-button");
        menu->addChild(deleteButton);
    }

    menu->updateLayout();
    menu->setID("button-menu");
    this->addChildAtPosition(menu, Anchor::Right, CCPoint{-5.0f, 0.0f});

    m_songInfoLayer = CCLayer::create();
    auto songMetadata = m_songInfo.metadata();

    std::vector<std::string> metadataList = {};

    m_songInfo.visit<void>(
        [&metadataList](auto _) {},
        [&metadataList](YTSong* yt) { metadataList.push_back("youtube"); },
        [&metadataList](HostedSong* hosted) {
            metadataList.push_back("hosted");
        });

    if (m_songInfo.indexID().has_value()) {
        auto indexID = m_songInfo.indexID().value();
        auto indexName = IndexManager::get()->getIndexName(indexID);
        metadataList.push_back(indexName.has_value() ? indexName.value()
                                                     : indexID);
    }

    if (m_songInfo.metadata()->m_level.has_value()) {
        metadataList.push_back(m_songInfo.metadata()->m_level.value());
    }

    if (metadataList.size() > 0) {
        m_metadataLabel = CCLabelBMFont::create(
            metadataList.size() >= 1
                ? std::accumulate(std::next(metadataList.begin()),
                                  metadataList.end(), metadataList[0],
                                  [](const std::string& a, const std::string& b) {
                                      return a + " : " + b;
                                  })
                      .c_str()
                : "",
            "bigFont.fnt");
        m_metadataLabel->limitLabelWidth(220.f, 0.4f, 0.1f);
        m_metadataLabel->setColor({0x00, 0xc9, 0xff});
        m_metadataLabel->setID("metadata");
    }

    m_songNameLabel =
        CCLabelBMFont::create(songMetadata->m_name.c_str(), "bigFont.fnt");
    m_songNameLabel->limitLabelWidth(220.f, 0.7f, 0.1f);

    if (selected) {
        m_songNameLabel->setColor(ccc3(188, 254, 206));
    }

    m_authorNameLabel =
        CCLabelBMFont::create(songMetadata->m_artist.c_str(), "goldFont.fnt");
    m_authorNameLabel->limitLabelWidth(220.f, 0.7f, 0.1f);
    m_authorNameLabel->setID("author-name");
    m_songNameLabel->setID("song-name");

    if (m_metadataLabel != nullptr) {
        m_songInfoLayer->addChild(m_metadataLabel);
    }
    m_songInfoLayer->addChild(m_authorNameLabel);
    m_songInfoLayer->addChild(m_songNameLabel);
    m_songInfoLayer->setID("song-info");
    auto layout = ColumnLayout::create();
    layout->setAutoScale(false);
    if (m_metadataLabel != nullptr) {
        layout->setAxisAlignment(AxisAlignment::Even);
    } else {
        layout->setAxisAlignment(AxisAlignment::Center);
    }
    layout->setCrossAxisLineAlignment(AxisAlignment::Start);
    m_songInfoLayer->setContentSize(ccp(240.f, this->getContentSize().height));
    m_songInfoLayer->setAnchorPoint(ccp(0.f, 0.f));
    m_songInfoLayer->setPosition(ccp(12.f, 0.f));
    m_songInfoLayer->setLayout(layout);

    this->addChild(m_songInfoLayer);
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

NongCell* NongCell::create(int songID, Nong info, bool isDefault, bool selected,
                           CCSize const& size, std::function<void()> onSelect,
                           std::function<void()> onFixDefault,
                           std::function<void()> onDelete,
                           std::function<void()> onDownload,
                           std::function<void()> onEdit) {
    auto ret = new NongCell();
    if (ret &&
        ret->init(songID, std::move(info), isDefault, selected, size, onSelect,
                  onFixDefault, onDelete, onDownload, onEdit)) {
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

}  // namespace jukebox
