#pragma once

#include <Geode/cocos/cocoa/CCObject.h>

#include "list_cell.hpp"
#include "nong_dropdown_layer.hpp"
#include "../types/song_info.hpp"

using namespace geode::prelude;

class NongDropdownLayer;

class NongCell : public JBListCell {
protected:
    Nong m_songInfo;
    CCLabelBMFont* m_songNameLabel;
    CCLabelBMFont* m_authorNameLabel;
    CCLabelBMFont* m_levelNameLabel = nullptr;
    CCLayer* m_songInfoLayer;

    NongDropdownLayer* m_parentPopup;

    bool m_isDefault;
    bool m_isActive;

    bool init(Nong info, NongDropdownLayer* parentPopup, CCSize const& size, bool selected, bool isDefault);

    virtual void FLAlert_Clicked(FLAlertLayer*, bool btn2);
public:
    static NongCell* create(Nong info, NongDropdownLayer* parentPopup, CCSize const& size, bool selected, bool isDefault);
    void onSet(CCObject*);
    void deleteSong(CCObject*);
    void onFixDefault(CCObject*);
};