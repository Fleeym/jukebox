#pragma once

#include <Geode/utils/function.hpp>

#include <Geode/ui/Label.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>

#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox {

// Fields are public because they are meant to be changed and then UI will
// update when build() is called.
class NongCellUI : public cocos2d::CCNode {
protected:
    geode::Label* m_songNameLabel = nullptr;
    geode::Label* m_authorNameLabel = nullptr;
    geode::Label* m_metadataLabel = nullptr;

    cocos2d::CCNode* m_songInfoNode = nullptr;

    cocos2d::CCMenu* m_buttonsMenu = nullptr;
    CCMenuItemSpriteExtra* m_selectButton = nullptr;
    CCMenuItemSpriteExtra* m_trashButton = nullptr;
    CCMenuItemSpriteExtra* m_fixButton = nullptr;
    CCMenuItemSpriteExtra* m_downloadButton = nullptr;
    CCMenuItemSpriteExtra* m_editButton = nullptr;

    cocos2d::CCMenu* m_downloadProgressContainer = nullptr;
    cocos2d::CCProgressTimer* m_downloadProgressTimer = nullptr;

    bool init(const cocos2d::CCSize& size, geode::Function<void()> onSelect, geode::Function<void()> onTrash,
              geode::Function<void()> onFixDefault, geode::Function<void()> onDownload, geode::Function<void()> onEdit);

public:
    std::string m_songName = "None";
    std::string m_authorName = "None";
    std::string m_metadata = "";

    cocos2d::CCSize m_size;

    geode::Function<void()> m_onSelect;
    geode::Function<void()> m_onTrash;
    geode::Function<void()> m_onFixDefault;
    geode::Function<void()> m_onDownload;
    geode::Function<void()> m_onEdit;

    bool m_showSelectButton = false;
    bool m_showTrashButton = false;
    bool m_showFixDefaultButton = false;
    bool m_showDownloadButton = false;
    bool m_showEditButton = false;
    bool m_isVerified = false;
    bool m_isDownloaded = false;
    bool m_isSelected = false;

    bool m_isDownloading = 0;
    float m_downloadProgress = 0;

    static NongCellUI* create(const cocos2d::CCSize& size, geode::Function<void()> onSelect,
                              geode::Function<void()> onTrash, geode::Function<void()> onFixDefault,
                              geode::Function<void()> onDownload, geode::Function<void()> onEdit);

    void build();
    void buildOnlyDownloadProgress();

    void onSelect(CCObject*);
    void onTrash(CCObject*);
    void onFixDefault(CCObject*);
    void onDownload(CCObject*);
    void onEdit(CCObject*);
};

}  // namespace jukebox
