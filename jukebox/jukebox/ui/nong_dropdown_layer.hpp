#pragma once

#include <vector>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/platform/CCPlatformMacros.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>

#include <jukebox/events/get_song_info.hpp>
#include <jukebox/events/song_download_failed.hpp>
#include <jukebox/events/song_error.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/ui/list/nong_cell.hpp>
#include <jukebox/ui/list/nong_list.hpp>
#include <jukebox/ui/list/song_cell.hpp>
#include <jukebox/ui/nong_add_popup.hpp>

namespace jukebox {

class NongDropdownLayer
    : public geode::Popup<std::vector<int>, CustomSongWidget*, int> {
protected:
    std::vector<int> m_songIDS;
    std::optional<int> m_currentSongID = std::nullopt;
    int m_defaultSongID;
    geode::Ref<CustomSongWidget> m_parentWidget;
    NongList* m_list = nullptr;

    CCMenuItemSpriteExtra* m_addBtn = nullptr;
    CCMenuItemSpriteExtra* m_discordBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteBtn = nullptr;

    geode::EventListener<geode::EventFilter<jukebox::event::SongError>>
        m_songErrorListener;
    geode::EventListener<geode::EventFilter<jukebox::event::GetSongInfo>>
        m_songInfoListener;
    geode::EventListener<geode::EventFilter<jukebox::event::SongDownloadFailed>>
        m_downloadFailedListener;

    bool m_fetching = false;

    bool setup(std::vector<int> ids, CustomSongWidget* parent,
               int defaultSongID) override;
    void createList();
    cocos2d::CCSize getCellSize() const;
    void deleteAllNongs(cocos2d::CCObject*);
    void fetchSongFileHub(cocos2d::CCObject*);
    void onSettings(cocos2d::CCObject*);
    void openAddPopup(cocos2d::CCObject*);

public:
    void onSelectSong(int songID);
    void onDiscord(cocos2d::CCObject*);
    void setActiveSong(int gdSongID, const std::string& uniqueID);
    void deleteSong(int gdSongID, const std::string& uniqueID, bool onlyAudio,
                    bool confirm);
    void downloadSong(int gdSongID, const std::string& uniqueID);
    void addSong(Nongs&& song, bool popup = true);
    void updateParentWidget(SongMetadata const& song);

    static NongDropdownLayer* create(std::vector<int> ids,
                                     CustomSongWidget* parent,
                                     int defaultSongID) {
        auto ret = new NongDropdownLayer;
        if (ret && ret->initAnchored(420.f, 280.f, ids, parent, defaultSongID,
                                     "GJ_square01.png")) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

}  // namespace jukebox
