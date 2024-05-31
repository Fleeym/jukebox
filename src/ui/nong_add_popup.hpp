#pragma once

#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/ui/TextInput.hpp>

#include "nong_dropdown_layer.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class NongAddPopup : public Popup<NongDropdownLayer*> {
protected:
    NongDropdownLayer* m_parentPopup;
    CCMenuItemSpriteExtra* m_selectSongButton;
    CCMenuItemSpriteExtra* m_addSongButton;
    CCMenu* m_selectSongMenu;
    CCMenu* m_addSongMenu;

    TextInput* m_songNameInput;
    TextInput* m_artistNameInput;
    TextInput* m_levelNameInput;

    fs::path m_songPath;
    CCNode* m_selectedContainer;
    CCNode* m_songPathContainer;
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

}
