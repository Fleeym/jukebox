#pragma once

#include <functional>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/binding/CCMenuItemToggler.hpp>

#include <jukebox/nong/index.hpp>

namespace jukebox {

using namespace jukebox::index;

class IndexesPopup;

class IndexCell : public cocos2d::CCNode {
protected:
    IndexesPopup* m_parentPopup;
    IndexSource* m_index;
    std::function<void()> m_onDelete;

    CCMenuItemToggler* m_toggleButton;

    bool init(IndexesPopup* parentPopup, IndexSource* index,
              std::function<void()> onDelete, cocos2d::CCSize const& size);

public:
    static IndexCell* create(IndexesPopup* parentPopup, IndexSource* index,
                             std::function<void()> onDelete,
                             cocos2d::CCSize const& size);
    void updateUI();
    void onToggle(CCObject*);
    void onDelete(CCObject*);
};

}  // namespace jukebox
