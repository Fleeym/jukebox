#pragma once

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/misc_nodes/CCProgressTimer.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/Event.hpp>

#include <jukebox/events/song_download_failed.hpp>
#include <jukebox/events/song_download_progress.hpp>
#include <jukebox/nong/index.hpp>

namespace jukebox {

class IndexSongCell : public cocos2d::CCNode {
protected:
    index::IndexSongMetadata* m_song = nullptr;
    int m_gdId;

    CCNode* m_songInfoNode = nullptr;
    cocos2d::CCLabelBMFont* m_songNameLabel = nullptr;
    cocos2d::CCLabelBMFont* m_artistLabel = nullptr;
    cocos2d::CCLabelBMFont* m_indexNameLabel = nullptr;

    cocos2d::CCMenu* m_downloadMenu = nullptr;
    CCMenuItemSpriteExtra* m_downloadButton = nullptr;

    cocos2d::CCNode* m_progressContainer = nullptr;
    cocos2d::CCProgressTimer* m_progressBar = nullptr;
    cocos2d::CCSprite* m_progressBarBack = nullptr;

    bool m_downloading = false;

    geode::EventListener<
        geode::EventFilter<jukebox::event::SongDownloadProgress>>
        m_downloadListener;
    geode::EventListener<geode::EventFilter<jukebox::event::SongDownloadFailed>>
        m_downloadFailedListener{this, &IndexSongCell::onDownloadFailed};

    bool init(index::IndexSongMetadata* song, int gdId,
              const cocos2d::CCSize& size);

    void onDownload(CCObject*);
    geode::ListenerResult onDownloadProgress(event::SongDownloadProgress* e);
    geode::ListenerResult onDownloadFailed(event::SongDownloadFailed* e);

public:
    index::IndexSongMetadata* song() const { return m_song; }

    static IndexSongCell* create(index::IndexSongMetadata* song, int gdId,
                                 const cocos2d::CCSize& size);
};

}  // namespace jukebox
