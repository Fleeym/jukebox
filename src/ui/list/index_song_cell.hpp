#pragma once

#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/misc_nodes/CCProgressTimer.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Event.hpp"

#include "events/song_download_failed.hpp"
#include "events/song_download_progress.hpp"
#include "index.hpp"

using namespace geode::prelude;
using namespace jukebox::index;

namespace jukebox {

class IndexSongCell : public CCNode {
protected:
    IndexSongMetadata* m_song = nullptr;
    int m_gdId;

    CCNode* m_songInfoNode = nullptr;
    CCLabelBMFont* m_songNameLabel = nullptr;
    CCLabelBMFont* m_artistLabel = nullptr;
    CCLabelBMFont* m_indexNameLabel = nullptr;

    CCMenu* m_downloadMenu = nullptr;
    CCMenuItemSpriteExtra* m_downloadButton = nullptr;

    CCNode* m_progressContainer = nullptr;
    CCProgressTimer* m_progressBar = nullptr;
    CCSprite* m_progressBarBack = nullptr;

    bool m_downloading = false;

    EventListener<EventFilter<event::SongDownloadProgress>> m_downloadListener;
    EventListener<EventFilter<event::SongDownloadFailed>>
        m_downloadFailedListener{this, &IndexSongCell::onDownloadFailed};

    bool init(IndexSongMetadata* song, int gdId, const CCSize& size);

    void onDownload(CCObject*);
    geode::ListenerResult onDownloadProgress(event::SongDownloadProgress* e);
    geode::ListenerResult onDownloadFailed(event::SongDownloadFailed* e);

public:
    IndexSongMetadata* song() const { return m_song; }

    static IndexSongCell* create(IndexSongMetadata* song, int gdId,
                                 const CCSize& size);
};

}  // namespace jukebox
