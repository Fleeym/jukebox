#pragma once

#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/ui/Popup.hpp"

using namespace geode::prelude;

namespace jukebox {

class IndexChoosePopup
    : public Popup<std::vector<std::string>,
                   std::function<void(const std::string& indexID)>> {
protected:
    std::vector<std::string> m_indexIDs;
    std::function<void(const std::string& indexID)> m_chooseIndex;
    CCLabelBMFont* m_label = nullptr;
    int m_currentIndex = 0;

    bool setup(
        std::vector<std::string> indexIDs,
        std::function<void(const std::string& indexID)> chooseIndex) override;
    void updateLabel();
    void onRight(CCObject*);
    void onLeft(CCObject*);
    void onOK(CCObject*);

public:
    static IndexChoosePopup* create(
        std::vector<std::string> indexIDs,
        std::function<void(const std::string& indexID)> chooseIndex);
};

}  // namespace jukebox
