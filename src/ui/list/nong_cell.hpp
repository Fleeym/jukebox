#pragma once

#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <functional>

#include "../../types/song_info.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class NongCell : public CCNode {
protected:
    int m_songID;
    SongInfo m_songInfo;
    CCLabelBMFont* m_songNameLabel = nullptr;
    CCLabelBMFont* m_authorNameLabel = nullptr;
    CCLabelBMFont* m_levelNameLabel = nullptr;
    CCLayer* m_songInfoLayer;

    std::function<void()> m_onSelect;
    std::function<void()> m_onFixDefault;
    std::function<void()> m_onDelete;

    bool m_isDefault;
    bool m_isActive;

    bool init(
        int songID,
        SongInfo info,
        bool isDefault,
        bool selected, 
        CCSize const& size,
        std::function<void()> onSelect,
        std::function<void()> onFixDefault,
        std::function<void()> onDelete
    );
public:
    static NongCell* create(
        int songID,
        SongInfo info,
        bool isDefault,
        bool selected, 
        CCSize const& size,
        std::function<void()> onSelect,
        std::function<void()> onFixDefault,
        std::function<void()> onDelete
    );
    void onSet(CCObject*);
    void deleteSong(CCObject*);
    void onFixDefault(CCObject*);
};

}
