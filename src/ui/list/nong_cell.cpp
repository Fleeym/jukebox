#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>
#include <fmt/format.h>

#include "../../managers/index_manager.hpp"
#include "nong_cell.hpp"

namespace jukebox {

bool NongCell::init(
    int songID,
    Nong info,
    bool isDefault,
    bool selected,
    bool isDownloaded,
    CCSize const& size,
    std::function<void()> onSelect,
    std::function<void()> onFixDefault,
    std::function<void()> onDelete,
    std::function<void()> onDownload
) {
    if (!CCNode::init()) {
        return false;
    }

    m_songID = songID;
    m_songInfo = std::move(info);
    m_isDefault = isDefault;
    m_isActive = selected;
    m_isDownloaded = isDownloaded;
    m_onSelect = onSelect;
    m_onFixDefault = onFixDefault;
    m_onDelete = onDelete;
    m_onDownload = onDownload;

    this->setContentSize(size);
    this->setAnchorPoint(CCPoint { 0.5f, 0.5f });

    CCMenuItemSpriteExtra* button;

    auto bg = CCScale9Sprite::create("square02b_001.png");
    bg->setColor({ 0, 0, 0 });
    bg->setOpacity(75);
    bg->setScale(0.3f);
    bg->setContentSize(size / bg->getScale());
    this->addChildAtPosition(bg, Anchor::Center);

    if (selected) {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        sprite->setScale(0.7f);
        button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            nullptr
        );
    } else {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        sprite->setScale(0.7f);
        button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(NongCell::onSet)
        );
    }
    button->setAnchorPoint(ccp(0.5f, 0.5f));
    button->setID("set-button");

    auto menu = CCMenu::create();
    if (m_songInfo.path().has_value()) {
      menu->addChild(button);
    }
    menu->setAnchorPoint(CCPoint { 1.0f, 0.5f });
    menu->setContentSize(CCSize { 50.f, 30.f });

    if (isDefault) {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
        sprite->setScale(0.8f);
        if (!selected) {
            sprite->setColor(cc3x(0x808080));
        }
        auto fixButton = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(NongCell::onFixDefault)
        );
        fixButton->setID("fix-button");
        menu->addChild(fixButton);
    } else if (!m_isDownloaded) {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        sprite->setScale(0.8f);
        m_downloadButton = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(NongCell::onDownload)
        );
        m_downloadButton->setID("download-button");
        menu->addChild(m_downloadButton);

        auto progressBarBack = CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
        progressBarBack->setColor(ccc3(50, 50, 50));
        progressBarBack->setScale(0.62f);

        auto spr = CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
        spr->setColor(ccc3(0, 255, 0));

        m_downloadProgress = CCProgressTimer::create(spr);
        m_downloadProgress->setType(CCProgressTimerType::kCCProgressTimerTypeRadial);
        m_downloadProgress->setPercentage(50.f);
        m_downloadProgress->setID("progress-bar");
        m_downloadProgress->setScale(0.66f);

        m_downloadProgressContainer = CCMenu::create();

        m_downloadProgressContainer->addChildAtPosition(m_downloadProgress, Anchor::Center);
        m_downloadProgressContainer->addChildAtPosition(progressBarBack, Anchor::Center);

        m_downloadProgressContainer->setZOrder(-1);
        m_downloadProgressContainer->setVisible(false);

        m_downloadButton->addChildAtPosition(m_downloadProgressContainer, Anchor::Center);
        // TODO initial set download progress
    } else {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
        sprite->setScale(0.7f);
        auto deleteButton = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(NongCell::onDelete)
        );
        deleteButton->setID("delete-button");
        menu->addChild(deleteButton);
    }

    menu->setLayout(
        RowLayout::create()
            ->setGap(5.f)
            ->setAxisAlignment(AxisAlignment::Even)
    );
    menu->updateLayout();
    menu->setID("button-menu");
    this->addChildAtPosition(menu, Anchor::Right, CCPoint { -5.0f, 0.0f });

    m_songInfoLayer = CCLayer::create();
    auto songMetadata = m_songInfo.metadata();

    if (songMetadata->m_level.has_value()) {
        m_levelNameLabel = CCLabelBMFont::create(songMetadata->m_level->c_str(), "bigFont.fnt");
        m_levelNameLabel->limitLabelWidth(220.f, 0.4f, 0.1f);
        m_levelNameLabel->setColor(cc3x(0x00c9ff));
        m_levelNameLabel->setID("level-name");
    }

    m_songNameLabel = CCLabelBMFont::create(songMetadata->m_name.c_str(), "bigFont.fnt");
    m_songNameLabel->limitLabelWidth(220.f, 0.7f, 0.1f);

    if (selected) {
        m_songNameLabel->setColor(ccc3(188, 254, 206));
    }

    m_authorNameLabel = CCLabelBMFont::create(songMetadata->m_artist.c_str(), "goldFont.fnt");
    m_authorNameLabel->limitLabelWidth(220.f, 0.7f, 0.1f);
    m_authorNameLabel->setID("author-name");
    m_songNameLabel->setID("song-name");

    if (m_levelNameLabel != nullptr) {
        m_songInfoLayer->addChild(m_levelNameLabel);
    }
    m_songInfoLayer->addChild(m_authorNameLabel);
    m_songInfoLayer->addChild(m_songNameLabel);
    m_songInfoLayer->setID("song-info");
    auto layout = ColumnLayout::create();
    layout->setAutoScale(false);
    if (m_levelNameLabel != nullptr) {
        layout->setAxisAlignment(AxisAlignment::Even);
    } else {
        layout->setAxisAlignment(AxisAlignment::Center);
    }
    layout->setCrossAxisLineAlignment(AxisAlignment::Start);
    m_songInfoLayer->setContentSize(ccp(240.f, this->getContentSize().height - 6.f));
    m_songInfoLayer->setAnchorPoint(ccp(0.f, 0.f));
    m_songInfoLayer->setPosition(ccp(12.f, 1.5f));
    m_songInfoLayer->setLayout(layout);

    this->addChild(m_songInfoLayer);
    return true;
}

void NongCell::onFixDefault(CCObject* target) {
    if (!m_isActive) {
        FLAlertLayer::create("Error", "Set this NONG as <cr>active</c> first", "Ok")->show();
        return;
    }
    createQuickPopup(
        "Fix default",
        "Do you want to refetch song info <cb>for the default NONG</c>? Use this <cr>ONLY</c> if it gets renamed by accident!",
        "No",
        "Yes",
        [this](FLAlertLayer* alert, bool btn2) {
            if (btn2) {
                m_onFixDefault();
            }
        }
    );
}

void NongCell::setDownloadProgress(float progress) {
    m_downloadProgressContainer->setVisible(true);
    auto sprite = CCSprite::createWithSpriteFrameName("GJ_cancelDownloadBtn_001.png");
    sprite->setScale(0.8f);
    m_downloadButton->setSprite(sprite);
    m_downloadButton->setColor(ccc3(105, 105, 105));
    m_downloadProgress->setPercentage(progress*100.f);
}

void NongCell::onSet(CCObject* target) {
    m_onSelect();
}

void NongCell::onDownload(CCObject* target) {
    m_onDownload();
}

void NongCell::onDelete(CCObject* target) {
    createQuickPopup(
        "Are you sure?",
        fmt::format("Are you sure you want to delete <cy>{}</c> from your NONGs?", m_songInfo.metadata()->m_name),
        "No",
        "Yes",
        [this] (FLAlertLayer* self, bool btn2) {
            if (btn2) {
                m_onDelete();
            }
        }
    );
}

NongCell* NongCell::create(
    int songID,
    Nong info,
    bool isDefault,
    bool selected,
    bool isDownloaded,
    CCSize const& size,
    std::function<void()> onSelect,
    std::function<void()> onFixDefault,
    std::function<void()> onDelete,
    std::function<void()> onDownload
) {
    auto ret = new NongCell();
    if (ret && ret->init(songID, std::move(info), isDefault, selected, isDownloaded, size, onSelect, onFixDefault, onDelete, onDownload)) {
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

}
