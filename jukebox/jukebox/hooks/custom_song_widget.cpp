#include <Geode/Result.hpp>
#include <filesystem>
#include <optional>
#include <sstream>

#include <Geode/cocos/actions/CCActionInterval.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <fmt/format.h>

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/modify/CustomSongWidget.hpp>  // IWYU pragma: keep
#include <Geode/modify/LevelInfoLayer.hpp>    // IWYU pragma: keep
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/SimpleAxisLayout.hpp>
#include <Geode/utils/cocos.hpp>
#include <ranges>

#include <jukebox/events/song_state_changed.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/ui/nong_dropdown_layer.hpp>

using namespace geode::prelude;
using namespace jukebox;

class $modify(JBSongWidget, CustomSongWidget) {
    struct Fields {
        Nongs* nongs = nullptr;
        CCMenu* labelMenu = nullptr;
        CCMenu* pinMenu = nullptr;
        CCLabelBMFont* sizeIdLabel = nullptr;
        std::string songIds = "";
        std::string sfxIds = "";
        bool firstRun = true;
        bool searching = false;
        CCSprite* sprRays = nullptr;
        CCMenuItemSpriteExtra* btnDisc = nullptr;
        std::unordered_map<int, Nongs*> assetNongData;
        std::optional<int> levelID = std::nullopt;
    };

    std::optional<int> getLevelID() { return m_fields->levelID; }

    void setLevelID(int levelID) {
        m_fields->levelID = levelID;

        this->setVerifiedUI();
    }

    // Set the disc to show the song is verified.
    // The function only works if there is a verified song.
    void setVerifiedUI() {
        if (!m_fields->levelID.has_value()) {
            return;
        }
        std::vector<int> songIDs;
        // m_songs size + SongInfoObject id
        songIDs.reserve(m_songs.size() + 1);
        for (auto const& kv : m_songs) {
            songIDs.push_back(kv.first);
        }

        int id = m_songInfoObject->m_songID;
        if (m_isRobtopSong) {
            id++;
            id = -id;
        }
        songIDs.push_back(id);

        bool isVerified = !NongManager::get().getVerifiedNongsForLevel(m_fields->levelID.value(), songIDs).empty();
        if (!isVerified) {
            return;
        }

        if (m_fields->sprRays) {
            m_fields->sprRays->setVisible(true);
        }
        if (m_fields->btnDisc) {
            CCSprite* sprDiscGold = CCSprite::createWithSpriteFrameName("JB_PinDiscGold.png"_spr);
            if (m_isMusicLibrary) {
                sprDiscGold->setScale(0.5f);
            } else {
                sprDiscGold->setScale(0.7f);
            }
            m_fields->btnDisc->setSprite(sprDiscGold);
        }
    }

    bool init(SongInfoObject* songInfo, CustomSongDelegate* songDelegate, bool showSongSelect, bool showPlayMusic,
              bool showDownload, bool isRobtopSong, bool unk, bool isMusicLibrary, int unkInt) {
        this->adjustSongInfoObject(songInfo);
        if (!CustomSongWidget::init(songInfo, songDelegate, showSongSelect, showPlayMusic, showDownload, isRobtopSong,
                                    unk, isMusicLibrary, unkInt)) {
            return false;
        }
        /*log::info("CSW INIT");*/
        /*log::info("name: {}, artist: {}, url: {}, unknown: {}, id: {}",*/
        /*          songInfo->m_songName, songInfo->m_artistName,*/
        /*          songInfo->m_songUrl, songInfo->m_isUnknownSong,*/
        /*          songInfo->m_songID);*/
        /*log::info(*/
        /*    "songselect {}, playmusic {}, download {}, robtop {}, unk {}, "*/
        /*    "musiclib {}, int: {}",*/
        /*    m_showSelectSongBtn, m_showPlayMusicBtn, m_showDownloadBtn,*/
        /*    m_isRobtopSong, unk, m_isMusicLibrary, unkInt);*/
        this->setupJBSW();
        m_fields->firstRun = false;

        this->addEventListener(
            "CSW-song-state"_spr,
            event::SongStateChanged(),
            [this](const event::SongStateChangedData& event) {
                if (!m_songInfoObject) {
                    return ListenerResult::Propagate;
                }

                this->maybeChangeDownloadedMark(event.nongs());

                if (event.nongs()->songID() !=
                    NongManager::get().adjustSongID(m_songInfoObject->m_songID, m_isRobtopSong)) {
                    return ListenerResult::Propagate;
                }

                Song* active = event.nongs()->active();

                m_songInfoObject->m_songName = active->metadata()->name;
                m_songInfoObject->m_artistName = active->metadata()->artist;
                this->updateSongInfo();
                this->fixMultiAssetSize();

                return ListenerResult::Propagate;
            });

        return true;
    }

    void maybeChangeDownloadedMark(Nongs* nongs) {
        if (m_songs.size() == 0) {
            return;
        }

        std::optional optPath = nongs->active()->path();

        if (auto found = m_songs.find(nongs->songID()); found != m_songs.end()) {
            if (!optPath) {
                found->second = false;
            } else {
                found->second = std::filesystem::exists(optPath.value());
            }
        }
    }

    void adjustSongInfoObject(SongInfoObject* object) {
        if (!object || !object->m_unloaded) {
            return;
        }

        std::optional<Nongs*> opt = NongManager::get().getNongs(object->m_songID);

        if (!opt.has_value()) {
            return;
        }

        Nongs* nongs = opt.value();

        object->m_songName = nongs->active()->metadata()->name;
        object->m_artistName = nongs->active()->metadata()->artist;
    }

    void updateWithMultiAssets(gd::string p1, gd::string p2, int p3) {
        CustomSongWidget::updateWithMultiAssets(p1, p2, p3);
        m_fields->songIds = std::string(p1);
        m_fields->sfxIds = std::string(p2);
        this->fixMultiAssetSize();
        if (!m_songInfoObject || m_isRobtopSong) {
            return;
        }

        if (!m_fields->nongs) {
            auto res = NongManager::get().getNongs(m_songInfoObject->m_songID);
            if (res) {
                m_fields->nongs = res.value();
                this->createSongLabels(m_fields->nongs);
            }
        } else {
            this->createSongLabels(m_fields->nongs);
        }
    }

    void updateMultiAssetInfo(bool p) {
        CustomSongWidget::updateMultiAssetInfo(p);
        this->fixMultiAssetSize();
    }

    void onFixMultiAssetSizeFinished(std::string value) {
        if (m_songIDLabel == nullptr) {
            return;
        }

        int downloaded = 0;
        int total = 0;
        for (const auto& kv : m_songs) {
            total++;
            if (kv.second) {
                downloaded++;
            }
        }
        for (const auto& kv : m_sfx) {
            total++;
            if (kv.second) {
                downloaded++;
            }
        }

        std::string label;

        if (downloaded != total) {
            label = fmt::format("Songs: {}  SFX: {}  Size: {} ({}/{})", m_songs.size(), m_sfx.size(), value,
                                downloaded, total);
        } else {
            label = fmt::format("Songs: {}  SFX: {}  Size: {}", m_songs.size(), m_sfx.size(), value);
        }

        m_songIDLabel->setString(label.c_str());
    }

    void fixMultiAssetSize() {
        auto flag = Mod::get()->getSettingValue<bool>("fix-empty-size");
        if ((m_fields->songIds.empty() && m_fields->sfxIds.empty()) || !flag) {
            return;
        }

        const auto resources = std::filesystem::path(CCFileUtils::get()->getWritablePath2()) / "Resources";
        const auto song = std::filesystem::path(CCFileUtils::get()->getWritablePath());

        async::spawn(NongManager::get().getMultiAssetSizes(m_fields->songIds, m_fields->sfxIds, resources, song),
                     [this](std::string result) { this->onFixMultiAssetSizeFinished(std::move(result)); });
    }

    void restoreUI() {
        if (m_fields->pinMenu != nullptr) {
            m_fields->pinMenu->removeFromParent();
            m_fields->pinMenu = nullptr;
        }
        if (m_fields->labelMenu != nullptr) {
            m_songLabel->setVisible(true);
            m_fields->labelMenu->removeFromParent();
            m_fields->labelMenu = nullptr;
        }

        this->resetIdAndSizeLabel();
    }

    void onDelete(CCObject* obj) {
        if (!m_fields->nongs) {
            CustomSongWidget::onDelete(obj);
            return;
        }

        if (!m_fields->nongs->isDefaultActive()) {
            FLAlertLayer::create("Cannot delete",
                                 "Cannot delete song while a NONG is set. "
                                 "Change to default song first.",
                                 "Ok")
                ->show();
        } else {
            CustomSongWidget::onDelete(obj);
        }
    }

    void setupJBSW() {
        SongInfoObject* obj = m_songInfoObject;
        if (obj == nullptr) {
            return;
        }

        if (m_showSelectSongBtn && obj->m_artistName.empty() && obj->m_songUrl.empty()) {
            this->restoreUI();
            return;
        }

        if (m_songs.size() != 0) {
            this->getMultiAssetSongInfo();
        }

        int id = obj->m_songID;
        int adjustedId = NongManager::get().adjustSongID(obj->m_songID, m_isRobtopSong);

        if (adjustedId == 0) {
            this->restoreUI();
            return;
        }

        Nongs* nongs = nullptr;

        if (!NongManager::get().hasSongID(adjustedId)) {
            Result<Nongs*> res = NongManager::get().initSongID(obj, id, m_isRobtopSong);
            if (res.isErr()) {
                std::string err = res.unwrapErr();
                log::error("{}", err);
                FLAlertLayer::create("Error",
                                     fmt::format("Failed to initialize Jukebox data for this "
                                                 "song ID. Please open a help thread on the "
                                                 "Discord server if this keeps happening. Error: {}",
                                                 err),
                                     "Ok")
                    ->show();
                this->restoreUI();
                return;
            } else {
                nongs = res.unwrap();
            }
        } else {
            nongs = NongManager::get().getNongs(adjustedId).value();
        }

        m_fields->nongs = nongs;
        this->createSongLabels(nongs);
    }

    void updateSongInfo() {
        // log::info("update song object");
        // log::info("songselect {}, playmusic {}, download {}, robtop {}, unk
        // {}, musiclib {}", m_showSelectSongBtn, m_showPlayMusicBtn,
        // m_showDownloadBtn, m_isRobtopSong, m_unkBool2, m_isMusicLibrary);

        if (m_fields->firstRun && m_isRobtopSong && m_songInfoObject) {
            std::optional<Nongs*> opt =
                NongManager::get().getNongs(NongManager::get().adjustSongID(m_songInfoObject->m_songID, true));
            if (opt) {
                Nongs* n = opt.value();

                m_songInfoObject->m_songName = n->active()->metadata()->name;
                m_songInfoObject->m_artistName = n->active()->metadata()->artist;
            }
        }

        CustomSongWidget::updateSongInfo();
        if (!m_fields->firstRun) {
            this->setupJBSW();
        }
    }

    void getMultiAssetSongInfo() {
        for (auto const& kv : m_songs) {
            Nongs* nongs = nullptr;
            if (!NongManager::get().hasSongID(kv.first)) {
                Result<Nongs*> result = NongManager::get().initSongID(nullptr, kv.first, false);

                if (result.isErr()) {
                    log::error("Failed to init multi asset song: {}", result.unwrapErr());
                    continue;
                }

                nongs = result.unwrap();
            } else {
                nongs = NongManager::get().getNongs(kv.first).value();
            }
            m_fields->assetNongData[kv.first] = nongs;
        }
    }

    void resetIdAndSizeLabel() {
        if (m_songIDLabel == nullptr || m_songInfoObject == nullptr) {
            return;
        }

        if (m_songs.size() == 0 && m_sfx.size() == 0) {
            std::string label;
            if (m_isMusicLibrary) {
                label = fmt::format("ID: {}  Size: {}MB", m_songInfoObject->m_songID, m_songInfoObject->m_fileSize);
            } else {
                label = fmt::format("SongID: {}  Size: {}MB", m_songInfoObject->m_songID, m_songInfoObject->m_fileSize);
            }

            m_songIDLabel->setString(label.c_str());
        } else {
            int downloaded = 0;
            int total = 0;
            for (auto [k, v] : m_songs) {
                total++;
                if (v) {
                    downloaded++;
                }
            }
            for (auto [k, v] : m_sfx) {
                total++;
                if (v) {
                    downloaded++;
                }
            }

            std::string label;

            if (downloaded != total) {
                label = fmt::format("Songs: {}  SFX: {}  Size: 0.00MB ({}/{})", m_songs.size(), m_sfx.size(),
                                    downloaded, total);
            } else {
                label = fmt::format("Songs: {}  SFX: {}  Size: 0.0MB", m_songs.size(), m_sfx.size());
            }

            m_songIDLabel->setString(label.c_str());
        }
    }

    void updateIdAndSizeLabel(Nongs* nongs) {
        if (m_songs.size() > 0 || m_sfx.size() > 0) {
            return;
        }

        Song* active = nongs->active();
        std::optional<std::filesystem::path> activePathOpt = active->path();
        int songID = NongManager::get().adjustSongID(nongs->songID(), m_isRobtopSong);

        if (!activePathOpt) {
            return;
        }

        std::filesystem::path activePath = activePathOpt.value();

        std::string sizeText;
        if (std::filesystem::exists(activePath)) {
            sizeText = NongManager::get().getFormattedSize(activePath);
        } else if (m_songInfoObject) {
            sizeText = fmt::format("{:.2f}MB", m_songInfoObject->m_fileSize);
        } else {
            sizeText = "N/A";
        }

        std::string labelText;
        std::string idDisplay;
        if (nongs->isDefaultActive()) {
            std::stringstream ss;
            int displayId = songID;
            if (displayId < 0) {
                displayId = (-displayId) - 1;
                ss << "(R) ";
            }
            ss << displayId;
            idDisplay = ss.str();
        } else if (songID < 0) {
            idDisplay = "(R) NONG";
        } else {
            idDisplay = "NONG";
        }

        if (m_isMusicLibrary) {
            labelText = fmt::format("ID: {}  Size: {}", idDisplay, sizeText);
        } else {
            labelText = fmt::format("SongID: {}  Size: {}", idDisplay, sizeText);
        }

        if (m_songIDLabel != nullptr) {
            m_songIDLabel->setString(labelText.c_str());
        }
        geode::cocos::handleTouchPriority(this);
    }

    void addPopupOpener(Nongs* nongs) {
        Song* active = nongs->active();

        if (m_fields->pinMenu != nullptr) {
            m_fields->pinMenu->removeFromParent();
            m_fields->pinMenu = nullptr;
        }

        if (m_fields->labelMenu != nullptr) {
            m_fields->labelMenu->removeFromParent();
            m_fields->labelMenu = nullptr;
        }

        if (Mod::get()->getSettingValue<bool>("old-label-display")) {
            m_fields->labelMenu = CCMenu::create();
            m_fields->labelMenu->setID("nong-menu"_spr);
            m_fields->labelMenu->ignoreAnchorPointForPosition(false);
            CCLabelBMFont* label = CCLabelBMFont::create(active->metadata()->name.c_str(), "bigFont.fnt");
            label->setID("song-name-label"_spr);
            CCMenuItemSpriteExtra* btn =
                CCMenuItemSpriteExtra::create(label, this, menu_selector(JBSongWidget::addNongLayer));
            btn->setID("nong-button"_spr);
            float labelScale = label->getScale();
            m_fields->labelMenu->addChild(btn);
            m_fields->labelMenu->setAnchorPoint(m_songLabel->getAnchorPoint());
            m_fields->labelMenu->setContentSize(m_songLabel->getContentSize());
            m_fields->labelMenu->setScale(m_songLabel->getScale());
            m_fields->labelMenu->setPosition(m_songLabel->getPosition());
            m_fields->labelMenu->setLayout(SimpleRowLayout::create()
                                               ->setMainAxisScaling(AxisScaling::Scale)
                                               ->setCrossAxisScaling(AxisScaling::Scale));
            m_songLabel->setVisible(false);
            this->addChild(m_fields->labelMenu);
        } else {
            CCSize pos;

            if (!m_isMusicLibrary) {
                if (m_ncsLogo && m_ncsLogo->isVisible()) {
                    pos = m_ncsLogo->getPosition();
                } else {
                    pos = m_songLabel->getPosition();
                }

                pos = pos + CCSize{-9.0f, 13.0f};
            } else {
                pos = m_songIDLabel->getPosition() + CCSize{-7.0f, -11.0f};
            }

            m_fields->pinMenu = CCMenu::create();
            m_fields->pinMenu->setID("nong-menu"_spr);

            CCSprite* sprRays = CCSprite::createWithSpriteFrameName("JB_PinDiscRays.png"_spr);
            CCSprite* sprDisc = CCSprite::createWithSpriteFrameName("JB_PinDisc.png"_spr);
            if (m_isMusicLibrary) {
                sprDisc->setScale(0.5f);
                sprRays->setScale(0.5f * 1.15f);
            } else {
                sprDisc->setScale(0.7f);
                sprRays->setScale(0.7f * 1.15f);
            }
            sprDisc->setID("nong-pin"_spr);

            CCMenuItemSpriteExtra* btnDisc =
                CCMenuItemSpriteExtra::create(sprDisc, this, menu_selector(JBSongWidget::addNongLayer));
            btnDisc->setID("nong-button"_spr);

            sprRays->setID("nong-pin-rays"_spr);
            sprRays->setZOrder(-1);
            sprRays->setAnchorPoint({0.5f, 0.5f});
            sprRays->setVisible(false);

            m_fields->sprRays = sprRays;
            m_fields->btnDisc = btnDisc;

            m_fields->pinMenu->addChildAtPosition(sprRays, Anchor::Center);

            static constexpr float rotationDuration = 80.0f;
            CCRepeatForever* rotateAction = CCRepeatForever::create(CCRotateBy::create(rotationDuration, 360));
            sprRays->runAction(rotateAction);

            m_fields->pinMenu->setAnchorPoint({0.5f, 0.5f});
            m_fields->pinMenu->ignoreAnchorPointForPosition(false);
            m_fields->pinMenu->addChildAtPosition(btnDisc, Anchor::Center);
            m_fields->pinMenu->setContentSize(btnDisc->getScaledContentSize());
            m_fields->pinMenu->setLayout(AnchorLayout::create());

            m_fields->pinMenu->setPosition(pos);
            this->addChild(m_fields->pinMenu);

            this->setVerifiedUI();
        }
    }

    void createSongLabels(Nongs* nongs) {
        this->addPopupOpener(nongs);
        this->updateIdAndSizeLabel(nongs);
    }

    void addNongLayer(CCObject* target) {
        if (m_songs.size() != 0) {
            this->getMultiAssetSongInfo();
        }
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        std::vector<int> ids;
        int id = m_songInfoObject->m_songID;
        if (m_isRobtopSong) {
            id++;
            id = -id;
        }
        if (m_songs.size() > 1) {
            for (auto const& kv : m_songs) {
                if (!NongManager::get().getNongs(kv.first).has_value()) {
                    return;
                }
                ids.push_back(kv.first);
            }
        } else {
            ids.push_back(id);
        }
        auto layer = NongDropdownLayer::create(ids, this, id, this->getLevelID());
        // based robtroll
        layer->setZOrder(106);
        layer->show();
    }
};

class $modify(JBLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) {
            return false;
        }
        if (Mod::get()->getSavedValue("show-tutorial", true) && GameManager::get()->m_levelEditorLayer == nullptr) {
            auto popup = FLAlertLayer::create("Jukebox",
                                              "Thank you for using <co>Jukebox</c>! To "
                                              "begin swapping songs, click "
                                              "on the <cr>song name</c>!",
                                              "Ok");
            Mod::get()->setSavedValue("show-tutorial", false);
            popup->m_scene = this;
            popup->show();
        }
        static_cast<JBSongWidget*>(this->m_songWidget)->setLevelID(m_level->m_levelID.value());
        return true;
    }
};
