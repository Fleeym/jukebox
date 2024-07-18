#pragma once

#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/ui/TextInput.hpp>
#include <optional>

#include "Geode/loader/Event.hpp"
#include "Geode/utils/Result.hpp"
#include "Geode/utils/Task.hpp"
#include "nong_dropdown_layer.hpp"
#include "../../include/index.hpp"

using namespace geode::prelude;

namespace jukebox {

class IndexesPopup : public Popup<std::vector<IndexSource>, std::function<void(std::vector<IndexSource>)>> {
protected:
    std::vector<IndexSource> m_indexes;
    std::function<void(std::vector<IndexSource>)> m_setIndexesCallback;
    geode::ScrollLayer* m_list;

    bool setup(std::vector<IndexSource>, std::function<void(std::vector<IndexSource>)> setIndexesCallback) override;
    void createList();
    CCSize getPopupSize();
    void onClose(CCObject*) override;
    void onAdd(CCObject*);
public:
    static IndexesPopup* create(std::vector<IndexSource>, std::function<void(std::vector<IndexSource>)> setIndexesCallback);
};

}
