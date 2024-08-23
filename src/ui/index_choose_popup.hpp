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

class IndexChoosePopup : public Popup<std::vector<std::string>, std::function<void(const std::string& indexID)>> {
protected:
    std::vector<std::string> m_indexIDs;
    std::function<void(const std::string& indexID)> m_chooseIndex;
    CCLabelBMFont* m_label = nullptr;
    int m_currentIndex = 0;

    bool setup(std::vector<std::string> indexIDs, std::function<void(const std::string& indexID)> chooseIndex) override;
    void updateLabel();
    void onRight(CCObject*);
    void onLeft(CCObject*);
    void onOK(CCObject*);
public:
    static IndexChoosePopup* create(std::vector<std::string> indexIDs, std::function<void(const std::string& indexID)> chooseIndex);
};

}
