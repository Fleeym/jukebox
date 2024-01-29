#pragma once

#include <Geode/Geode.hpp>

#include "../types/song_info.hpp"
#include "list_cell.hpp"
#include "nong_dropdown_layer.hpp"

using namespace geode::prelude;

class NongDropdownLayer;

class JBSongCell : public JBListCell {
protected:
    SongInfo m_active;
    CCLabelBMFont* m_songNameLabel;
    CCLabelBMFont* m_authorNameLabel;
    int m_songID;

    NongDropdownLayer* m_parentPopup;

    bool init(NongData data, int id, NongDropdownLayer* parentPopup, CCSize const& size);
public:
    static JBSongCell* create(NongData data, int id, NongDropdownLayer* parentPopup, CCSize const& size) {
        auto ret = new JBSongCell();
        if (ret && ret->init(data, id, parentPopup, size)) {
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
    void onSelectSong(CCObject*);
};