#pragma once

#include <functional>

#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCObject.h"

#include "nong.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class NongCell : public CCNode {
protected:
    int m_songID;
    CCLabelBMFont* m_songNameLabel = nullptr;
    CCLabelBMFont* m_authorNameLabel = nullptr;
    CCLabelBMFont* m_metadataLabel = nullptr;
    CCLayer* m_songInfoLayer;

    std::function<void()> m_onSelect;
    std::function<void()> m_onFixDefault;
    std::function<void()> m_onDelete;
    std::function<void()> m_onDownload;
    std::function<void()> m_onEdit;

    bool m_isDefault;
    bool m_isActive;
    bool m_isDownloaded;
    bool m_isDownloadable;

    CCMenuItemSpriteExtra* m_downloadButton;
    CCMenu* m_downloadProgressContainer;
    CCProgressTimer* m_downloadProgress;

    bool init(int songID, Song*, bool isDefault, bool selected,
              CCSize const& size, std::function<void()> onSelect,
              std::function<void()> onFixDefault,
              std::function<void()> onDelete, std::function<void()> onDownload,
              std::function<void()> onEdit);

public:
    Song* m_songInfo;
    static NongCell* create(int songID, Song* song, bool isDefault,
                            bool selected, CCSize const& size,
                            std::function<void()> onSelect,
                            std::function<void()> onFixDefault,
                            std::function<void()> onDelete,
                            std::function<void()> onDownload,
                            std::function<void()> onEdit);

    void onSet(CCObject*);
    void onDelete(CCObject*);
    void onFixDefault(CCObject*);
    void onDownload(CCObject*);
    void onEdit(CCObject*);
    void setDownloadProgress(float progress);
};

}  // namespace jukebox
