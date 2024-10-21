#include "ui/list/nong_list.hpp"
#include <fmt/core.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_set>

#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/binding/OptionsScrollLayer.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/base_nodes/Layout.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/platform/CCPlatformMacros.h"
#include "Geode/loader/Event.hpp"
#include "Geode/platform/windows.hpp"
#include "Geode/ui/ScrollLayer.hpp"

#include "Geode/utils/cocos.hpp"
#include "events/song_download_finished.hpp"
#include "index.hpp"
#include "managers/index_manager.hpp"
#include "managers/nong_manager.hpp"
#include "nong.hpp"
#include "ui/list/index_song_cell.hpp"
#include "ui/list/nong_cell.hpp"
#include "ui/list/song_cell.hpp"

using namespace geode::prelude;

namespace jukebox {

bool NongList::init(
    std::vector<int>& songIds, const cocos2d::CCSize& size,
    std::function<void(int, const std::string&)> onSetActive,
    std::function<void(int)> onFixDefault,
    std::function<void(int, const std::string&, bool onlyAudio, bool confirm)>
        onDelete,
    std::function<void(int, const std::string&)> onDownload,
    std::function<void(int, const std::string&)> onEdit,
    std::function<void(std::optional<int>)> onListTypeChange) {
    if (!CCNode::init()) {
        return false;
    }

    m_songIds = songIds;
    m_onSetActive = onSetActive;
    m_onFixDefault = onFixDefault;
    m_onDelete = onDelete;
    m_onDownload = onDownload;
    m_onEdit = onEdit;
    m_onListTypeChange = onListTypeChange;

    if (m_songIds.size() == 1) {
        m_currentSong = m_songIds.front();
    }

    this->setContentSize(size);
    this->setAnchorPoint({0.5f, 0.5f});

    m_bg = CCScale9Sprite::create("square02b_001.png");
    m_bg->setColor({0, 0, 0});
    m_bg->setOpacity(75);
    m_bg->setScale(0.3f);
    m_bg->setContentSize(size / m_bg->getScale());
    this->addChildAtPosition(m_bg, Anchor::Center);

    CCMenu* menu = CCMenu::create();
    auto spr = CCSprite::createWithSpriteFrameName("backArrowPlain_01_001.png");
    auto backBtn = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(NongList::onBack));
    m_backBtn = backBtn;
    m_backBtn->setVisible(false);
    menu->addChild(backBtn);
    menu->setLayout(ColumnLayout::create());
    menu->updateLayout();
    menu->setZOrder(1);
    this->addChildAtPosition(menu, Anchor::Left, CCPoint{-10.0f, 0.0f});

    m_list =
        ScrollLayer::create({size.width - s_padding, size.height - s_padding});
    m_list->m_contentLayer->setLayout(ColumnLayout::create()
                                          ->setAxisReverse(true)
                                          ->setAxisAlignment(AxisAlignment::End)
                                          ->setAutoGrowAxis(size.height)
                                          ->setGap(s_padding / 2));

    this->addChildAtPosition(m_list, Anchor::Center,
                             -m_list->getScaledContentSize() / 2);

    m_onListTypeChange(m_currentSong);

    this->build();
    return true;
}

void NongList::setDownloadProgress(std::string uniqueID, float progress) {
    for (auto& cell : listedNongCells) {
        if (cell->m_songInfo->metadata()->uniqueID == uniqueID) {
            cell->setDownloadProgress(progress);
            break;
        }
    }
}

void NongList::build() {
    if (m_list->m_contentLayer->getChildrenCount() > 0) {
        m_list->m_contentLayer->removeAllChildren();
    }

    if (m_songIds.size() == 0) {
        return;
    }

    CCSize itemSize = {m_list->getScaledContentSize().width, s_itemSize};

    if (m_onListTypeChange) {
        m_onListTypeChange(m_currentSong);
    }

    if (!m_currentSong) {
        for (int id : m_songIds) {
            std::optional<Nongs*> nongs = NongManager::get().getNongs(id);

            if (!nongs) {
                continue;
            }

            Song* active = nongs.value()->active();

            m_list->m_contentLayer->addChild(
                SongCell::create(id, active->metadata(), itemSize,
                                 [this, id]() { this->onSelectSong(id); }));
        }
    } else {
        // Single item
        int id = m_currentSong.value();

        std::optional<Nongs*> optNongs = NongManager::get().getNongs(id);
        if (!optNongs) {
            return;
        }

        Nongs* nongs = optNongs.value();
        LocalSong* defaultSong = nongs->defaultSong();
        std::string defaultID = defaultSong->metadata()->uniqueID;
        Song* active = nongs->active();

        m_list->m_contentLayer->addChild(NongCell::create(
            id, defaultSong, true,
            defaultSong->metadata()->uniqueID == active->metadata()->uniqueID,
            itemSize, [this, id, defaultID]() { m_onSetActive(id, defaultID); },
            [this, id]() { m_onFixDefault(id); },
            [this, id, defaultID]() { m_onDelete(id, defaultID, true, true); },
            [this, id, defaultID]() { m_onDownload(id, defaultID); },
            [this, id, defaultID]() { m_onEdit(id, defaultID); }));

        CCLabelBMFont* localSongs =
            CCLabelBMFont::create("Stored nongs", "goldFont.fnt");
        localSongs->setLayoutOptions(
            AxisLayoutOptions::create()->setScaleLimits(0.2f, 0.6f));
        m_list->m_contentLayer->addChild(localSongs);

        std::unordered_set<std::string> localYt;
        std::unordered_set<std::string> localHosted;

        if (nongs->locals().size() == 0 && nongs->youtube().size() == 0 &&
            nongs->hosted().size() == 0) {
            CCLabelBMFont* label = CCLabelBMFont::create(
                "You have no stored nongs :(", "bigFont.fnt");
            label->setLayoutOptions(
                AxisLayoutOptions::create()->setAutoScale(false));
            label->limitLabelWidth(150.0f, 0.7f, 0.1f);
            m_list->m_contentLayer->addChild(label);
        }

        for (std::unique_ptr<LocalSong>& nong : nongs->locals()) {
            this->addSongToList(nong.get(), nongs);
        }

        for (std::unique_ptr<YTSong>& nong : nongs->youtube()) {
            this->addSongToList(nong.get(), nongs);
            if (nong->indexID().has_value()) {
                localYt.insert(fmt::format("{}|{}", nong->indexID().value(),
                                           nong->metadata()->uniqueID));
            }
        }

        for (std::unique_ptr<HostedSong>& nong : nongs->hosted()) {
            this->addSongToList(nong.get(), nongs);
            if (nong->indexID().has_value()) {
                localHosted.insert(fmt::format("{}|{}", nong->indexID().value(),
                                               nong->metadata()->uniqueID));
            }
        }

        if (nongs->indexSongs().size() > 0) {
            CCLabelBMFont* indexLabel =
                CCLabelBMFont::create("Download nongs", "goldFont.fnt");
            indexLabel->setLayoutOptions(
                AxisLayoutOptions::create()->setScaleLimits(0.2f, 0.6f));
            m_list->m_contentLayer->addChild(indexLabel);
        }

        for (index::IndexSongMetadata* index : nongs->indexSongs()) {
            const std::string id =
                fmt::format("{}|{}", index->parentID->m_id, index->uniqueID);

            if (index->ytId.has_value() && localYt.contains(id)) {
                continue;
            }

            if (index->url.has_value() && localHosted.contains(id)) {
                continue;
            }

            this->addIndexSongToList(index, nongs);
        }
    }
    m_list->m_contentLayer->updateLayout();
    this->scrollToTop();
}

void NongList::addSongToList(Song* nong, Nongs* parent) {
    int id = m_currentSong.value();
    Song* defaultSong = parent->defaultSong();
    Song* active = parent->active();

    CCSize itemSize = {m_list->getScaledContentSize().width, s_itemSize};

    std::string uniqueID = nong->metadata()->uniqueID;
    std::optional<std::filesystem::path> path = nong->path();
    bool isFromIndex = nong->indexID().has_value();
    bool isDownloaded =
        path.has_value() && std::filesystem::exists(path.value());
    NongCell* cell = NongCell::create(
        id, nong, uniqueID == defaultSong->metadata()->uniqueID,
        uniqueID == active->metadata()->uniqueID, itemSize,
        [this, id, uniqueID]() { m_onSetActive(id, uniqueID); },
        [this, id]() { m_onFixDefault(id); },
        [this, id, uniqueID]() { m_onDelete(id, uniqueID, false, true); },
        [this, id, uniqueID]() { m_onDownload(id, uniqueID); },
        [this, id, uniqueID]() { m_onEdit(id, uniqueID); });

    if (auto progress = IndexManager::get().getSongDownloadProgress(uniqueID);
        progress.has_value()) {
        cell->setDownloadProgress(progress.value());
    };

    m_list->m_contentLayer->addChild(cell);
}

void NongList::addIndexSongToList(index::IndexSongMetadata* song,
                                  Nongs* parent) {
    const CCSize itemSize = {m_list->getScaledContentSize().width, s_itemSize};
    IndexSongCell* cell =
        IndexSongCell::create(song, m_currentSong.value(), itemSize);

    m_list->m_contentLayer->addChild(cell);
}

void NongList::scrollToTop() {
    m_list->m_contentLayer->setPositionY(
        -m_list->m_contentLayer->getContentHeight() +
        m_list->getContentHeight());
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
    if (std::find(m_songIds.begin(), m_songIds.end(), songId) !=
        m_songIds.end()) {
        m_currentSong = songId;
    }
}

void NongList::onSelectSong(int songId) {
    if (m_currentSong.has_value() ||
        std::find(m_songIds.begin(), m_songIds.end(), songId) ==
            m_songIds.end()) {
        return;
    }

    m_currentSong = songId;
    this->build();
    this->scrollToTop();
    m_backBtn->setVisible(true);
}

ListenerResult NongList::onDownloadFinish(event::SongDownloadFinished* e) {
    if (!m_currentSong.has_value()) {
        return ListenerResult::Propagate;
    }
    std::optional<Nongs*> optNongs =
        NongManager::get().getNongs(m_currentSong.value());

    if (!optNongs.has_value()) {
        return ListenerResult::Propagate;
    }

    Nongs* nongs = optNongs.value();

    if (e->destination()->type() == NongType::HOSTED) {
        HostedSong* destination = static_cast<HostedSong*>(e->destination());
        for (const std::unique_ptr<HostedSong>& song : nongs->hosted()) {
            if (destination != song.get()) {
                continue;
            }

            this->build();
            return ListenerResult::Propagate;
        }
    }

    if (e->destination()->type() == NongType::YOUTUBE) {
        YTSong* destination = static_cast<YTSong*>(e->destination());
        for (const std::unique_ptr<YTSong>& song : nongs->youtube()) {
            if (destination != song.get()) {
                continue;
            }

            this->build();
            break;
        }
    }

    return ListenerResult::Propagate;
}

NongList* NongList::create(
    std::vector<int>& songIds, const cocos2d::CCSize& size,
    std::function<void(int, const std::string&)> onSetActive,
    std::function<void(int)> onFixDefault,
    std::function<void(int, const std::string&, bool onlyAudio, bool confirm)>
        onDelete,
    std::function<void(int, const std::string&)> onDownload,
    std::function<void(int, const std::string&)> onEdit,
    std::function<void(std::optional<int>)> onListTypeChange) {
    auto ret = new NongList();
    if (!ret->init(songIds, size, onSetActive, onFixDefault, onDelete,
                   onDownload, onEdit, onListTypeChange)) {
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    ret->autorelease();
    return ret;
}

}  // namespace jukebox
