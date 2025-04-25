#include <jukebox/ui/list/nong_cell.hpp>

#include <functional>

#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <fmt/core.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/Popup.hpp>

#include <jukebox/events/song_download_failed.hpp>
#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/events/song_state_changed.hpp>
#include <jukebox/managers/index_manager.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/events/start_download.hpp>
#include <jukebox/ui/list/nong_cell_ui.hpp>
#include <jukebox/ui/nong_add_popup.hpp>
#include <jukebox/utils/get_domain_from_url.hpp>

using namespace geode::prelude;

namespace jukebox {

bool NongCell::init(
    int gdSongID,
    const std::string& uniqueID,
    const cocos2d::CCSize& size,
    std::optional<int> levelID,
    std::optional<index::IndexSongMetadata*> indexSongMetadataOpt
) {
    if (!CCNode::init()) {
        return false;
    }

    m_songID = gdSongID;
    m_uniqueID = uniqueID;
    m_levelID = levelID;
    m_indexSongMetadataOpt = indexSongMetadataOpt;

    m_nongCell = NongCellUI::create(
        size,
        [this]{ this->onSelect(); },
        [this]{ this->onTrash(); },
        [this]{ this->onFixDefault(); },
        [this]{ this->onDownload(); },
        [this]{ this->onEdit(); }
    );

    if (isIndex()) {
        bool success = initIndex();
        if (!success) {
            return false;
        }
    } else {
        bool success = initLocal();
        if (!success) {
            return false;
        }
    }

    build();

    this->setContentSize(m_nongCell->getContentSize());
    this->addChildAtPosition(m_nongCell, Anchor::Center);

    return true;
}

bool NongCell::initLocal() {
    auto nongs = NongManager::get().getNongs(m_songID);
    if (!nongs.has_value()) {
        log::error("Failed to create Nong cell for {}.", m_uniqueID);
        return false;
    }

    std::optional<Song*> songInfoOpt = nongs.value()->findSong(m_uniqueID);
    if (!songInfoOpt.has_value()) {
        log::error("Failed to create Nong cell for {}.", m_uniqueID);
        return false;
    }

    Song* songInfo = songInfoOpt.value();

    std::vector<std::string> verifiedNongs = m_levelID.has_value() ? NongManager::get().getVerifiedNongsForLevel(
        m_levelID.value(),
        {m_songID}
    ) : std::vector<std::string>{};

    std::vector<std::string> metadataList = {};

    switch (songInfo->type()) {
        case NongType::YOUTUBE:
            metadataList.push_back("youtube");
            break;
        case NongType::HOSTED:
            metadataList.push_back("hosted");
            break;
        default:
            break;
    }

    if (songInfo->indexID().has_value()) {
        auto indexID = songInfo->indexID().value();
        auto indexName = IndexManager::get().getIndexName(indexID);
        metadataList.push_back(indexName.has_value() ? indexName.value()
                                                          : indexID);
    } else if (songInfo->type() == NongType::HOSTED) {
        std::string url = (static_cast<HostedSong*>(songInfo))->url();
        metadataList.push_back(getDomainFromUrl(url).value_or(url));
    } else if (songInfo->type() == NongType::YOUTUBE) {
        metadataList.push_back((static_cast<YTSong*>(songInfo))->youtubeID());
    }

    if (songInfo->metadata()->level.has_value()) {
        metadataList.push_back("level");
        metadataList.push_back(songInfo->metadata()->level.value());
    }

    m_nongCell->m_songName = songInfo->metadata()->name;
    m_nongCell->m_authorName = songInfo->metadata()->artist;

    std::ostringstream oss;
    for (size_t i = 0; i + 1 < metadataList.size(); i += 2) {
        if (i > 0) oss << " | ";
        oss << metadataList[i] << ": " << metadataList[i + 1];
    }
    m_nongCell->m_metadata = oss.str();

    m_isVerified = std::find(verifiedNongs.begin(), verifiedNongs.end(), m_uniqueID) != verifiedNongs.end();
    m_isDefault = nongs.value()->defaultSong()->metadata()->uniqueID == m_uniqueID;
    m_isActive = nongs.value()->active()->metadata()->uniqueID == m_uniqueID;
    m_isDownloaded = songInfo->path().has_value() &&
                     std::filesystem::exists(songInfo->path().value());
    m_isDownloadable = songInfo->type() != NongType::LOCAL;
    m_isDownloading = false;
    m_nongCell->m_showEditButton = !m_isDefault && !songInfo->indexID().has_value();

    return true;
}

bool NongCell::initIndex() {
    index::IndexSongMetadata* indexSongMetadata = m_indexSongMetadataOpt.value();

    std::vector<std::string> metadataList = {};

    if (indexSongMetadata->ytId.has_value()) {
        metadataList.push_back("youtube");
    } else if (indexSongMetadata->url.has_value()) {
        metadataList.push_back("hosted");
    } else {
        log::error("Couldn't figure out index song type of {}", indexSongMetadata->uniqueID);
        return false;
    }

    metadataList.push_back(indexSongMetadata->parentID->m_name);

    std::ostringstream oss;
    for (size_t i = 0; i + 1 < metadataList.size(); i += 2) {
        if (i > 0) oss << " | ";
        oss << metadataList[i] << ": " << metadataList[i + 1];
    }
    m_nongCell->m_metadata = oss.str();

    m_nongCell->m_songName = indexSongMetadata->name;
    m_nongCell->m_authorName = indexSongMetadata->artist;

    auto& verifiedLevelIDs = indexSongMetadata->verifiedLevelIDs;
    m_isVerified = m_levelID.has_value()
                   ? std::find(verifiedLevelIDs.begin(), verifiedLevelIDs.end(), m_levelID.value()) != verifiedLevelIDs.end()
                   : false;
    m_isDefault = false;
    m_isActive = false;
    m_isDownloaded = false;
    m_isDownloadable = true;
    m_isDownloading = false;
    m_nongCell->m_showEditButton = false;

    return true;
}

void NongCell::build() {
    m_nongCell->m_isVerified = m_isVerified;
    m_nongCell->m_isDownloaded = m_isDownloaded;
    m_nongCell->m_isSelected = m_isActive;
    m_nongCell->m_isDownloading = m_isDownloading;

    m_nongCell->m_showDownloadButton = m_isDownloadable && !m_isDownloaded;
    m_nongCell->m_showFixDefaultButton = m_isDefault;
    m_nongCell->m_showSelectButton = m_isDownloaded || m_isDefault;
    m_nongCell->m_showTrashButton = !m_isDefault && !isIndex();

    m_nongCell->build();
}

void NongCell::onSelect() {
    if (auto err = NongManager::get().setActiveSong(m_songID, m_uniqueID);
        err.isErr()) {
        FLAlertLayer::create(
            "Failed", fmt::format("Failed to set song: {}", err.unwrapErr()),
            "Ok")
            ->show();
        return;
    }
}

void NongCell::onTrash() {
    deleteSong(false);
}

void NongCell::onDownload() {
    if (m_nongCell->m_isDownloading) {
        log::info("Canceling downloads is not currently implemented.");
        return;
    }
    event::StartDownload(m_songID, m_uniqueID).post();
}

void NongCell::onEdit() {
    auto nongsOpt = NongManager::get().getNongs(m_songID);
    if (!nongsOpt.has_value()) {
        return;
    }
    std::optional<Song*> songInfoOpt = nongsOpt.value()->findSong(m_uniqueID);
    if (!songInfoOpt.has_value()) {
        return;
    }
    NongAddPopup::create(m_songID, songInfoOpt.value())->show();
}

void NongCell::onFixDefault() {
    if (!m_isActive) {
        FLAlertLayer::create(
            "Error", "Set the default song as <cr>active</c> first", "Ok")
            ->show();
        return;
    }

    createQuickPopup(
        "Fix default",
        "Do you want to refetch song info <cb>for the default song</c>? Use "
        "this <cr>ONLY</c> if it gets renamed by accident!",
        "No", "Yes", [this](FLAlertLayer* alert, bool btn2) {
            if (btn2) {
                NongManager::get().refetchDefault(m_songID);
            }
        });
}

ListenerResult NongCell::onDownloadProgress(event::SongDownloadProgress* e) {
    if (e->uniqueID() != m_uniqueID || e->gdSongID() != m_songID) {
        return ListenerResult::Propagate;
    }

    m_isDownloading = true;
    m_nongCell->m_isDownloading = m_isDownloading;
    m_nongCell->m_downloadProgress = e->progress();
    m_nongCell->buildOnlyDownloadProgress();

    return ListenerResult::Propagate;
}

ListenerResult NongCell::onDownloadFailed(event::SongDownloadFailed* e) {
    if (e->gdSongId() != m_songID || e->uniqueId() != m_uniqueID) {
        return ListenerResult::Propagate;
    }

    m_isDownloading = false;
    build();

    return ListenerResult::Propagate;
}

ListenerResult NongCell::onDownloadFinish(event::SongDownloadFinished* e) {
    if (e->destination()->metadata()->gdID != m_songID ||
        e->destination()->metadata()->uniqueID != m_uniqueID) {
        return ListenerResult::Propagate;
    }

    m_isDownloading = false;
    m_isDownloaded = true;
    build();

    return ListenerResult::Propagate;
}

ListenerResult NongCell::onStateChange(event::SongStateChanged* e) {
    bool sameIDAsActive =
        e->nongs()->active()->metadata()->uniqueID == m_uniqueID;
    bool switchedToActive = !m_isActive && sameIDAsActive;
    bool switchedToInactive = m_isActive && !sameIDAsActive;

    if (e->nongs()->songID() != m_songID || (!m_isDownloaded && !m_isDefault)) {
        return ListenerResult::Propagate;
    }

    if (!switchedToActive && !switchedToInactive) {
        return ListenerResult::Propagate;
    }

    bool selected = e->nongs()->active()->metadata()->uniqueID == m_uniqueID;
    m_isActive = selected;

    build();


    return ListenerResult::Propagate;
}

// Only on default song when doing fix default
ListenerResult NongCell::onGetSongInfo(event::GetSongInfo* e) {
    if (e->gdSongID() != m_songID || !m_isDefault) {
        return ListenerResult::Propagate;
    }

    m_nongCell->m_songName = e->songName();
    m_nongCell->m_authorName = e->artistName();
    build();

    return ListenerResult::Propagate;
}

void NongCell::deleteSong(bool onlyAudio) {
    auto func = [this, onlyAudio]() {
        if (onlyAudio) {
            if (auto err =
                    NongManager::get().deleteSongAudio(this->m_songID, this->m_uniqueID);
                err.isErr()) {
                log::error("Failed to delete audio for {}: {}", this->m_uniqueID,
                           err.unwrapErr());
            }
        } else {
            if (auto err = NongManager::get().deleteSong(
                                                            this->m_songID,
                                                            this->m_uniqueID
                                                            );
                err.isErr()) {
                FLAlertLayer::create(
                    "Failed",
                    fmt::format("Failed to delete song: {}", err.unwrapErr()),
                    "Ok")
                    ->show();
                return;
            }
        }

        FLAlertLayer::create("Success", "The song was deleted!", "Ok")
            ->show();
    };

    createQuickPopup(
        "Are you sure?",
        fmt::format(
            "Are you sure you want to delete the song from your NONGs?"),
        "No", "Yes", [this, func](FLAlertLayer* self, bool btn2) {
            if (!btn2) {
                return;
            }
            func();
    });

}

NongCell* NongCell::create(
    int gdSongID,
    const std::string& uniqueID,
    const cocos2d::CCSize& size,
    std::optional<int> levelID,
    std::optional<index::IndexSongMetadata*> indexSongMetadataOpt
) {
    auto ret = new NongCell();
    if (ret && ret->init(gdSongID, uniqueID, size, levelID, indexSongMetadataOpt)) {
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool NongCell::isIndex() {
    return m_indexSongMetadataOpt.has_value();
}


}
