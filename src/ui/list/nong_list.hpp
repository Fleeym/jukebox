#pragma once

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/ScrollLayer.hpp>
#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <optional>

#include "../../../include/nong.hpp"

namespace jukebox {


class NongList : public cocos2d::CCNode {
public:
    enum class ListType {
        Single = 0,
        Multiple = 1
    };
protected:
    std::vector<int> m_songIds;
    geode::ScrollLayer* m_list;
    cocos2d::extension::CCScale9Sprite* m_bg;
    std::optional<int> m_currentSong = std::nullopt;

    CCMenuItemSpriteExtra* m_backBtn = nullptr;

    std::function<void(int, const SongMetadataPathed&)> m_onSetActive;
    std::function<void(int)> m_onFixDefault;
    std::function<void(int, const SongMetadataPathed&)> m_onDelete;
    std::function<void(bool)> m_onListTypeChange;

    static constexpr float s_padding = 10.0f;
    static constexpr float s_itemSize = 60.f;
public:
    void scrollToTop();
    void setCurrentSong(int songId);
    void build();
    void onBack(cocos2d::CCObject*);
    void onSelectSong(int songId);

    static NongList* create(
        std::vector<int>& songIds,
        const cocos2d::CCSize& size,
        std::function<void(int, const SongMetadataPathed&)> onSetActive,
        std::function<void(int)> onFixDefault,
        std::function<void(int, const SongMetadataPathed&)> onDelete,
        std::function<void(bool)> onListTypeChange = {}
    );
protected:
    bool init(
        std::vector<int>& songIds,
        const cocos2d::CCSize& size,
        std::function<void(int, const SongMetadataPathed&)> onSetActive,
        std::function<void(int)> onFixDefault,
        std::function<void(int, const SongMetadataPathed&)> onDelete,
        std::function<void(bool)> onListTypeChange = {}
    );
};

}
