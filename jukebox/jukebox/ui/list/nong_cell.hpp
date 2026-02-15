#pragma once

#include <optional>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/cocos.hpp>

#include <jukebox/events/get_song_info.hpp>
#include <jukebox/events/song_download_failed.hpp>
#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/events/song_download_progress.hpp>
#include <jukebox/events/song_state_changed.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/ui/list/nong_cell_ui.hpp>

namespace jukebox {

class NongDropdownLayer;

class NongCell : public cocos2d::CCNode {
protected:
    int m_songID = 0;
    std::string m_uniqueID;
    std::optional<int> m_levelID;

    std::optional<index::IndexSongMetadata*> m_indexSongMetadataOpt;

    geode::Ref<NongCellUI> m_nongCell;

    bool m_isDefault = false;
    bool m_isActive = false;
    bool m_isDownloaded = false;
    bool m_isDownloadable = false;
    bool m_isDownloading = false;
    bool m_isVerified = false;

    geode::ListenerHandle m_progressListener;
    geode::ListenerHandle m_songInfoListener;
    geode::ListenerHandle m_downloadSuccessListener;
    geode::ListenerHandle m_downloadFailedListener;
    geode::ListenerHandle m_stateListener;

    bool init(int gdSongID, const std::string& uniqueID, const cocos2d::CCSize& size, std::optional<int> levelID,
              std::optional<index::IndexSongMetadata*> indexSongMetadataOpt);

    geode::ListenerResult onDownloadProgress(const event::SongDownloadProgressData& e);
    geode::ListenerResult onGetSongInfo(const event::GetSongInfoData& e);
    geode::ListenerResult onDownloadFailed(const event::SongDownloadFailedData& e);
    geode::ListenerResult onDownloadFinish(const event::SongDownloadFinishedData& e);
    geode::ListenerResult onStateChange(const event::SongStateChangedData& e);

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
    static NongCell* create(int gdSongID, const std::string& uniqueID, const cocos2d::CCSize& size,
                            std::optional<int> levelID, std::optional<index::IndexSongMetadata*> indexSongMetadataOpt);
};

}  // namespace jukebox
