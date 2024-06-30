#pragma once

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/ui/ListView.hpp>

#include "../../../include/nong.hpp"
#include "list/nong_list.hpp"
#include "nong_add_popup.hpp"
#include "list/nong_cell.hpp"
#include "list/song_cell.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer : public Popup<std::vector<int>, CustomSongWidget*, int> {
protected:
    std::unordered_map<int, Nongs> m_data;
    std::vector<int> m_songIDS;
    std::optional<int> m_currentSongID = std::nullopt;
    int m_defaultSongID;
    Ref<CustomSongWidget> m_parentWidget;
    NongList* m_list = nullptr;

    CCMenuItemSpriteExtra* m_downloadBtn = nullptr;
    CCMenuItemSpriteExtra* m_addBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteBtn = nullptr;

    bool m_fetching = false;

    bool setup(std::vector<int> ids, CustomSongWidget* parent, int defaultSongID) override;
    void createList();
    CCSize getCellSize() const;
    void deleteAllNongs(CCObject*);
    void fetchSongFileHub(CCObject*);
    void onSettings(CCObject*);
    void openAddPopup(CCObject*);
public:
    void onSelectSong(int songID);
    void onDiscord(CCObject*);
    void setActiveSong(SongInfo const& song);
    void deleteSong(SongInfo const& song);
    void addSong(SongInfo const& song);
    void updateParentWidget(SongInfo const& song);
    void refreshList();

    static NongDropdownLayer* create(std::vector<int> ids, CustomSongWidget* parent, int defaultSongID) {
        auto ret = new NongDropdownLayer;
        if (ret && ret->initAnchored(420.f, 280.f, ids, parent, defaultSongID, "GJ_square01.png")) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

}