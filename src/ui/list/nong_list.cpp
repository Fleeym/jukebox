#include "nong_list.hpp"

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/platform/CCPlatformMacros.h>
#include <Geode/ui/ScrollLayer.hpp>
#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <filesystem>
#include <optional>
#include <unordered_map>

#include "../../managers/nong_manager.hpp"
#include "../../managers/index_manager.hpp"
#include "song_cell.hpp"

using namespace geode::prelude;

namespace jukebox {

bool NongList::init(
    std::vector<int>& songIds,
    const cocos2d::CCSize& size,
    std::function<void(int, const std::string&)> onSetActive,
    std::function<void(int)> onFixDefault,
    std::function<void(int, const std::string&)> onDelete,
    std::function<void(int, const std::string&)> onDownload,
    std::function<void(std::optional<int>)> onListTypeChange
) {
    if (!CCNode::init()) {
        return false;
    }

    m_songIds = songIds;
    m_onSetActive = onSetActive;
    m_onFixDefault = onFixDefault;
    m_onDelete = onDelete;
    m_onDownload = onDownload;
    m_onListTypeChange = onListTypeChange;

    if (m_songIds.size() == 1) {
        m_currentSong = m_songIds.front();
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

    m_onListTypeChange(m_currentSong);

    this->build();
    return true;
}

void NongList::setDownloadProgress(std::string uniqueID, float progress) {
    for (auto& cell : listedNongCells) {
        if (cell->m_songInfo.metadata()->m_uniqueID == uniqueID) {
            cell->setDownloadProgress(progress);
            break;
        }
    }
}

void NongList::build() {
    listedNongCells.clear();
    if (m_list->m_contentLayer->getChildrenCount() > 0) {
        m_list->m_contentLayer->removeAllChildren();
    }

    if (m_songIds.size() == 0) {
        return;
    }

    CCSize itemSize = {
        m_list->getScaledContentSize().width,
        s_itemSize
    };

    if (m_onListTypeChange) {
        m_onListTypeChange(m_currentSong);
    }

    if (!m_currentSong) {
        for (const auto& id : m_songIds) {
            auto nongs = NongManager::get()->getNongs(id);

            if (!nongs) {
                continue;
            }

            auto active = nongs.value()->activeNong();

            m_list->m_contentLayer->addChild(
                jukebox::SongCell::create(
                    id,
                    active.metadata(),
                    itemSize,
                    [this, id] () {
                        this->onSelectSong(id);
                    }
                )
            );
        }
    } else {
        // Single item
        int id = m_currentSong.value();

        auto localNongs = NongManager::get()->getNongs(id);
        if (!localNongs) {
            return;
        }

        auto defaultSong = localNongs.value()->defaultSong();
        auto active = localNongs.value()->active();

        auto nongs = IndexManager::get()->getNongs(id);
        if (nongs.isErr()) {
            return;
        }

        for (auto& nong : nongs.value()) {
          auto metadata = *nong.metadata();
          auto path = nong.path();
          auto uniqueID = metadata.m_uniqueID;

          auto cell =  jukebox::NongCell::create(
              id,
              std::move(nong),
              uniqueID == defaultSong->path(),
              uniqueID == active,
              path.has_value() && std::filesystem::exists(path.value()),
              itemSize,
              [this, id, uniqueID] () {
                  m_onSetActive(id, uniqueID);
              },
              [this, id] () {
                  m_onFixDefault(id);
              },
              [this, id, uniqueID] () {
                  m_onDelete(id, uniqueID);
              },
              [this, id, uniqueID] () {
                  m_onDownload(id, uniqueID);
              }
          );

          if (auto progress = IndexManager::get()->getSongDownloadProgress(metadata.m_uniqueID); progress.has_value()) {
              cell->setDownloadProgress(progress.value());
          };

          listedNongCells.push_back(cell);
          m_list->m_contentLayer->addChild(cell);
        }

        // for (std::unique_ptr<LocalSong>& song : nongs.value()->locals()) {
        //     m_list->m_contentLayer->addChild(
        //         jukebox::NongCell::create(
        //             id, Nong(*song),
        //             false,
        //             song->path() == active->m_path,
        //             itemSize,
        //             [this, id, &song] () {
        //               m_onSetActive(id, SongMetadataPathed(
        //                   *song->metadata(),
        //                   song->path()
        //               ));
        //             },
        //             [this, id] () {
        //                 m_onFixDefault(id);
        //             },
        //             [this, id, &song] () {
        //                 m_onDelete(id, SongMetadataPathed{*song});
        //             }
        //         )
        //     );
        // }
    }
    m_list->m_contentLayer->updateLayout();
    this->scrollToTop();
}

void NongList::scrollToTop() {
    m_list->m_contentLayer->setPositionY(
        -m_list->m_contentLayer->getContentHeight() + m_list->getContentHeight()
    );
}

void NongList::onBack(cocos2d::CCObject* target) {
    if (!m_currentSong.has_value() || m_songIds.size() < 2) {
        return;
    }

    m_currentSong = std::nullopt;
    this->build();
    this->scrollToTop();
    m_backBtn->setVisible(false);
}

void NongList::setCurrentSong(int songId) {
    if (std::find(m_songIds.begin(), m_songIds.end(), songId) != m_songIds.end()) {
        m_currentSong = songId;
    }
}

void NongList::onSelectSong(int songId) {
    if (m_currentSong.has_value() || std::find(m_songIds.begin(), m_songIds.end(), songId) == m_songIds.end()) {
        return;
    }

    m_currentSong = songId;
    this->build();
    this->scrollToTop();
    m_backBtn->setVisible(true);
}

NongList* NongList::create(
    std::vector<int>& songIds,
    const cocos2d::CCSize& size,
    std::function<void(int, const std::string&)> onSetActive,
    std::function<void(int)> onFixDefault,
    std::function<void(int, const std::string&)> onDelete,
    std::function<void(int, const std::string&)> onDownload,
    std::function<void(std::optional<int>)> onListTypeChange
) {
    auto ret = new NongList();
    if (!ret->init(songIds, size, onSetActive, onFixDefault, onDelete, onDownload, onListTypeChange)) {
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    ret->autorelease();
    return ret;
}

}
