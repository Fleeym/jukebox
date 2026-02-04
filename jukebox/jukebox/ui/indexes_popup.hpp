#pragma once

#include <functional>
#include <vector>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

#include <jukebox/nong/index.hpp>

namespace jukebox {

using IndexesCallback = std::function<void(std::vector<index::IndexSource>)>;

class IndexesPopup : public geode::Popup {
protected:
    std::vector<index::IndexSource> m_indexes;
    IndexesCallback m_setIndexesCallback;
    geode::Ref<geode::ScrollLayer> m_list;

    bool init(std::vector<index::IndexSource>, IndexesCallback setIndexesCallback);
    void createList();
    cocos2d::CCSize getPopupSize();
    void onClose(cocos2d::CCObject*) override;
    void onAdd(cocos2d::CCObject*);

public:
    static IndexesPopup* create(std::vector<index::IndexSource>, IndexesCallback setIndexesCallback);
};

}  // namespace jukebox
