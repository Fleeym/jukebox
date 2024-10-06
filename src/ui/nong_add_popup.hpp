#pragma once

#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/ui/TextInput.hpp>
#include <optional>

#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/loader/Event.hpp"
#include "Geode/utils/Result.hpp"
#include "Geode/utils/Task.hpp"
#include "nong_dropdown_layer.hpp"

using namespace geode::prelude;
namespace fs = std::filesystem;

namespace jukebox {

class NongDropdownLayer;

class NongAddPopup : public Popup<NongDropdownLayer*, int, std::optional<Nong>> {
protected:
    enum class NongAddPopupSongType {
      local,
      yt,
      hosted,
    };

    struct ParsedMetadata {
        std::optional<std::string> name;
        std::optional<std::string> artist;
    };

    NongDropdownLayer* m_parentPopup;

    int m_songID;

    std::vector<std::string> m_publishableIndexes;

    CCMenuItemSpriteExtra* m_addSongButton;
    CCMenuItemSpriteExtra* m_switchLocalButton;
    CCMenuItemSpriteExtra* m_switchYTButton;
    CCMenuItemSpriteExtra* m_switchHostedButton;
    // CCMenu* m_selectSongMenu;
    CCMenu* m_addSongMenu;
    CCMenu* m_switchLocalMenu;
    CCMenu* m_switchYTMenu;
    CCMenu* m_switchHostedMenu;
    ButtonSprite* m_switchLocalButtonSprite;
    ButtonSprite* m_switchYTButtonSprite;
    ButtonSprite* m_switchHostedButtonSprite;

    CCMenu* m_switchButtonsMenu;
    CCMenu* m_specificInputsMenu;

    CCMenu* m_localMenu;
    CCMenu* m_localSongButtonMenu;
    CCMenuItemSpriteExtra* m_localSongButton;
    TextInput* m_localLinkInput;

    TextInput* m_ytLinkInput;

    TextInput* m_hostedLinkInput;

    NongAddPopupSongType m_songType;

    TextInput* m_songNameInput;
    TextInput* m_artistNameInput;
    TextInput* m_levelNameInput;
    TextInput* m_startOffsetInput;

    EventListener<Task<Result<std::filesystem::path>>> m_pickListener;

    std::optional<Nong> m_replacedNong;

    bool setup(NongDropdownLayer* parent, int songID, std::optional<Nong> replacedNong) override;
    void createInputs();
    void addPathLabel(std::string const& path);
    void onFileOpen(Task<Result<std::filesystem::path>>::Event* event);
    void setSongType(NongAddPopupSongType type);
    void onSwitchToLocal(CCObject*);
    void onSwitchToYT(CCObject*);
    void onSwitchToHosted(CCObject*);

    CCSize getPopupSize();
    void openFile(CCObject*);
    void addSong(CCObject*);
    void onPublish(CCObject*);
    std::optional<ParsedMetadata> tryParseMetadata(std::filesystem::path path);
    void onClose(CCObject*) override;
public:
    static NongAddPopup* create(NongDropdownLayer* parent, int songID, std::optional<Nong> nong = std::nullopt);
};

}
