#include "nong_list.hpp"

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/platform/CCPlatformMacros.h>
#include <Geode/ui/ScrollLayer.hpp>
#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <optional>
#include <unordered_map>

#include "../../managers/nong_manager.hpp"
#include "nong_cell.hpp"
#include "song_cell.hpp"

using namespace geode::prelude;

namespace jukebox {

bool NongList::init(
    std::unordered_map<int, Nongs>& data,
    const cocos2d::CCSize& size,
    std::function<void(int, const SongInfo&)> onSetActive,
    std::function<void(int)> onFixDefault,
    std::function<void(int, const SongInfo&)> onDelete,
    std::function<void(bool)> onListTypeChange
) {
    if (!CCNode::init()) {
        return false;
    }

    m_data = data;
    m_onSetActive = onSetActive;
    m_onFixDefault = onFixDefault;
    m_onDelete = onDelete;
    m_onListTypeChange = onListTypeChange;

    if (m_data.size() == 1) {
        m_currentSong = m_data.begin()->first;
    }

    this->setContentSize(size);
    this->setAnchorPoint({ 0.5f, 0.5f });

    m_bg = CCScale9Sprite::create("square02b_001.png");
    m_bg->setColor({ 0, 0, 0 });
    m_bg->setOpacity(75);
    m_bg->setScale(0.3f);
    m_bg->setContentSize(size / m_bg->getScale());
    this->addChildAtPosition(m_bg, Anchor::Center);

    CCMenu* menu = CCMenu::create();
    auto spr = CCSprite::createWithSpriteFrameName("backArrowPlain_01_001.png");
    auto backBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongList::onBack)
    );
    m_backBtn = backBtn;
    m_backBtn->setVisible(false);
    menu->addChild(backBtn);
    menu->setLayout(ColumnLayout::create());
    menu->updateLayout();
    menu->setZOrder(1);
    this->addChildAtPosition(menu, Anchor::Left, CCPoint { -10.0f, 0.0f });

    m_list = ScrollLayer::create({ size.width - s_padding, size.height - s_padding });
    m_list->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(size.height)
            ->setGap(s_padding / 2)
    );

    this->addChildAtPosition(
        m_list,
        Anchor::Center,
        -m_list->getScaledContentSize() / 2
    );

    this->build();
    return true;
}

void NongList::build() {
    if (m_list->m_contentLayer->getChildrenCount() > 0) {
        m_list->m_contentLayer->removeAllChildren();
    }

    if (m_data.size() == 0) {
        return;
    }

    CCSize itemSize = {
        m_list->getScaledContentSize().width,
        s_itemSize
    };

    if (!m_currentSong) {
        if (m_onListTypeChange) {
            m_onListTypeChange(true);
        }
        for (const auto& kv : m_data) {
            SongInfo display;
            auto active = NongManager::get()->getActiveNong(kv.first);

            if (!active) {
                continue;
            }

            int id = kv.first;
            m_list->m_contentLayer->addChild(
                jukebox::SongCell::create(
                    id,
                    active.value(),
                    itemSize,
                    [this, id] () {
                        this->onSelectSong(id);
                    }
                )
            );
        }
    } else {
        if (m_onListTypeChange) {
            m_onListTypeChange(false);
        }
        // Single item
        int id = m_currentSong.value();
        if (!m_data.contains(id)) {
            return;
        }
        Nongs data = m_data.at(id);
        auto active = NongManager::get()->getActiveNong(id);
        auto defaultRes = NongManager::get()->getDefaultNong(id);
        if (!defaultRes) {
            return;
        }
        if (!active) {
            return;
        }

        auto defaultSong = defaultRes.value();

        m_list->m_contentLayer->addChild(
            jukebox::NongCell::create(
                id, defaultSong,
                true,
                defaultSong.path == data.active,
                itemSize,
                [this, id, defaultSong] () {
                    m_onSetActive(id, defaultSong);
                },
                [this, id] () {
                    m_onFixDefault(id);
                },
                [this, id, defaultSong] () {
                    m_onDelete(id, defaultSong);
                }
            )
        );

        for (const SongInfo& song : data.songs) {
            if (song.path == data.defaultPath) {
                continue;
            }
            m_list->m_contentLayer->addChild(
                jukebox::NongCell::create(
                    id, song,
                    song.path == data.defaultPath,
                    song.path == data.active,
                    itemSize,
                    [this, id, song] () {
                        m_onSetActive(id, song);
                    },
                    [this, id] () {
                        m_onFixDefault(id);
                    },
                    [this, id, song] () {
                        m_onDelete(id, song);
                    }
                )
            );
        }
    }
    m_list->m_contentLayer->updateLayout();
    this->scrollToTop();
}

void NongList::scrollToTop() {
    m_list->m_contentLayer->setPositionY(
        -m_list->m_contentLayer->getContentHeight() + m_list->getContentHeight()
    );
}

void NongList::setData(std::unordered_map<int, Nongs>& data) {
    m_data = data;
}

void NongList::onBack(cocos2d::CCObject* target) {
    if (!m_currentSong.has_value() || m_data.size() < 2) {
        return;
    }

    m_currentSong = std::nullopt;
    this->build();
    this->scrollToTop();
    m_backBtn->setVisible(false);
}

void NongList::setCurrentSong(int songId) {
    if (m_data.contains(songId)) {
        m_currentSong = songId;
    }
}

void NongList::onSelectSong(int songId) {
    if (m_currentSong.has_value() || !m_data.contains(songId)) {
        return;
    }

    m_currentSong = songId;
    this->build();
    this->scrollToTop();
    m_backBtn->setVisible(true);
}

NongList* NongList::create(
    std::unordered_map<int, Nongs>& data,
    const cocos2d::CCSize& size,
    std::function<void(int, const SongInfo&)> onSetActive,
    std::function<void(int)> onFixDefault,
    std::function<void(int, const SongInfo&)> onDelete,
    std::function<void(bool)> onListTypeChange
) {
    auto ret = new NongList();
    if (!ret->init(data, size, onSetActive, onFixDefault, onDelete, onListTypeChange)) {
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    ret->autorelease();
    return ret;
}

}