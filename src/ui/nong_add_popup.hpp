#pragma once

#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/ui/TextInput.hpp>
#include <optional>

#include "Geode/loader/Event.hpp"
#include "Geode/utils/Result.hpp"
#include "Geode/utils/Task.hpp"
#include "nong_dropdown_layer.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class NongAddPopup : public Popup<NongDropdownLayer*> {
protected:
    struct ParsedMetadata {
        std::optional<std::string> name;
        std::optional<std::string> artist;
    };

    NongDropdownLayer* m_parentPopup;
    CCMenuItemSpriteExtra* m_selectSongButton;
    CCMenuItemSpriteExtra* m_addSongButton;
    CCMenu* m_selectSongMenu;
    CCMenu* m_addSongMenu;

    TextInput* m_songNameInput;
    TextInput* m_artistNameInput;
    TextInput* m_levelNameInput;
    TextInput* m_startOffsetInput;

    fs::path m_songPath;
    CCNode* m_selectedContainer;
    CCNode* m_songPathContainer;
    CCLabelBMFont* m_songPathLabel;
    EventListener<Task<Result<std::filesystem::path>>> m_pickListener;

    bool setup(NongDropdownLayer* parent) override;
    void createInputs();
    void addPathLabel(std::string const& path);
    void onFileOpen(Task<Result<std::filesystem::path>>::Event* event);

    CCSize getPopupSize();
    void openFile(CCObject*);
    void addSong(CCObject*);
    std::optional<ParsedMetadata> tryParseMetadata(std::filesystem::path path);
public:
    static NongAddPopup* create(NongDropdownLayer* parent);
};

}
