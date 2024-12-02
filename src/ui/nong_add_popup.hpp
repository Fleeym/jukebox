#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Geode/binding/ButtonSprite.hpp"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/loader/Event.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/Result.hpp"
#include "Geode/utils/Task.hpp"

#include "nong.hpp"
#include "ui/nong_dropdown_layer.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class NongAddPopup : public Popup<int, std::optional<Song*>> {
protected:
    enum class SongType {
        LOCAL,
        YOUTUBE,
        HOSTED,
    };

    struct ParsedMetadata {
        std::optional<std::string> name;
        std::optional<std::string> artist;
    };

    int m_songID;

    std::vector<std::string> m_publishableIndexes;

    std::string m_memoizedLocalInput;
    std::string m_memoizedHostedInput;

    std::optional<std::filesystem::path> m_localPath = std::nullopt;

    CCNode* m_container = nullptr;
    TextInput* m_songNameInput = nullptr;
    TextInput* m_artistNameInput = nullptr;
    TextInput* m_levelNameInput = nullptr;
    TextInput* m_startOffsetInput = nullptr;

    CCMenu* m_switchMenu = nullptr;
    ButtonSprite* m_switchLocalSpr = nullptr;
    CCMenuItemSpriteExtra* m_switchLocalButton = nullptr;
    ButtonSprite* m_switchYTSpr = nullptr;
    CCMenuItemSpriteExtra* m_switchYTButton = nullptr;
    ButtonSprite* m_switchHostedSpr = nullptr;
    CCMenuItemSpriteExtra* m_switchHostedButton = nullptr;

    CCNode* m_specialInfoNode = nullptr;
    TextInput* m_specialInput = nullptr;
    CCMenu* m_localSongMenu = nullptr;
    CCMenuItemSpriteExtra* m_localSongButton = nullptr;

    CCMenu* m_addSongMenu = nullptr;
    CCMenuItemSpriteExtra* m_addSongButton = nullptr;
    CCMenuItemSpriteExtra* m_publishSongButton = nullptr;

    SongType m_songType;

    EventListener<Task<Result<std::filesystem::path>>> m_pickListener;

    std::optional<Song*> m_replacedNong;

    bool setup(int songID, std::optional<Song*> replacedNong) override;
    void addPathLabel(std::string const& path);
    void onFileOpen(Task<Result<std::filesystem::path>>::Event* event);
    void setSongType(SongType type);
    void onSwitchToLocal(CCObject*);
    void onSwitchToYT(CCObject*);
    void onSwitchToHosted(CCObject*);

    CCSize getPopupSize();
    void openFile(CCObject*);
    void addSong(CCObject*);
    geode::Result<> addLocalSong(const std::string& songName,
                                 const std::string& artistName,
                                 const std::optional<std::string> levelName,
                                 int offset);
    geode::Result<> addYTSong(const std::string& songName,
                              const std::string& artistName,
                              const std::optional<std::string> levelName,
                              int offset);
    geode::Result<> addHostedSong(const std::string& songName,
                                  const std::string& artistName,
                                  const std::optional<std::string> levelName,
                                  int offset);
    void onPublish(CCObject*);
    std::optional<ParsedMetadata> tryParseMetadata(std::filesystem::path path);

public:
    static NongAddPopup* create(int songID,
                                std::optional<Song*> nong = std::nullopt);
};

}  // namespace jukebox
