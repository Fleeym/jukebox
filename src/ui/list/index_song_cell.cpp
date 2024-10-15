#include "ui/list/index_song_cell.hpp"

#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/cocos/base_nodes/Layout.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/platform/CCPlatformMacros.h"

#include "Geode/loader/Log.hpp"
#include "ccTypes.h"
#include "index.hpp"

using namespace jukebox::index;

namespace jukebox {

bool IndexSongCell::init(IndexSongMetadata* song, const CCSize& size) {
    if (!CCNode::init()) {
        return false;
    }

    m_song = song;

    this->setContentSize(size);
    this->setAnchorPoint({0.5f, 0.5f});

    CCScale9Sprite* bg = CCScale9Sprite::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(75);
    bg->setScale(0.3f);
    bg->setContentSize(size / bg->getScale());
    this->addChildAtPosition(bg, Anchor::Center);

    m_songInfoNode = CCNode::create();
    m_songInfoNode->setAnchorPoint({0.0f, 0.5f});
    m_songInfoNode->setContentSize({220.0f, size.height - 12.0f});

    m_songNameLabel =
        CCLabelBMFont::create(m_song->name.c_str(), "bigFont.fnt");
    m_songNameLabel->setAnchorPoint({0.0f, 0.5f});
    m_songNameLabel->limitLabelWidth(220.0f, 0.56f, 0.1f);

    m_artistLabel =
        CCLabelBMFont::create(m_song->artist.c_str(), "goldFont.fnt");
    m_artistLabel->setAnchorPoint({0.0f, 0.5f});
    m_artistLabel->limitLabelWidth(220.0f, 0.5f, 0.1f);

    m_indexNameLabel =
        CCLabelBMFont::create(m_song->parentID->m_name.c_str(), "bigFont.fnt");
    m_indexNameLabel->setAnchorPoint({0.0f, 0.5f});
    m_indexNameLabel->limitLabelWidth(220.0f, 0.4f, 0.1f);
    m_indexNameLabel->setColor(ccColor3B{.r = 162, .g = 191, .b = 255});

    m_songInfoNode->addChild(m_songNameLabel);
    m_songInfoNode->addChild(m_artistLabel);
    m_songInfoNode->addChild(m_indexNameLabel);
    m_songInfoNode->setLayout(
        ColumnLayout::create()
            ->setAutoScale(false)
            ->setAxisReverse(true)
            ->setCrossAxisOverflow(false)
            ->setAxisAlignment(AxisAlignment::Center)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::Start));
    this->addChildAtPosition(m_songInfoNode, Anchor::Left, {12.0f, 0.0f});

    return true;
}

IndexSongCell* IndexSongCell::create(IndexSongMetadata* song,
                                     const CCSize& size) {
    IndexSongCell* ret = new IndexSongCell();
    if (ret->init(song, size)) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

}  // namespace jukebox
