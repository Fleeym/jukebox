#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/ui/ScrollLayer.hpp>

#include <jukebox/events/manual_song_added.hpp>
#include <jukebox/events/nong_deleted.hpp>
#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/ui/list/nong_cell.hpp>

namespace jukebox {

class NongList final : public cocos2d::CCNode {
public:
    enum class ListType { Single = 0, Multiple = 1 };

protected:
    std::vector<int> m_songIds;
    geode::Ref<geode::ScrollLayer> m_list = nullptr;
    geode::Ref<cocos2d::extension::CCScale9Sprite> m_bg = nullptr;
    std::optional<int> m_currentSong = std::nullopt;
    std::optional<int> m_levelID;

    geode::Ref<CCMenuItemSpriteExtra> m_backBtn = nullptr;

    std::function<void(std::optional<int>)> m_onListTypeChange;

    geode::ListenerHandle m_downloadFinishedListener;
    geode::ListenerHandle m_nongDeletedListener;
    geode::ListenerHandle m_nongAddedListener;

    static constexpr float s_padding = 10.0f;
    static constexpr float s_itemSize = 60.f;

    void addNoLocalSongsNotice(bool liveInsert = false);
    void addSongToList(Song* nong, Nongs* parent, bool liveInsert = false);
    void addIndexSongToList(index::IndexSongMetadata* song, Nongs* parent);
    geode::ListenerResult onDownloadFinish(const event::SongDownloadFinishedData& e);
    geode::ListenerResult onNongDeleted(const event::NongDeletedData& e);
    geode::ListenerResult onSongAdded(const event::ManualSongAddedData& e);

public:
    void scrollToTop();
    void setCurrentSong(int songId);
    void build();
    void onBack(cocos2d::CCObject*);
    void onSelectSong(int songId);

    static NongList* create(std::vector<int> songIds, const cocos2d::CCSize& size, std::optional<int> levelID,
                            std::function<void(std::optional<int>)> onListTypeChange = {});

protected:
    bool init(std::vector<int> songIds, const cocos2d::CCSize& size, std::optional<int> levelID,
              std::function<void(std::optional<int>)> onListTypeChange = {});
};

}  // namespace jukebox
