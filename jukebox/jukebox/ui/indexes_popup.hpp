#pragma once

#include <functional>
#include <vector>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

#include <jukebox/nong/index.hpp>

using namespace geode::prelude;

namespace jukebox {

using namespace jukebox::index;

using IndexesCallback = std::function<void(std::vector<IndexSource>)>;

class IndexesPopup : public Popup<std::vector<IndexSource>, IndexesCallback> {
protected:
    std::vector<IndexSource> m_indexes;
    IndexesCallback m_setIndexesCallback;
    geode::ScrollLayer* m_list;

    bool setup(std::vector<IndexSource>,
               IndexesCallback setIndexesCallback) override;
    void createList();
    CCSize getPopupSize();
    void onClose(CCObject*) override;
    void onAdd(CCObject*);

public:
    static IndexesPopup* create(std::vector<IndexSource>,
                                IndexesCallback setIndexesCallback);
};

}  // namespace jukebox
