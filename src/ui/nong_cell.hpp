#pragma once

#include <Geode/Geode.hpp>

#include "../types/song_info.hpp"
#include "list_cell.hpp"
#include "nong_dropdown_layer.hpp"

using namespace geode::prelude;

class NongDropdownLayer;

class NongCell : public JBListCell {
protected:
    SongInfo m_songInfo;
    CCLabelBMFont* m_songNameLabel;
    CCLabelBMFont* m_authorNameLabel;
    CCLabelBMFont* m_levelNameLabel = nullptr;
    CCLayer* m_songInfoLayer;

    NongDropdownLayer* m_parentPopup;

    bool init(SongInfo info, NongDropdownLayer* parentPopup, CCSize const& size, bool selected, bool isDefault);

    virtual void FLAlert_Clicked(FLAlertLayer*, bool btn2);
public:
    static NongCell* create(SongInfo info, NongDropdownLayer* parentPopup, CCSize const& size, bool selected, bool isDefault);
    void onSet(CCObject*);
    void deleteSong(CCObject*);
};