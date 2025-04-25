#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/Result.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

#include <jukebox/nong/nong.hpp>
#include <jukebox/ui/nong_dropdown_layer.hpp>

namespace jukebox {

class NongDropdownLayer;

class NongAddPopup : public geode::Popup<int, std::optional<Song*>> {
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
    std::string m_memoizedYoutubeInput;
    std::string m_memoizedHostedInput;

    std::optional<std::filesystem::path> m_localPath = std::nullopt;

    cocos2d::CCNode* m_container = nullptr;
    geode::TextInput* m_songNameInput = nullptr;
    geode::TextInput* m_artistNameInput = nullptr;
    geode::TextInput* m_levelNameInput = nullptr;
    geode::TextInput* m_startOffsetInput = nullptr;

    cocos2d::CCMenu* m_switchMenu = nullptr;
    ButtonSprite* m_switchLocalSpr = nullptr;
    CCMenuItemSpriteExtra* m_switchLocalButton = nullptr;
    ButtonSprite* m_switchYTSpr = nullptr;
    CCMenuItemSpriteExtra* m_switchYTButton = nullptr;
    ButtonSprite* m_switchHostedSpr = nullptr;
    CCMenuItemSpriteExtra* m_switchHostedButton = nullptr;

    cocos2d::CCNode* m_specialInfoNode = nullptr;
    geode::TextInput* m_specialInput = nullptr;
    cocos2d::CCMenu* m_localSongMenu = nullptr;
    CCMenuItemSpriteExtra* m_localSongButton = nullptr;

    cocos2d::CCMenu* m_addSongMenu = nullptr;
    CCMenuItemSpriteExtra* m_addSongButton = nullptr;
    CCMenuItemSpriteExtra* m_publishSongButton = nullptr;

    SongType m_songType;

    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>>
        m_pickListener;

    std::optional<Song*> m_replacedNong;

    bool setup(int songID, std::optional<Song*> replacedNong) override;
    void addPathLabel(std::string const& path);
    void onFileOpen(
        geode::Task<geode::Result<std::filesystem::path>>::Event* event);
    void setSongType(SongType type, bool memorizePrevious);
    void onSwitchToLocal(cocos2d::CCObject*);
    void onSwitchToYT(cocos2d::CCObject*);
    void onSwitchToHosted(cocos2d::CCObject*);

    cocos2d::CCSize getPopupSize();
    void openFile(cocos2d::CCObject*);
    void addSong(cocos2d::CCObject*);
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
    void onPublish(cocos2d::CCObject*);
    std::optional<ParsedMetadata> tryParseMetadata(std::filesystem::path path);

public:
    static NongAddPopup* create(int songID,
                                std::optional<Song*> nong = std::nullopt);
};

}  // namespace jukebox
