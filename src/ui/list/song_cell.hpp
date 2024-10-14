#pragma once

#include <functional>

#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"

#include "nong.hpp"

using namespace geode::prelude;

namespace jukebox {

class NongDropdownLayer;

class SongCell : public CCNode {
protected:
    SongMetadata* m_active;
    CCLabelBMFont* m_songNameLabel;
    CCLabelBMFont* m_authorNameLabel;
    CCLabelBMFont* m_songIDLabel;
    int m_songID;

    std::function<void()> m_callback;

    bool init(int id, SongMetadata* songInfo, const CCSize& size,
              std::function<void()> selectCallback);

public:
    static SongCell* create(int id, SongMetadata* songInfo, const CCSize& size,
                            std::function<void()> selectCallback) {
        auto ret = new SongCell();
        if (ret && ret->init(id, songInfo, size, selectCallback)) {
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
    void onSelectSong(CCObject*);
};

}  // namespace jukebox
