#pragma once

#include <functional>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>

#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox {

// Fields are public because they are meant to be changed and then UI will
// update when build() is called.
class NongCellUI : public cocos2d::CCNode {
protected:
    cocos2d::CCLabelBMFont* m_songNameLabel = nullptr;
    cocos2d::CCLabelBMFont* m_authorNameLabel = nullptr;
    cocos2d::CCLabelBMFont* m_metadataLabel = nullptr;

    cocos2d::CCNode* m_songInfoNode = nullptr;

    cocos2d::CCMenu* m_buttonsMenu = nullptr;
    CCMenuItemSpriteExtra* m_selectButton = nullptr;
    CCMenuItemSpriteExtra* m_trashButton = nullptr;
    CCMenuItemSpriteExtra* m_fixButton = nullptr;
    CCMenuItemSpriteExtra* m_downloadButton = nullptr;
    CCMenuItemSpriteExtra* m_editButton = nullptr;

    cocos2d::CCMenu* m_downloadProgressContainer = nullptr;
    cocos2d::CCProgressTimer* m_downloadProgressTimer = nullptr;

    bool init(const cocos2d::CCSize& size, std::function<void()> onSelect, std::function<void()> onTrash,
              std::function<void()> onFixDefault, std::function<void()> onDownload, std::function<void()> onEdit);

public:
    std::string m_songName = "None";
    std::string m_authorName = "None";
    std::string m_metadata = "";

    cocos2d::CCSize m_size;

    std::function<void()> m_onSelect;
    std::function<void()> m_onTrash;
    std::function<void()> m_onFixDefault;
    std::function<void()> m_onDownload;
    std::function<void()> m_onEdit;

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

    static NongCellUI* create(const cocos2d::CCSize& size, std::function<void()> onSelect,
                              std::function<void()> onTrash, std::function<void()> onFixDefault,
                              std::function<void()> onDownload, std::function<void()> onEdit);

    void build();
    void buildOnlyDownloadProgress();

    void onSelect(CCObject*);
    void onTrash(CCObject*);
    void onFixDefault(CCObject*);
    void onDownload(CCObject*);
    void onEdit(CCObject*);
};

}  // namespace jukebox
