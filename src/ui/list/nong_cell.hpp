#pragma once

#include <functional>
#include <optional>

#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCObject.h"

#include "Geode/loader/Event.hpp"
#include "events/get_song_info.hpp"
#include "events/song_download_failed.hpp"
#include "events/song_download_progress.hpp"
#include "events/song_state_changed.hpp"
#include "nong.hpp"
#include "spitfire/reactive_node.hpp"

using namespace geode::prelude;

namespace jukebox {

struct NongCellState {
    bool active;
    bool isDownloaded;
    std::optional<float> downloadProgress = std::nullopt;
};

class NongDropdownLayer;

class NongCell : public spitfire::ReactiveNode<NongCellState> {
protected:
    Nongs* m_parent = nullptr;
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

    bool m_isDownloadable;
    bool m_isDefault;

    CCMenuItemSpriteExtra* m_fixButton = nullptr;
    CCMenuItemSpriteExtra* m_downloadButton = nullptr;
    CCMenuItemSpriteExtra* m_selectButton = nullptr;
    CCMenu* m_downloadProgressContainer = nullptr;
    CCProgressTimer* m_downloadProgress = nullptr;

    EventListener<EventFilter<event::SongDownloadProgress>> m_progressListener{
        this, &NongCell::onDownloadProgress};
    EventListener<EventFilter<event::GetSongInfo>> m_songInfoListener{
        this, &NongCell::onGetSongInfo};
    EventListener<EventFilter<event::SongDownloadFailed>>
        m_downloadFailedListener{this, &NongCell::onDownloadFailed};
    EventListener<EventFilter<event::SongStateChanged>> m_stateListener{
        this, &NongCell::onStateChange};

    bool init(Nongs* parent, Song*, bool isDefault, bool selected,
              CCSize const& size, std::function<void()> onSelect,
              std::function<void()> onDelete, std::function<void()> onDownload,
              std::function<void()> onEdit);
    
    void onChanges(const NongCellState& state);
    ListenerResult onDownloadProgress(event::SongDownloadProgress* e);
    ListenerResult onGetSongInfo(event::GetSongInfo* e);
    ListenerResult onDownloadFailed(event::SongDownloadFailed* e);
    ListenerResult onStateChange(event::SongStateChanged* e);

public:
    Song* m_songInfo = nullptr;
    static NongCell* create(Nongs* parent, Song* song, bool isDefault,
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
