#pragma once

#include <optional>

#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/loader/Event.hpp"
#include "Geode/ui/ScrollLayer.hpp"

#include "events/song_download_finished.hpp"
#include "nong.hpp"
#include "ui/list/index_song_cell.hpp"
#include "ui/list/nong_cell.hpp"

namespace jukebox {

class NongList : public cocos2d::CCNode {
public:
    enum class ListType { Single = 0, Multiple = 1 };

protected:
    std::vector<int> m_songIds;
    geode::ScrollLayer* m_list;
    cocos2d::extension::CCScale9Sprite* m_bg;
    std::optional<int> m_currentSong = std::nullopt;

    CCMenuItemSpriteExtra* m_backBtn = nullptr;

    std::function<void(int, const std::string&)> m_onSetActive;
    std::function<void(int)> m_onFixDefault;
    std::function<void(int, const std::string&, bool onlyAudio, bool confirm)>
        m_onDelete;
    std::function<void(int, const std::string&)> m_onDownload;
    std::function<void(int, const std::string&)> m_onEdit;
    std::function<void(std::optional<int>)> m_onListTypeChange;

    std::vector<NongCell*> listedNongCells;

    geode::EventListener<EventFilter<event::SongDownloadFinished>>
        m_downloadFinishedListener = {this, &NongList::onDownloadFinish};

    static constexpr float s_padding = 10.0f;
    static constexpr float s_itemSize = 60.f;
    
    void addNoLocalSongsNotice(bool liveInsert = false);
    void addSongToList(Song* nong, Nongs* parent, bool liveInsert = false);
    void addIndexSongToList(index::IndexSongMetadata* song, Nongs* parent);
    geode::ListenerResult onDownloadFinish(event::SongDownloadFinished* e);

public:
    void scrollToTop();
    void setCurrentSong(int songId);
    void build();
    void onBack(cocos2d::CCObject*);
    void onSelectSong(int songId);
    void setDownloadProgress(std::string uniqueID, float progress);

    static NongList* create(
        std::vector<int>& songIds, const cocos2d::CCSize& size,
        std::function<void(int, const std::string&)> onSetActive,
        std::function<void(int)> onFixDefault,
        std::function<void(int, const std::string&, bool onlyAudio,
                           bool confirm)>
            onDelete,
        std::function<void(int, const std::string&)> onDownload,
        std::function<void(int, const std::string&)> onEdit,
        std::function<void(std::optional<int>)> onListTypeChange = {});

protected:
    bool init(std::vector<int>& songIds, const cocos2d::CCSize& size,
              std::function<void(int, const std::string&)> onSetActive,
              std::function<void(int)> onFixDefault,
              std::function<void(int, const std::string&, bool onlyAudio,
                                 bool confirm)>
                  onDelete,
              std::function<void(int, const std::string&)> onDownload,
              std::function<void(int, const std::string&)> onEdit,
              std::function<void(std::optional<int>)> onListTypeChange = {});
};

}  // namespace jukebox
