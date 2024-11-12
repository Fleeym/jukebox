#pragma once

#include <functional>

#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCObject.h"

#include "Geode/loader/Event.hpp"
#include "events/get_song_info.hpp"
#include "events/song_download_failed.hpp"
#include "events/song_download_progress.hpp"
#include "events/song_state_changed.hpp"
#include "nong.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class NongCell : public CCNode {
protected:
    int m_songID;
    std::string m_uniqueID;
    CCLabelBMFont* m_songNameLabel = nullptr;
    CCLabelBMFont* m_authorNameLabel = nullptr;
    CCLabelBMFont* m_metadataLabel = nullptr;

    CCNode* m_songInfoNode = nullptr;

    std::function<void()> m_onSelect;
    std::function<void()> m_onDelete;
    std::function<void()> m_onDownload;
    std::function<void()> m_onEdit;

    bool m_isDefault;
    bool m_isActive;
    bool m_isDownloaded;
    bool m_isDownloadable;

    CCMenuItemSpriteExtra* m_downloadButton;
    CCMenuItemSpriteExtra* m_selectButton = nullptr;
    CCMenu* m_downloadProgressContainer;
    CCProgressTimer* m_downloadProgress;

    EventListener<EventFilter<event::SongDownloadProgress>> m_progressListener{
        this, &NongCell::onDownloadProgress};
    EventListener<EventFilter<event::GetSongInfo>> m_songInfoListener{
        this, &NongCell::onGetSongInfo};
    EventListener<EventFilter<event::SongDownloadFailed>>
        m_downloadFailedListener{this, &NongCell::onDownloadFailed};
    EventListener<EventFilter<event::SongStateChanged>> m_stateListener{
        this, &NongCell::onStateChange};

    bool init(int songID, Song*, bool isDefault, bool selected,
              CCSize const& size, std::function<void()> onSelect,
              std::function<void()> onDelete, std::function<void()> onDownload,
              std::function<void()> onEdit);

    ListenerResult onDownloadProgress(event::SongDownloadProgress* e);
    ListenerResult onGetSongInfo(event::GetSongInfo* e);
    ListenerResult onDownloadFailed(event::SongDownloadFailed* e);
    ListenerResult onStateChange(event::SongStateChanged* e);

public:
    Song* m_songInfo = nullptr;
    static NongCell* create(int songID, Song* song, bool isDefault,
                            bool selected, CCSize const& size,
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
