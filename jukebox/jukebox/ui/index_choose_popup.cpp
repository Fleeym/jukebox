#include <jukebox/ui/index_choose_popup.hpp>

#include <optional>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/ui/Layout.hpp>

#include <jukebox/managers/index_manager.hpp>

using namespace geode::prelude;

namespace jukebox {

bool IndexChoosePopup::init(std::vector<std::string> indexIDs,
                            std::function<void(const std::string& indexID)> chooseIndex) {
    if (!Popup::init(280.0f, 80.0f)) {
        return false;
    }
    m_indexIDs = std::move(indexIDs);
    m_chooseIndex = std::move(chooseIndex);

    auto menu = CCMenu::create();
    menu->setContentWidth(m_mainLayer->getContentWidth() - 10.f);
    menu->setContentHeight(m_mainLayer->getContentHeight() - 20.f);

    auto switchMenu = CCMenu::create();
    switchMenu->setLayout(RowLayout::create());
    switchMenu->setContentSize({menu->getContentSize().width, 40.f});

    auto spriteLeft = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    spriteLeft->setScale(1.5);
    auto btnLeft = CCMenuItemSpriteExtra::create(spriteLeft, this, menu_selector(IndexChoosePopup::onLeft));

    auto spriteRight = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    spriteRight->setScale(1.5);
    auto btnRight = CCMenuItemSpriteExtra::create(spriteRight, this, menu_selector(IndexChoosePopup::onRight));

    auto label = CCLabelBMFont::create("", "bigFont.fnt");
    m_label = label;
    label->limitLabelWidth(switchMenu->getContentWidth() - 10.f, 0.8f, 0.1f);
    label->setID("index-name");
    auto labelMenu = CCMenu::create();
    labelMenu->addChild(label);
    labelMenu->setContentWidth(switchMenu->getContentWidth());
    labelMenu->addChildAtPosition(label, Anchor::Center);

    switchMenu->addChild(btnLeft);
    switchMenu->addChild(labelMenu);
    switchMenu->addChild(btnRight);
    switchMenu->updateLayout();

    auto addSongButton =
        CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, menu_selector(IndexChoosePopup::onOK));
    auto addSongMenu = CCMenu::create();
    addSongMenu->setID("add-song-menu");
    addSongButton->setID("add-song-button");
    addSongMenu->setAnchorPoint({0.5f, 0.5f});
    addSongMenu->setContentSize(addSongButton->getContentSize());
    addSongMenu->addChildAtPosition(addSongButton, Anchor::Center);

    menu->addChildAtPosition(switchMenu, Anchor::Center);

    m_mainLayer->addChildAtPosition(menu, Anchor::Center);
    m_mainLayer->addChildAtPosition(addSongMenu, Anchor::Bottom, {0, 3});

    label->setPositionY(label->getPositionY() + 1.f);

    // // auto indexes =
    // m_

    // m_mainLayer->addChild(addSongButton);
    // auto menu = CCMenu::create();
    // menu->addChild(addBtn);
    // menu->setZOrder(1);
    // m_mainLayer->addChildAtPosition(menu, Anchor::BottomRight, { -5.f -
    // addBtn->getContentWidth()/2.f, 8.f + addBtn->getContentHeight()/2.f });

    updateLabel();

    return true;
}

void IndexChoosePopup::updateLabel() {
    m_label->setString(IndexManager::get().getIndexName(m_indexIDs.at(m_currentIndex))->c_str());
}

void IndexChoosePopup::onLeft(CCObject*) {
    m_currentIndex = (m_currentIndex - 1 + m_indexIDs.size()) % m_indexIDs.size();
    updateLabel();
}

void IndexChoosePopup::onRight(CCObject*) {
    m_currentIndex = (m_currentIndex + 1) % m_indexIDs.size();
    updateLabel();
}

void IndexChoosePopup::onOK(CCObject*) {
    m_chooseIndex(m_indexIDs.at(m_currentIndex));
    m_closeBtn->activate();
}

IndexChoosePopup* IndexChoosePopup::create(std::vector<std::string> indexIDs,
                                           std::function<void(const std::string&)> setIndexesCallback) {
    auto ret = new IndexChoosePopup();
    if (ret->init(std::move(indexIDs), std::move(setIndexesCallback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

}  // namespace jukebox
