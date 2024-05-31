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

#include "nong_cell.hpp"

namespace jukebox {

bool NongCell::init(
    int songID,
    SongInfo info,
    bool isDefault,
    bool selected, 
    CCSize const& size,
    std::function<void()> onSelect,
    std::function<void()> onFixDefault,
    std::function<void()> onDelete
) {
    if (!CCNode::init()) {
        return false;
    }

    m_songID = songID;
    m_songInfo = info;
    m_isDefault = isDefault;
    m_isActive = selected;
    m_onSelect = onSelect;
    m_onFixDefault = onFixDefault;
    m_onDelete = onDelete;

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
    menu->addChild(button);
    menu->setAnchorPoint(CCPoint { 1.0f, 0.5f });
    menu->setContentSize(CCSize { 50.f, 30.f });

    if (!isDefault) {
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
        sprite->setScale(0.7f);
        auto deleteButton = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(NongCell::deleteSong)
        );
        deleteButton->setID("delete-button");
        menu->addChild(deleteButton);
    } else {
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

    if (!m_songInfo.levelName.empty()) {
        m_levelNameLabel = CCLabelBMFont::create(m_songInfo.levelName.c_str(), "bigFont.fnt");
        m_levelNameLabel->limitLabelWidth(220.f, 0.4f, 0.1f);
        m_levelNameLabel->setColor(cc3x(0x00c9ff));
        m_levelNameLabel->setID("level-name");
    }
    
    m_songNameLabel = CCLabelBMFont::create(m_songInfo.songName.c_str(), "bigFont.fnt");
    m_songNameLabel->limitLabelWidth(220.f, 0.7f, 0.1f);

    if (selected) {
        m_songNameLabel->setColor(ccc3(188, 254, 206));
    }

    m_authorNameLabel = CCLabelBMFont::create(m_songInfo.authorName.c_str(), "goldFont.fnt");
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

void NongCell::onSet(CCObject* target) {
    m_onSelect();
}

void NongCell::deleteSong(CCObject* target) {
    createQuickPopup(
        "Are you sure?",
        fmt::format("Are you sure you want to delete <cy>{}</c> from your NONGs?", m_songInfo.songName),
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
    SongInfo info,
    bool isDefault,
    bool selected, 
    CCSize const& size,
    std::function<void()> onSelect,
    std::function<void()> onFixDefault,
    std::function<void()> onDelete
) {
    auto ret = new NongCell();
    if (ret && ret->init(songID, info, isDefault, selected, size, onSelect, onFixDefault, onDelete)) {
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

}
