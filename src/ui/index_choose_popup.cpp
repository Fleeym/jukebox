#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/cocos/CCDirector.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/MiniFunction.hpp>
#include <Geode/utils/Result.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/ui/TextInput.hpp>
#include <ccTypes.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>

#include <filesystem>
#include <fmt/core.h>
#include <fmod.hpp>
#include <fmod_common.h>
#include <optional>
#include <sstream>
#include <system_error>

#include "index_choose_popup.hpp"
#include "list/index_cell.hpp"
#include "../managers/index_manager.hpp"

namespace jukebox {

bool IndexChoosePopup::setup(
    std::vector<std::string> indexIDs,
    std::function<void(const std::string& indexID)> chooseIndex
) {
    m_indexIDs = indexIDs;
    m_chooseIndex = chooseIndex;

    auto menu = CCMenu::create();
    menu->setContentWidth(m_mainLayer->getContentWidth() - 10.f);
    menu->setContentHeight(m_mainLayer->getContentHeight() - 20.f);

    auto switchMenu = CCMenu::create();
    switchMenu->setLayout(RowLayout::create());
    switchMenu->setContentSize({menu->getContentSize().width, 40.f});

    auto spriteLeft = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    spriteLeft->setScale(1.5);
    auto btnLeft = CCMenuItemSpriteExtra::create(
        spriteLeft,
        this,
        menu_selector(IndexChoosePopup::onLeft)
    );

    auto spriteRight = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    spriteRight->setScale(1.5);
    auto btnRight = CCMenuItemSpriteExtra::create(
        spriteRight,
        this,
        menu_selector(IndexChoosePopup::onRight)
    );

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

    auto addSongButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("OK"),
        this,
        menu_selector(IndexChoosePopup::onOK)
    );
    auto addSongMenu = CCMenu::create();
    addSongMenu->setID("add-song-menu");
    addSongButton->setID("add-song-button");
    addSongMenu->setAnchorPoint({0.5f, 0.5f});
    addSongMenu->setContentSize(addSongButton->getContentSize());
    addSongMenu->addChildAtPosition(addSongButton, Anchor::Center);

    menu->addChildAtPosition(switchMenu, Anchor::Center);

    m_mainLayer->addChildAtPosition(menu, Anchor::Center);
    m_mainLayer->addChildAtPosition(addSongMenu, Anchor::Bottom, { 0, 3 });

    label->setPositionY(label->getPositionY() + 1.f);


    // // auto indexes =
    // m_

    // m_mainLayer->addChild(addSongButton);
    // auto menu = CCMenu::create();
    // menu->addChild(addBtn);
    // menu->setZOrder(1);
    // m_mainLayer->addChildAtPosition(menu, Anchor::BottomRight, { -5.f - addBtn->getContentWidth()/2.f, 8.f + addBtn->getContentHeight()/2.f });

    updateLabel();

    return true;
}

void IndexChoosePopup::updateLabel() {
    m_label->setString(IndexManager::get()->getIndexName(m_indexIDs.at(m_currentIndex))->c_str());
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

IndexChoosePopup* IndexChoosePopup::create(
    std::vector<std::string> indexIDs,
    std::function<void(const std::string&)> setIndexesCallback
) {
    auto ret = new IndexChoosePopup();
    if (ret && ret->initAnchored(280, 80, indexIDs, setIndexesCallback)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

}
