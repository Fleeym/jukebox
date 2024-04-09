#include "song_cell.hpp"

#include <Geode/utils/cocos.hpp>
#include <ccTypes.h>
#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <fmt/format.h>

namespace jukebox {

bool JBSongCell::init(NongData data, int id, NongDropdownLayer* parentPopup, CCSize const& size) {
    if (!JBListCell::init(parentPopup, size)) return false;
    m_parentPopup = parentPopup;
    m_songID = id;
    for (auto const& song : data.songs) {
        if (song.path == data.active) {
            m_active = song;
        }
    }
    auto label = CCLabelBMFont::create(m_active.songName.c_str(), "bigFont.fnt");
    label->setAnchorPoint(ccp(0, 0.5f));
    label->limitLabelWidth(240.f, 0.8f, 0.1f);
    label->setPosition(ccp(12.f, 40.f));
    this->addChild(label);
    m_songNameLabel = label;
    auto author = CCLabelBMFont::create(m_active.authorName.c_str(), "goldFont.fnt");
    author->setAnchorPoint(ccp(0, 0.5f));
    author->limitLabelWidth(260.f, 0.6f, 0.1f);
    author->setPosition(ccp(12.f, 15.f));
    m_authorNameLabel = author;
    this->addChild(author);
    auto idLabel = CCLabelBMFont::create(
        fmt::format("#{}", id).c_str(),
        "chatFont.fnt"
    );
    idLabel->setPosition({ size.width - 5.f, 0 + 3.f });
    idLabel->setAnchorPoint({ 1.0f, 0.0f });
    idLabel->setColor(ccColor3B(51, 51, 51));
    idLabel->setScale(0.6f);
    m_songIDLabel = idLabel;
    this->addChild(idLabel);
    auto menu = CCMenu::create();
    auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    spr->setFlipX(true);
    spr->setScale(0.8f);
    auto btn = CCMenuItemSpriteExtra::create(
        spr,
        this, 
        menu_selector(JBSongCell::onSelectSong)
    );
    menu->addChild(btn);
    this->addChild(menu);
    menu->setPosition(ccp(290.f, 30.f));
    return true;
}

void JBSongCell::onSelectSong(CCObject*) {
    m_parentPopup->onSelectSong(m_songID);
}

}