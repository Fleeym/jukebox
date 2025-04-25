#include <jukebox/ui/list/nong_list.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_set>

#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/platform/CCPlatformMacros.h>
#include <fmt/core.h>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/cocos.hpp>

#include <jukebox/events/nong_deleted.hpp>
#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/ui/list/nong_cell.hpp>
#include <jukebox/ui/list/song_cell.hpp>

using namespace geode::prelude;

namespace jukebox {

bool NongList::init(
    std::vector<int>& songIds, const cocos2d::CCSize& size,
    std::optional<int> levelID,
    std::function<void(std::optional<int>)> onListTypeChange
) {
    if (!CCNode::init()) {
        return false;
    }

    m_songIds = songIds;
    m_levelID = levelID;
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
        ScrollLayer::create({size.width, size.height - s_padding});
    m_list->m_contentLayer->setLayout(ColumnLayout::create()
                                          ->setAxisReverse(true)
                                          ->setAxisAlignment(AxisAlignment::End)
                                          ->setAutoGrowAxis(size.height)
                                          ->setGap(s_padding / 2));
    m_list->m_contentLayer->setPositionX(s_padding / 2.f);

    this->addChildAtPosition(m_list, Anchor::Center,
                             -m_list->getScaledContentSize() / 2);

    m_onListTypeChange(m_currentSong);

    this->build();
    return true;
}

void NongList::setDownloadProgress(std::string uniqueID, float progress) {}

void NongList::build() {
    if (m_list->m_contentLayer->getChildrenCount() > 0) {
        m_list->m_contentLayer->removeAllChildrenWithCleanup(true);
    }

    if (m_songIds.size() == 0) {
        return;
    }

    CCSize itemSize = {m_list->getScaledContentSize().width - s_padding, s_itemSize};

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
            id,
            defaultID,
            itemSize,
            m_levelID,
            std::nullopt
        ));

        CCLabelBMFont* localSongs =
            CCLabelBMFont::create("Stored nongs", "goldFont.fnt");
        localSongs->setID("local-section");
        localSongs->setLayoutOptions(
            AxisLayoutOptions::create()->setScaleLimits(0.2f, 0.6f));
        m_list->m_contentLayer->addChild(localSongs);

        std::unordered_set<std::string> localYt;
        std::unordered_set<std::string> localHosted;

        if (nongs->locals().size() == 0 && nongs->youtube().size() == 0 &&
            nongs->hosted().size() == 0) {
            this->addNoLocalSongsNotice();
        }

        std::vector<Song*> allLocalNongs;

        for (std::unique_ptr<LocalSong>& nong : nongs->locals()) {
            allLocalNongs.push_back(nong.get());
        }

        for (std::unique_ptr<YTSong>& nong : nongs->youtube()) {
            allLocalNongs.push_back(nong.get());
            if (nong->indexID().has_value()) {
                localYt.insert(fmt::format("{}|{}", nong->indexID().value(),
                                           nong->metadata()->uniqueID));
            }
        }

        for (std::unique_ptr<HostedSong>& nong : nongs->hosted()) {
            allLocalNongs.push_back(nong.get());
            if (nong->indexID().has_value()) {
                localHosted.insert(fmt::format("{}|{}", nong->indexID().value(),
                                               nong->metadata()->uniqueID));
            }
        }

        
        std::vector<std::string> verifiedNongs = m_levelID.has_value()
                                                    ? NongManager::get().getVerifiedNongsForLevel(
                                                        m_levelID.value(),
                                                        {m_currentSong.value()}
                                                    )
                                                    : std::vector<std::string>{};

        std::sort(allLocalNongs.begin(), allLocalNongs.end(), [verifiedNongs](Song* a, Song* b) {
            auto sourcePriority = [](Song* s) {
                if (dynamic_cast<LocalSong*>(s)) return 0;
                if (dynamic_cast<HostedSong*>(s)) return 1;
                if (dynamic_cast<YTSong*>(s)) return 2;
                return 3;
            };

            bool aVerified = std::find(verifiedNongs.begin(), verifiedNongs.end(), a->metadata()->uniqueID) != verifiedNongs.end();
            bool bVerified = std::find(verifiedNongs.begin(), verifiedNongs.end(), b->metadata()->uniqueID) != verifiedNongs.end();

            // Sort by Verified
            if (aVerified != bVerified)
                return aVerified;

            // Then by source priority
            int sourceA = sourcePriority(a);
            int sourceB = sourcePriority(b);
            if (sourceA != sourceB)
                return sourceA < sourceB;

            // Then by name 
            return a->metadata()->name < b->metadata()->name;
        });

        for (Song* nong : allLocalNongs) {
            this->addSongToList(nong, nongs);
        }

        if (nongs->indexSongs().size() > 0) {
            CCLabelBMFont* indexLabel =
                CCLabelBMFont::create("Download nongs", "goldFont.fnt");
            indexLabel->setID("index-section");
            indexLabel->setLayoutOptions(
                AxisLayoutOptions::create()->setScaleLimits(0.2f, 0.6f));
            m_list->m_contentLayer->addChild(indexLabel);
        }

        std::vector<index::IndexSongMetadata*> allIndexNongs;

        for (index::IndexSongMetadata* index : nongs->indexSongs()) {
            const std::string id =
                fmt::format("{}|{}", index->parentID->m_id, index->uniqueID);

            if (index->ytId.has_value() && localYt.contains(id)) {
                continue;
            }

            if (index->url.has_value() && localHosted.contains(id)) {
                continue;
            }

            allIndexNongs.push_back(index);
        }

        std::sort(allIndexNongs.begin(), allIndexNongs.end(), [this](
                  index::IndexSongMetadata* a,
                  index::IndexSongMetadata* b) {
            auto sourcePriority = [](index::IndexSongMetadata* s) {
                if (s->url.has_value()) return 0;
                if (s->ytId.has_value()) return 1;
                return 2;
            };

            bool aVerified = m_levelID.has_value() ? std::find(a->verifiedLevelIDs.begin(), a->verifiedLevelIDs.end(), m_levelID.value()) != a->verifiedLevelIDs.end() : false;
            bool bVerified = m_levelID.has_value() ? std::find(b->verifiedLevelIDs.begin(), b->verifiedLevelIDs.end(), m_levelID.value()) != b->verifiedLevelIDs.end() : false;

            // Sort by Verified
            if (aVerified != bVerified)
                return aVerified;

            // Then by source priority
            int sourceA = sourcePriority(a);
            int sourceB = sourcePriority(b);
            if (sourceA != sourceB)
                return sourceA < sourceB;

            // Then by name 
            return a->name < b->name;
            
        });

        for (index::IndexSongMetadata* indexNong : allIndexNongs) {
            this->addIndexSongToList(indexNong, nongs);
        }
    }
    m_list->m_contentLayer->updateLayout();
    this->scrollToTop();
}

void NongList::addSongToList(Song* nong, Nongs* parent, bool liveInsert) {
    int id = m_currentSong.value();
    Song* defaultSong = parent->defaultSong();
    Song* active = parent->active();

    CCSize itemSize = {m_list->getScaledContentSize().width - s_padding, s_itemSize};

    std::string uniqueID = nong->metadata()->uniqueID;
    std::optional<std::filesystem::path> path = nong->path();
    bool isFromIndex = nong->indexID().has_value();
    bool isDownloaded =
        path.has_value() && std::filesystem::exists(path.value());
    NongCell* cell = NongCell::create(
        id,
        uniqueID,
        itemSize,
        m_levelID,
        std::nullopt
    );
    cell->setID(uniqueID);

    if (!liveInsert) {
        m_list->m_contentLayer->addChild(cell);
        return;
    }

    CCNode* indexSection =
        m_list->m_contentLayer->getChildByID("index-section");

    if (indexSection) {
        m_list->m_contentLayer->insertBefore(cell, indexSection);
    } else {
        m_list->m_contentLayer->addChild(cell);
    }
}

void NongList::addIndexSongToList(index::IndexSongMetadata* song,
                                  Nongs* parent) {
    const CCSize itemSize = {m_list->getScaledContentSize().width - s_padding, s_itemSize};
    NongCell* cell = NongCell::create(
        m_currentSong.value(),
        song->uniqueID,
        itemSize,
        m_levelID,
        song
    );
    cell->setID(fmt::format("{}-{}", song->parentID->m_id, song->uniqueID));
    m_list->m_contentLayer->addChild(cell);
}

void NongList::addNoLocalSongsNotice(bool liveInsert) {
    CCLabelBMFont* label =
        CCLabelBMFont::create("You have no stored nongs :(", "bigFont.fnt");
    label->setID("no-local-songs");
    label->setLayoutOptions(AxisLayoutOptions::create()->setAutoScale(false));
    label->limitLabelWidth(150.0f, 0.7f, 0.1f);

    if (!liveInsert) {
        m_list->m_contentLayer->addChild(label);
    } else {
        CCNode* section = m_list->m_contentLayer->getChildByID("local-section");
        if (!section) {
            log::error(
                "Couldn't insert no local songs notice. Section not found");
        } else {
            m_list->m_contentLayer->insertAfter(label, section);
        }
    }
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
    if (!m_list || !m_currentSong.has_value() ||
        !e->indexSource().has_value()) {
        return ListenerResult::Propagate;
    }

    std::optional<Nongs*> optNongs =
        NongManager::get().getNongs(m_currentSong.value());

    if (!optNongs.has_value()) {
        return ListenerResult::Propagate;
    }

    Nongs* nongs = optNongs.value();

    if (!nongs->findSong(e->destination()->metadata()->uniqueID)) {
        return ListenerResult::Propagate;
    }

    index::IndexSongMetadata* meta = e->indexSource().value();
    if (CCNode* i = m_list->m_contentLayer->getChildByID(
            fmt::format("{}-{}", meta->parentID->m_id, meta->uniqueID))) {
        i->removeFromParentAndCleanup(true);
    }

    auto res = nongs->setActive(e->destination()->metadata()->uniqueID);
    if (!res) {
        log::error("Failed to set newly downloaded song {} as active: {}",
                   e->destination()->metadata()->uniqueID, res.unwrapErr());
    }

    this->addSongToList(e->destination(), nongs, true);

    // Remove "you have no local songs label"
    if (auto node = m_list->m_contentLayer->getChildByID("no-local-songs")) {
        node->removeFromParentAndCleanup(true);
    }

    m_list->m_contentLayer->updateLayout();

    return ListenerResult::Propagate;
}

ListenerResult NongList::onNongDeleted(event::NongDeleted* e) {
    if (!m_list || !m_currentSong.has_value() ||
        m_currentSong.value() != e->gdId()) {
        return ListenerResult::Propagate;
    }

    CCNode* found = m_list->m_contentLayer->getChildByID(e->uniqueId());
    if (found) {
        found->removeFromParentAndCleanup(true);
        m_list->m_contentLayer->updateLayout();
    }

    std::optional<Nongs*> optNongs =
        NongManager::get().getNongs(m_currentSong.value());

    if (!optNongs.has_value()) {
        return ListenerResult::Propagate;
    }

    Nongs* nongs = optNongs.value();

    for (index::IndexSongMetadata* i : nongs->indexSongs()) {
        if (i->uniqueID != e->uniqueId()) {
            continue;
        }

        if (!m_list->m_contentLayer->getChildByID("index-section")) {
            CCLabelBMFont* indexLabel =
                CCLabelBMFont::create("Download nongs", "goldFont.fnt");
            indexLabel->setID("index-section");
            indexLabel->setLayoutOptions(
                AxisLayoutOptions::create()->setScaleLimits(0.2f, 0.6f));
            m_list->m_contentLayer->addChild(indexLabel);
        }

        this->addIndexSongToList(i, nongs);
        m_list->m_contentLayer->updateLayout();
    }

    return ListenerResult::Propagate;
}

ListenerResult NongList::onSongAdded(event::ManualSongAdded* e) {
    if (!m_list || !m_currentSong.has_value() ||
        m_currentSong.value() != e->nongs()->songID()) {
        return ListenerResult::Propagate;
    }

    this->addSongToList(e->song(), e->nongs(), true);

    // Remove "you have no local songs label"
    if (auto node = m_list->m_contentLayer->getChildByID("no-local-songs")) {
        node->removeFromParentAndCleanup(true);
    }

    m_list->m_contentLayer->updateLayout();
    return ListenerResult::Propagate;
}

NongList* NongList::create(
    std::vector<int>& songIds, const cocos2d::CCSize& size,
    std::optional<int> levelID,
    std::function<void(std::optional<int>)> onListTypeChange) {
    auto ret = new NongList();
    if (!ret->init(songIds, size, levelID, onListTypeChange)) {
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    ret->autorelease();
    return ret;
}

}  // namespace jukebox
