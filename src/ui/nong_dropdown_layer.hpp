#pragma once

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/ui/ListView.hpp>

#include "../types/song_info.hpp"
#include "../types/nong_list_type.hpp"
#include "nong_add_popup.hpp"
#include "nong_cell.hpp"
#include "song_cell.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer : public Popup<std::vector<int>, CustomSongWidget*, int> {
protected:
    std::map<int, NongData> m_data;
    std::vector<int> m_songIDS;
    int m_currentSongID = -1;
    int m_defaultSongID;
    Ref<CustomSongWidget> m_parentWidget;
    ListView* m_listView = nullptr;
    NongListType m_currentListType = NongListType::Single;

    CCMenuItemSpriteExtra* m_downloadBtn = nullptr;
    CCMenuItemSpriteExtra* m_addBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteBtn = nullptr;
    CCMenuItemSpriteExtra* m_backBtn = nullptr;

    bool m_fetching = false;

    bool setup(std::vector<int> ids, CustomSongWidget* parent, int defaultSongID) override;
    void createList();
    SongInfo getActiveSong();
    CCSize getCellSize() const;
    void deleteAllNongs(CCObject*);
    void fetchSongFileHub(CCObject*);
    void onSettings(CCObject*);
    void openAddPopup(CCObject*);
public:
    void onSelectSong(int songID);
    void onBack(CCObject*);
    void onDiscord(CCObject*);
    int getSongID();
    void setActiveSong(SongInfo const& song);
    void deleteSong(SongInfo const& song);
    void addSong(SongInfo const& song);
    void updateParentWidget(SongInfo const& song);
    void refreshList();

    static NongDropdownLayer* create(std::vector<int> ids, CustomSongWidget* parent, int defaultSongID) {
        auto ret = new NongDropdownLayer;
        if (ret && ret->initAnchored(420.f, 280.f, ids, parent, defaultSongID, "GJ_square02.png")) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

}