#include "song_cell.hpp"

#include <ccTypes.h>
#include <fmt/format.h>
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/ui/Layout.hpp"

namespace jukebox {

bool SongCell::init(int id, SongMetadata* songInfo, const CCSize& size,
                    std::function<void()> selectCallback) {
    if (!CCNode::init()) {
        return false;
    }
    m_songID = id;
    m_active = songInfo;
    m_callback = selectCallback;

    this->setContentSize(size);
    this->setAnchorPoint(CCPoint{0.5f, 0.5f});

    auto bg = CCScale9Sprite::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(75);
    bg->setScale(0.3f);
    bg->setContentSize(size / bg->getScale());
    this->addChildAtPosition(bg, Anchor::Center);

    auto label = CCLabelBMFont::create(songInfo->name.c_str(), "bigFont.fnt");
    label->setAnchorPoint(ccp(0, 0.5f));
    label->limitLabelWidth(240.f, 0.8f, 0.1f);
    label->setPosition(ccp(12.f, 40.f));
    this->addChild(label);
    m_songNameLabel = label;
    auto author =
        CCLabelBMFont::create(songInfo->artist.c_str(), "goldFont.fnt");
    author->setAnchorPoint(ccp(0, 0.5f));
    author->limitLabelWidth(260.f, 0.6f, 0.1f);
    author->setPosition(ccp(12.f, 15.f));
    m_authorNameLabel = author;
    this->addChild(author);
    auto idLabel =
        CCLabelBMFont::create(fmt::format("#{}", id).c_str(), "chatFont.fnt");
    idLabel->setPosition({size.width - 5.f, 0 + 3.f});
    idLabel->setAnchorPoint({1.0f, 0.0f});
    idLabel->setColor(ccColor3B{230, 230, 230});
    idLabel->setScale(0.6f);
    m_songIDLabel = idLabel;
    this->addChild(idLabel);
    auto menu = CCMenu::create();
    auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    spr->setFlipX(true);
    spr->setScale(0.8f);
    auto btn = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(SongCell::onSelectSong));
    menu->addChild(btn);
    this->addChild(menu);
    menu->setPosition(ccp(290.f, 30.f));
    return true;
}

void SongCell::onSelectSong(CCObject*) { m_callback(); }

}  // namespace jukebox
