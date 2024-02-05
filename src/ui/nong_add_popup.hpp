#pragma once

#include <Geode/cocos/label_nodes/CCLabelBMFont.h>

#include "nong_dropdown_layer.hpp"

using namespace geode::prelude;

class NongDropdownLayer;

class NongAddPopup : public Popup<NongDropdownLayer*> {
protected:
    NongDropdownLayer* m_parentPopup;
    CCMenuItemSpriteExtra* m_selectSongButton;
    CCMenuItemSpriteExtra* m_addSongButton;
    CCMenu* m_selectSongMenu;
    CCMenu* m_addSongMenu;
    CCLayer* m_containerLayer;

    CCTextInputNode* m_songNameInput;
    CCTextInputNode* m_artistNameInput;

    fs::path m_songPath;
    CCLabelBMFont* m_songPathLabel;

    bool setup(NongDropdownLayer* parent) override;
    void createInputs();
    void addPathLabel(std::string const& path);

    CCSize getPopupSize();
    void openFile(CCObject*);
    void addSong(CCObject*);
public:
    static NongAddPopup* create(NongDropdownLayer* parent);
};
