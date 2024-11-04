#pragma once

#include <vector>
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/binding/CustomSongWidget.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/platform/CCPlatformMacros.h"
#include "Geode/loader/Event.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/utils/cocos.hpp"

#include "events/song_download_failed.hpp"
#include "events/song_error.hpp"
#include "nong.hpp"
#include "ui/list/nong_cell.hpp"
#include "ui/list/nong_list.hpp"
#include "ui/list/song_cell.hpp"
#include "ui/nong_add_popup.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer
    : public Popup<std::vector<int>, CustomSongWidget*, int> {
protected:
    std::vector<int> m_songIDS;
    std::optional<int> m_currentSongID = std::nullopt;
    int m_defaultSongID;
    Ref<CustomSongWidget> m_parentWidget;
    NongList* m_list = nullptr;

    CCMenuItemSpriteExtra* m_addBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteBtn = nullptr;

    EventListener<EventFilter<event::SongError>> m_songErrorListener;
    EventListener<EventFilter<event::SongDownloadFailed>>
        m_downloadFailedListener;

    bool m_fetching = false;

    bool setup(std::vector<int> ids, CustomSongWidget* parent,
               int defaultSongID) override;
    void createList();
    CCSize getCellSize() const;
    void deleteAllNongs(CCObject*);
    void fetchSongFileHub(CCObject*);
    void onSettings(CCObject*);
    void openAddPopup(CCObject*);

public:
    void onSelectSong(int songID);
    void onDiscord(CCObject*);
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
