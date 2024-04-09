#pragma once

#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>

#include "list_cell.hpp"
#include "nong_dropdown_layer.hpp"
#include "../types/song_info.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class JBSongCell : public JBListCell {
protected:
    SongInfo m_active;
    CCLabelBMFont* m_songNameLabel;
    CCLabelBMFont* m_authorNameLabel;
    CCLabelBMFont* m_songIDLabel;
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

}
