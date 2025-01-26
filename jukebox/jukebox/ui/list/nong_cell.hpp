#pragma once

#include <functional>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/loader/Event.hpp>

#include <jukebox/events/get_song_info.hpp>
#include <jukebox/events/song_download_failed.hpp>
#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/events/song_download_progress.hpp>
#include <jukebox/events/song_state_changed.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox {

class NongDropdownLayer;

class NongCell : public cocos2d::CCNode {
protected:
    int m_songID;
    std::string m_uniqueID;
    cocos2d::CCLabelBMFont* m_songNameLabel = nullptr;
    cocos2d::CCLabelBMFont* m_authorNameLabel = nullptr;
    cocos2d::CCLabelBMFont* m_metadataLabel = nullptr;

    cocos2d::CCNode* m_songInfoNode = nullptr;

    std::function<void()> m_onSelect;
    std::function<void()> m_onDelete;
    std::function<void()> m_onDownload;
    std::function<void()> m_onEdit;

    bool m_isDefault;
    bool m_isActive;
    bool m_isDownloaded;
    bool m_isDownloadable;

    cocos2d::CCMenu* m_buttonMenu = nullptr;
    CCMenuItemSpriteExtra* m_fixButton = nullptr;
    CCMenuItemSpriteExtra* m_downloadButton = nullptr;
    CCMenuItemSpriteExtra* m_selectButton = nullptr;
    cocos2d::CCMenu* m_downloadProgressContainer = nullptr;
    cocos2d::CCProgressTimer* m_downloadProgress = nullptr;

    geode::EventListener<geode::EventFilter<event::SongDownloadProgress>>
        m_progressListener{this, &NongCell::onDownloadProgress};
    geode::EventListener<geode::EventFilter<event::GetSongInfo>>
        m_songInfoListener{this, &NongCell::onGetSongInfo};
    geode::EventListener<geode::EventFilter<event::SongDownloadFinished>>
        m_downloadSuccessListener{this, &NongCell::onDownloadFinish};
    geode::EventListener<geode::EventFilter<event::SongDownloadFailed>>
        m_downloadFailedListener{this, &NongCell::onDownloadFailed};
    geode::EventListener<geode::EventFilter<event::SongStateChanged>>
        m_stateListener{this, &NongCell::onStateChange};

    bool init(int songID, Song*, bool isDefault, bool selected,
              const cocos2d::CCSize& size, std::function<void()> onSelect,
              std::function<void()> onDelete, std::function<void()> onDownload,
              std::function<void()> onEdit);

    geode::ListenerResult onDownloadProgress(event::SongDownloadProgress* e);
    geode::ListenerResult onGetSongInfo(event::GetSongInfo* e);
    geode::ListenerResult onDownloadFailed(event::SongDownloadFailed* e);
    geode::ListenerResult onDownloadFinish(event::SongDownloadFinished* e);
    geode::ListenerResult onStateChange(event::SongStateChanged* e);

public:
    Song* m_songInfo = nullptr;
    static NongCell* create(int songID, Song* song, bool isDefault,
                            bool selected, const cocos2d::CCSize& size,
                            std::function<void()> onSelect,
                            std::function<void()> onDelete,
                            std::function<void()> onDownload,
                            std::function<void()> onEdit);

    void onSet(CCObject*);
    void onDelete(CCObject*);
    void onFixDefault(CCObject*);
    void onDownload(CCObject*);
    void onEdit(CCObject*);
};

}  // namespace jukebox
