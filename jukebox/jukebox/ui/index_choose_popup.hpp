#pragma once

#include <Geode/utils/function.hpp>
#include <string>
#include <vector>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/Label.hpp>
#include <Geode/ui/Popup.hpp>

namespace jukebox {

class IndexChoosePopup : public geode::Popup {
protected:
    std::vector<std::string> m_indexIDs;
    geode::Function<void(const std::string& indexID)> m_chooseIndex;
    geode::Label* m_label = nullptr;
    int m_currentIndex = 0;

    bool init(std::vector<std::string> indexIDs, geode::Function<void(const std::string& indexID)> chooseIndex);
    void updateLabel();
    void onRight(CCObject*);
    void onLeft(CCObject*);
    void onOK(CCObject*);

public:
    static IndexChoosePopup* create(std::vector<std::string> indexIDs,
                                    geode::Function<void(const std::string& indexID)> chooseIndex);
};

}  // namespace jukebox
