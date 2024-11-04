#include "ui/list/index_song_cell.hpp"

#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/base_nodes/Layout.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/platform/CCPlatformMacros.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Event.hpp"
#include "ccTypes.h"

#include "events/song_download_failed.hpp"
#include "events/song_download_progress.hpp"
#include "events/start_download.hpp"
#include "index.hpp"

using namespace jukebox::index;

namespace jukebox {

bool IndexSongCell::init(IndexSongMetadata* song, int gdId,
                         const CCSize& size) {
    if (!CCNode::init()) {
        return false;
    }

    m_song = song;
    m_gdId = gdId;

    m_downloadListener.bind(this, &IndexSongCell::onDownloadProgress);

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

    m_songInfoNode = CCNode::create();
    m_songInfoNode->setAnchorPoint({0.0f, 0.5f});
    m_songInfoNode->setContentSize({songInfoWidth, maxSize.height});
    m_songInfoNode->setID("song-info-node");

    m_songNameLabel =
        CCLabelBMFont::create(m_song->name.c_str(), "bigFont.fnt");
    m_songNameLabel->setAnchorPoint({0.0f, 0.5f});
    m_songNameLabel->limitLabelWidth(songInfoWidth, 0.56f, 0.1f);
    m_songNameLabel->setID("song-info-label");

    m_artistLabel =
        CCLabelBMFont::create(m_song->artist.c_str(), "goldFont.fnt");
    m_artistLabel->setAnchorPoint({0.0f, 0.5f});
    m_artistLabel->limitLabelWidth(songInfoWidth, 0.5f, 0.1f);
    m_songNameLabel->setID("artist-label");

    m_indexNameLabel =
        CCLabelBMFont::create(m_song->parentID->m_name.c_str(), "bigFont.fnt");
    m_indexNameLabel->setAnchorPoint({0.0f, 0.5f});
    m_indexNameLabel->limitLabelWidth(songInfoWidth, 0.4f, 0.1f);
    m_indexNameLabel->setColor({.r = 162, .g = 191, .b = 255});
    m_songNameLabel->setID("index-name-label");

    m_songInfoNode->addChild(m_songNameLabel);
    m_songInfoNode->addChild(m_artistLabel);
    m_songInfoNode->addChild(m_indexNameLabel);
    m_songInfoNode->setLayout(
        ColumnLayout::create()
            ->setAutoScale(false)
            ->setAxisReverse(true)
            ->setCrossAxisOverflow(false)
            ->setAxisAlignment(AxisAlignment::Even)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisOverflow(true)
            ->setCrossAxisLineAlignment(AxisAlignment::Start));
    this->addChildAtPosition(m_songInfoNode, Anchor::Left, {PADDING_X, 0.0f});

    m_downloadMenu = CCMenu::create();
    m_downloadMenu->ignoreAnchorPointForPosition(false);
    m_downloadMenu->setAnchorPoint({1.0f, 0.5f});
    m_downloadMenu->setID("download-menu");
    m_downloadMenu->setContentSize({buttonsWidth, maxSize.height});

    CCSprite* downloadSpr =
        CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
    downloadSpr->setScale(0.7f);
    m_downloadButton = CCMenuItemSpriteExtra::create(
        downloadSpr, this, menu_selector(IndexSongCell::onDownload));
    m_downloadButton->setID("download-button");

    m_progressBarBack =
        CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
    m_progressBarBack->setColor({50, 50, 50});
    m_progressBarBack->setScale(0.62f);

    CCSprite* spr = CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
    spr->setColor({0, 255, 0});

    m_progressBar = CCProgressTimer::create(spr);
    m_progressBar->setType(CCProgressTimerType::kCCProgressTimerTypeRadial);
    m_progressBar->setPercentage(50.f);
    m_progressBar->setID("progress-bar");
    m_progressBar->setScale(0.66f);

    m_progressContainer = CCNode::create();
    m_progressContainer->setAnchorPoint({0.5f, 0.5f});
    m_progressContainer->setContentSize(
        m_downloadButton->getScaledContentSize());
    m_progressContainer->setContentSize(m_progressBar->getScaledContentSize());
    m_progressContainer->addChildAtPosition(m_progressBar, Anchor::Center);
    m_progressContainer->addChildAtPosition(m_progressBarBack, Anchor::Center);
    m_progressContainer->setZOrder(-1);
    m_progressContainer->setVisible(false);

    m_downloadButton->addChildAtPosition(m_progressContainer, Anchor::Center);
    m_downloadMenu->addChild(m_downloadButton);
    m_downloadMenu->setLayout(
        RowLayout::create()->setAxisReverse(true)->setAxisAlignment(
            AxisAlignment::End));

    this->addChildAtPosition(m_downloadMenu, Anchor::Right, {-PADDING_X, 0.0f});
    return true;
}

void IndexSongCell::onDownload(CCObject*) {
    if (m_downloading) {
        return;
    }

    m_downloading = true;
    CCSprite* newSpr =
        CCSprite::createWithSpriteFrameName("GJ_cancelDownloadBtn_001.png");
    newSpr->setScale(0.7f);
    m_downloadButton->setSprite(newSpr);
    m_downloadButton->setColor(ccc3(105, 105, 105));
    m_progressContainer->setVisible(true);
    m_progressBar->setPercentage(0.0f);

    event::StartDownload(m_song, m_gdId).post();
}

ListenerResult IndexSongCell::onDownloadProgress(
    event::SongDownloadProgress* e) {
    if (e->gdSongID() != m_gdId || e->uniqueID() != m_song->uniqueID) {
        return ListenerResult::Propagate;
    }

    if (!m_progressContainer->isVisible()) {
        CCSprite* newSpr =
            CCSprite::createWithSpriteFrameName("GJ_cancelDownloadBtn_001.png");
        newSpr->setScale(0.7f);
        m_downloadButton->setSprite(newSpr);
        m_downloadButton->setColor(ccc3(105, 105, 105));
        m_progressContainer->setVisible(true);
    }

    m_progressBar->setPercentage(e->progress());
    return ListenerResult::Propagate;
}

ListenerResult IndexSongCell::onDownloadFailed(event::SongDownloadFailed* e) {
    if (e->gdSongId() != m_gdId || e->uniqueId() != m_song->uniqueID) {
        return ListenerResult::Propagate;
    }

    m_progressContainer->setVisible(false);
    m_progressBar->setPercentage(0.0f);
    CCSprite* downloadSpr =
        CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
    downloadSpr->setScale(0.7f);
    m_downloadButton->setSprite(downloadSpr);
    m_downloadButton->setColor({255, 255, 255});
    m_downloading = false;

    return ListenerResult::Propagate;
}

IndexSongCell* IndexSongCell::create(IndexSongMetadata* song, int gdId,
                                     const CCSize& size) {
    IndexSongCell* ret = new IndexSongCell();
    if (ret->init(song, gdId, size)) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

}  // namespace jukebox
