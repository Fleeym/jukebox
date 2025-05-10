#include <jukebox/ui/list/index_cell.hpp>

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <fmt/format.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/SimpleAxisLayout.hpp>
#include <Geode/ui/TextInput.hpp>

#include <jukebox/nong/index.hpp>

using namespace geode::prelude;

namespace jukebox {

bool IndexCell::init(IndexesPopup* parentPopup, IndexSource* index,
                     std::function<void()> onDelete, CCSize const& size) {
    if (!CCNode::init()) {
        return false;
    }

    static const float HORIZONTAL_PADDING = 2.5f;

    m_parentPopup = parentPopup;
    m_index = index;
    m_onDelete = onDelete;

    this->setContentSize(size);
    this->setAnchorPoint(CCPoint{0.5f, 0.5f});

    auto bg = CCScale9Sprite::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(75);
    bg->setScale(0.3f);
    bg->setContentSize(size / bg->getScale());
    this->addChildAtPosition(bg, Anchor::Center);

    // m_enableButton = CCMenuItemSpriteExtra::create(
    //     CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
    //     this,
    //     nullptr
    // );
    //
    // m_enableButton->setAnchorPoint(ccp(0.5f, 0.5f));
    // m_enableButton->setID("set-button");

    float m_buttonsSize = 0.f;

    m_toggleButton = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(IndexCell::onToggle), 1.0f);
    m_toggleButton->setScale(0.7f);
    m_toggleButton->setAnchorPoint(ccp(0.5f, 0.5f));

    m_buttonsSize += m_toggleButton->getContentSize().width;

    auto buttonsMenu = CCMenu::create();
    buttonsMenu->addChild(m_toggleButton);
    buttonsMenu->setAnchorPoint(CCPoint{1.0f, 0.5f});
    buttonsMenu->setContentSize(CCSize{50.f, 30.f});

    if (m_index->m_userAdded) {
        auto sprite =
            CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
        auto deleteButton = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(IndexCell::onDelete));
        deleteButton->setID("delete-button");
        deleteButton->setScale(0.7f);
        buttonsMenu->addChild(deleteButton);
        m_buttonsSize += deleteButton->getScaledContentWidth();
    }

    buttonsMenu->setLayout(SimpleRowLayout::create()
                               ->setGap(5.0f)
                               ->setMainAxisAlignment(MainAxisAlignment::Even)
                               ->setMainAxisScaling(AxisScaling::ScaleDown)
                               ->setCrossAxisScaling(AxisScaling::ScaleDown));
    buttonsMenu->setID("button-menu");

    auto inputNode = TextInput::create(
        size.width - HORIZONTAL_PADDING * 2 - buttonsMenu->getContentWidth(),
        "Index url", "chatFont.fnt");
    inputNode->setScale(1.f);
    inputNode->setCommonFilter(CommonFilter::Any);
    inputNode->setMaxCharCount(300);
    inputNode->setString(m_index->m_url, false);
    inputNode->setTextAlign(TextInputAlign::Left);
    inputNode->setCallback(
        [this](std::string const& str) { m_index->m_url = str; });

    auto menu = CCMenu::create();
    menu->addChild(inputNode);
    menu->addChild(buttonsMenu);

    menu->setLayout(SimpleRowLayout::create()
                        ->setGap(5.0f)
                        ->setMainAxisAlignment(MainAxisAlignment::Between)
                        ->setMainAxisScaling(AxisScaling::Scale));
    menu->setID("menu");
    menu->setAnchorPoint(CCPoint{0.5f, 0.5f});
    menu->setContentSize(
        CCSize{size.width - HORIZONTAL_PADDING * 2, size.height});
    menu->updateLayout();

    this->addChildAtPosition(menu, Anchor::Center, CCPoint{0.0f, 0.0f});
    this->updateUI();
    return true;
}

void IndexCell::updateUI() { m_toggleButton->toggle(m_index->m_enabled); }

void IndexCell::onToggle(CCObject*) {
    m_index->m_enabled = !m_index->m_enabled;
    this->updateUI();
    // Cancel the toggling of the button by cocos that happens after the
    // callback.
    m_toggleButton->toggle(!m_toggleButton->isToggled());
}

void IndexCell::onDelete(CCObject*) { m_onDelete(); }

IndexCell* IndexCell::create(IndexesPopup* parentPopup, IndexSource* index,
                             std::function<void()> onDelete,
                             CCSize const& size) {
    auto ret = new IndexCell();
    if (ret && ret->init(parentPopup, index, onDelete, size)) {
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

}  // namespace jukebox
