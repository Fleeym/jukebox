#pragma once

#include <functional>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/Label.hpp>
#include <Geode/utils/cocos.hpp>

#include <jukebox/nong/nong.hpp>

namespace jukebox {

class NongDropdownLayer;

class SongCell : public cocos2d::CCNode {
protected:
    SongMetadata* m_active = nullptr;
    geode::Ref<geode::Label> m_songNameLabel;
    geode::Ref<geode::Label> m_authorNameLabel;
    geode::Ref<geode::Label> m_songIDLabel;
    int m_songID = 0;

    std::function<void()> m_callback;

    bool init(int id, SongMetadata* songInfo, const cocos2d::CCSize& size, std::function<void()> selectCallback);

public:
    static SongCell* create(int id, SongMetadata* songInfo, const cocos2d::CCSize& size,
                            std::function<void()> selectCallback) {
        auto ret = new SongCell();
        if (ret->init(id, songInfo, size, std::move(selectCallback))) {
            return ret;
        }

        delete ret;
        return nullptr;
    }
    void onSelectSong(CCObject*);
};

}  // namespace jukebox
