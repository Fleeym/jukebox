#pragma once

#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <functional>

#include "../../types/song_info.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class SongCell : public CCNode {
protected:
    SongInfo m_active;
    CCLabelBMFont* m_songNameLabel;
    CCLabelBMFont* m_authorNameLabel;
    CCLabelBMFont* m_songIDLabel;
    int m_songID;

    std::function<void()> m_callback;

    bool init(
        int id,
        const SongInfo& songInfo,
        const CCSize& size,
        std::function<void()> selectCallback
    );
public:
    static SongCell* create(
        int id,
        const SongInfo& songInfo,
        const CCSize& size,
        std::function<void()> selectCallback
    ) {
        auto ret = new SongCell();
        if (ret && ret->init(id, songInfo, size, selectCallback)) {
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
    void onSelectSong(CCObject*);
};

}
