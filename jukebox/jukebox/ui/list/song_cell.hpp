#pragma once

#include <functional>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/platform/CCPlatformMacros.h>

#include <jukebox/nong/nong.hpp>

namespace jukebox {

class NongDropdownLayer;

class SongCell : public cocos2d::CCNode {
protected:
    SongMetadata* m_active;
    cocos2d::CCLabelBMFont* m_songNameLabel;
    cocos2d::CCLabelBMFont* m_authorNameLabel;
    cocos2d::CCLabelBMFont* m_songIDLabel;
    int m_songID;

    std::function<void()> m_callback;

    bool init(int id, SongMetadata* songInfo, const cocos2d::CCSize& size,
              std::function<void()> selectCallback);

public:
    static SongCell* create(int id, SongMetadata* songInfo,
                            const cocos2d::CCSize& size,
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
