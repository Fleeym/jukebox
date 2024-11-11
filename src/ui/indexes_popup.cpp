#include "indexes_popup.hpp"

#include <optional>

#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/Popup.hpp"

#include "index.hpp"
#include "ui/list/index_cell.hpp"

namespace jukebox {

using namespace jukebox::index;

bool IndexesPopup::setup(
    std::vector<IndexSource> indexes,
    std::function<void(std::vector<IndexSource>)> setIndexesCallback) {
    m_indexes = indexes;
    m_setIndexesCallback = setIndexesCallback;

    this->createList();

    auto spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    spr->setScale(0.7f);
    auto addBtn = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(IndexesPopup::onAdd));
    addBtn->setAnchorPoint({0.5f, 0.5f});
    auto menu = CCMenu::create();
    menu->addChild(addBtn);
    menu->setZOrder(1);
    m_mainLayer->addChildAtPosition(menu, Anchor::BottomRight,
                                    {-5.f - addBtn->getContentWidth() / 2.f,
                                     8.f + addBtn->getContentHeight() / 2.f});

    return true;
}

void IndexesPopup::onClose(CCObject* sender) {
    m_setIndexesCallback(m_indexes);
    Popup::onClose(sender);
}

void IndexesPopup::onAdd(CCObject*) {
    m_indexes.push_back({
        .m_url = "",
        .m_userAdded = true,
        .m_enabled = true,
    });
    this->createList();
}

IndexesPopup* IndexesPopup::create(
    std::vector<IndexSource> indexes,
    std::function<void(std::vector<IndexSource>)> setIndexesCallback) {
    auto ret = new IndexesPopup();
    auto size = ret->getPopupSize();
    if (ret && ret->initAnchored(size.width, size.height, indexes,
                                 setIndexesCallback)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCSize IndexesPopup::getPopupSize() { return {320.f, 240.f}; }

void IndexesPopup::createList() {
    auto size = this->m_mainLayer->getContentSize();

    constexpr float HORIZONTAL_PADDING = 5.f;

    if (m_list) {
        m_list->removeFromParent();
    }

    m_list = ScrollLayer::create({size.width - HORIZONTAL_PADDING * 2,
                                  size.height - HORIZONTAL_PADDING * 2 - 4.f});
    m_list->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(size.height - HORIZONTAL_PADDING * 2 - 4.f)
            ->setGap(HORIZONTAL_PADDING / 2));
    m_list->setPosition({HORIZONTAL_PADDING, HORIZONTAL_PADDING + 2.f});

    for (int i = 0; i < m_indexes.size(); i++) {
        auto cell = IndexCell::create(
            this, &m_indexes[i],
            [this, i] {
                m_indexes.erase(m_indexes.begin() + i);
                this->createList();
            },
            CCSize{this->getPopupSize().width - HORIZONTAL_PADDING * 2, 35.f});
        cell->setAnchorPoint({0.f, 0.f});
        m_list->m_contentLayer->addChild(cell);
    }
    auto menu = CCMenu::create();
    menu->setContentSize({0.f, 36.f});
    m_list->m_contentLayer->addChild(menu);

    m_list->m_contentLayer->updateLayout();
    this->m_mainLayer->addChild(m_list);
    handleTouchPriority(this);
}

}  // namespace jukebox
