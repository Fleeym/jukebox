#pragma once

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/loader/Event.hpp>

#include <jukebox/events/get_song_info.hpp>
#include <jukebox/events/song_download_failed.hpp>
#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/events/song_download_progress.hpp>
#include <jukebox/events/song_state_changed.hpp>
#include <jukebox/ui/list/nong_cell_ui.hpp>
#include <jukebox/nong/nong.hpp>
#include <optional>
#include "jukebox/nong/index.hpp"

namespace jukebox {

class NongDropdownLayer;

class NongCell : public cocos2d::CCNode {
protected:
    int m_songID;
    std::string m_uniqueID;
    std::optional<int> m_levelID;

    std::optional<index::IndexSongMetadata*> m_indexSongMetadataOpt;

    NongCellUI* m_nongCell;

    bool m_isDefault;
    bool m_isActive;
    bool m_isDownloaded;
    bool m_isDownloadable;
    bool m_isDownloading;
    bool m_isVerified;

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

    bool init(
        int gdSongID,
        const std::string& uniqueID,
        const cocos2d::CCSize& size,
        std::optional<int> levelID,
        std::optional<index::IndexSongMetadata*> indexSongMetadataOpt
    );

    geode::ListenerResult onDownloadProgress(event::SongDownloadProgress* e);
    geode::ListenerResult onGetSongInfo(event::GetSongInfo* e);
    geode::ListenerResult onDownloadFailed(event::SongDownloadFailed* e);
    geode::ListenerResult onDownloadFinish(event::SongDownloadFinished* e);
    geode::ListenerResult onStateChange(event::SongStateChanged* e);

    bool initLocal();
    bool initIndex();
    void build();

    void onSelect();
    void onTrash();
    void onFixDefault();
    void onDownload();
    void onEdit();

    void deleteSong(bool onlyAudio);
    bool isIndex();

public:
    static NongCell* create(
        int gdSongID,
        const std::string& uniqueID,
        const cocos2d::CCSize& size,
        std::optional<int> levelID,
        std::optional<index::IndexSongMetadata*> indexSongMetadataOpt
    );
};

}  // namespace jukebox

