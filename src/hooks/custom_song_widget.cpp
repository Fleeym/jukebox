#include <memory>
#include <sstream>

#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/binding/CustomSongWidget.hpp"
#include "Geode/binding/GameManager.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/cocos/base_nodes/Layout.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/loader/Event.hpp"
#include "Geode/loader/Log.hpp"
#include "Geode/modify/CustomSongWidget.hpp"  // IWYU pragma: keep
#include "Geode/modify/LevelInfoLayer.hpp"    // IWYU pragma: keep
#include "Geode/ui/GeodeUI.hpp"
#include "Geode/utils/cocos.hpp"

#include "managers/nong_manager.hpp"
#include "ui/nong_dropdown_layer.hpp"

using namespace geode::prelude;
using namespace jukebox;

class $modify(JBSongWidget, CustomSongWidget) {
    struct Fields {
        Nongs* nongs = nullptr;
        CCMenu* menu = nullptr;
        CCMenuItemSpriteExtra* songNameLabel = nullptr;
        CCLabelBMFont* sizeIdLabel = nullptr;
        std::string songIds = "";
        std::string sfxIds = "";
        bool firstRun = true;
        bool searching = false;
        std::unordered_map<int, Nongs*> assetNongData;
        EventListener<NongManager::MultiAssetSizeTask> m_multiAssetListener;
        std::unique_ptr<EventListener<EventFilter<event::SongStateChanged>>>
            m_songStateListener;
    };

    bool init(SongInfoObject* songInfo, CustomSongDelegate* songDelegate,
              bool showSongSelect, bool showPlayMusic, bool showDownload,
              bool isRobtopSong, bool unk, bool isMusicLibrary, int unkInt) {
        if (!CustomSongWidget::init(songInfo, songDelegate, showSongSelect,
                                    showPlayMusic, showDownload, isRobtopSong,
                                    unk, isMusicLibrary, unkInt)) {
            return false;
        }
        // log::info("CSW INIT");
        // log::info("{}, {}, {}, {}, {}", songInfo->m_songName,
        // songInfo->m_artistName, songInfo->m_songUrl,
        // songInfo->m_isUnkownSong, songInfo->m_songID); log::info("songselect
        // {}, playmusic {}, download {}, robtop {}, unk {}, musiclib {}, int:
        // {}", m_showSelectSongBtn, m_showPlayMusicBtn, m_showDownloadBtn,
        // m_isRobtopSong, unk, m_isMusicLibrary, unkInt);
        m_songLabel->setVisible(false);
        this->setupJBSW();
        m_fields->firstRun = false;

        m_fields->m_songStateListener = std::make_unique<
            EventListener<EventFilter<event::SongStateChanged>>>(
            ([this](event::SongStateChanged* event) {
                if (!m_songInfoObject) {
                    return ListenerResult::Propagate;
                }
                if (event->nongs()->songID() != m_songInfoObject->m_songID) {
                    return ListenerResult::Propagate;
                }

                Song* active = event->nongs()->active();

                m_songInfoObject->m_songName = active->metadata()->name;
                m_songInfoObject->m_artistName = active->metadata()->artist;
                this->updateSongInfo();

                return ListenerResult::Propagate;
            }));

        return true;
    }

    void updateWithMultiAssets(gd::string p1, gd::string p2, int p3) {
        CustomSongWidget::updateWithMultiAssets(p1, p2, p3);
        m_fields->songIds = std::string(p1);
        m_fields->sfxIds = std::string(p2);
        this->fixMultiAssetSize();
        if (m_isRobtopSong) {
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

    void fixMultiAssetSize() {
        auto flag = Mod::get()->getSettingValue<bool>("fix-empty-size");
        if ((m_fields->songIds.empty() && m_fields->sfxIds.empty()) || !flag) {
            return;
        }

        m_fields->m_multiAssetListener.bind(
            [this](NongManager::MultiAssetSizeTask::Event* e) {
                if (!m_songIDLabel) {
                    return;
                }
                if (const std::string* value = e->getValue()) {
                    m_songIDLabel->setString(
                        fmt::format("Songs: {}  SFX: {}  Size: {}",
                                    m_songs.size(), m_sfx.size(), *value)
                            .c_str());
                }
            });
        m_fields->m_multiAssetListener.setFilter(
            NongManager::get().getMultiAssetSizes(m_fields->songIds,
                                                  m_fields->sfxIds));
    }

    void restoreUI() {
        m_songLabel->setVisible(true);
        if (m_fields->menu != nullptr) {
            m_fields->songNameLabel->removeFromParent();
            m_fields->songNameLabel = nullptr;
            m_fields->menu->removeFromParent();
            m_fields->menu = nullptr;
        }
        if (m_fields->sizeIdLabel != nullptr) {
            m_songIDLabel->setVisible(true);
            m_fields->sizeIdLabel->removeFromParent();
            m_fields->sizeIdLabel = nullptr;
        }
    }

    void setupJBSW() {
        SongInfoObject* obj = m_songInfoObject;
        if (obj == nullptr) {
            return;
        }
        if (m_songs.size() != 0) {
            this->getMultiAssetSongInfo();
        }
        int id = obj->m_songID;
        if (m_isRobtopSong) {
            id++;
            id = -id;
        }
        if (id == 0) {
            this->restoreUI();
            return;
        }
        if (m_showSelectSongBtn && obj->m_artistName.empty() &&
            obj->m_songUrl.empty()) {
            this->restoreUI();
            return;
        }
        m_songLabel->setVisible(false);
        NongManager::get().initSongID(obj, obj->m_songID, m_isRobtopSong);
        std::optional<Nongs*> result = NongManager::get().getNongs(
            NongManager::get().adjustSongID(id, m_isRobtopSong));

        auto nongs = result.value();

        m_fields->nongs = nongs;
        this->createSongLabels(nongs);
        if (nongs->isDefaultActive()) {
            m_deleteBtn->setVisible(false);
        }
    }

    void updateSongInfo() {
        // log::info("update song object");
        // log::info("songselect {}, playmusic {}, download {}, robtop {}, unk
        // {}, musiclib {}", m_showSelectSongBtn, m_showPlayMusicBtn,
        // m_showDownloadBtn, m_isRobtopSong, m_unkBool2, m_isMusicLibrary);
        CustomSongWidget::updateSongInfo();
        if (!m_fields->firstRun) {
            this->setupJBSW();
        }
    }

    void getMultiAssetSongInfo() {
        for (auto const& kv : m_songs) {
            NongManager::get().initSongID(nullptr, kv.first, false);
            auto result = NongManager::get().getNongs(kv.first);
            auto value = result.value();
            m_fields->assetNongData[kv.first] = value;
        }
    }

    void createSongLabels(Nongs* nongs) {
        int songID = m_songInfoObject->m_songID;
        if (m_isRobtopSong) {
            songID++;
            songID = -songID;
        }
        Song* active = nongs->active();
        if (m_fields->menu != nullptr) {
            m_fields->menu->removeFromParent();
            m_fields->menu = nullptr;
        }
        auto menu = CCMenu::create();
        menu->setID("song-name-menu");
        auto label = CCLabelBMFont::create(active->metadata()->name.c_str(),
                                           "bigFont.fnt");
        auto songNameMenuLabel = CCMenuItemSpriteExtra::create(
            label, this, menu_selector(JBSongWidget::addNongLayer));
        auto labelScale = label->getScale();
        songNameMenuLabel->setID("song-name-label");
        m_fields->songNameLabel = songNameMenuLabel;
        menu->addChild(songNameMenuLabel);
        menu->setLayout(RowLayout::create()
                            ->setDefaultScaleLimits(0.1f, 1.0f)
                            ->setAxisAlignment(AxisAlignment::Start));
        menu->setAnchorPoint(m_songLabel->getAnchorPoint());
        menu->setContentSize(m_songLabel->getScaledContentSize());
        menu->setPosition(m_songLabel->getPosition());
        menu->updateLayout();
        m_fields->menu = menu;
        this->addChild(menu);
        if (m_songs.size() == 0 && m_sfx.size() == 0 && !m_isMusicLibrary) {
            if (m_fields->sizeIdLabel != nullptr) {
                m_fields->sizeIdLabel->removeFromParent();
            }
            auto data = NongManager::get().getNongs(songID).value();

            // TODO this might be fuckery
            if (!std::filesystem::exists(active->path().value()) &&
                nongs->isDefaultActive()) {
                m_songIDLabel->setVisible(true);
                geode::cocos::handleTouchPriority(this);
                return;
            } else if (m_songIDLabel) {
                m_songIDLabel->setVisible(false);
            }

            std::string sizeText;
            if (std::filesystem::exists(active->path().value())) {
                sizeText =
                    NongManager::get().getFormattedSize(active->path().value());
            } else {
                sizeText = "NA";
            }
            std::string labelText;
            if (nongs->isDefaultActive()) {
                std::stringstream ss;
                int displayId = songID;
                if (displayId < 0) {
                    displayId = (-displayId) - 1;
                    ss << "(R) ";
                }
                ss << displayId;
                labelText = "SongID: " + ss.str() + "  Size: " + sizeText;
            } else {
                std::string display = "NONG";
                if (songID < 0) {
                    display = "(R) NONG";
                }
                labelText = "SongID: " + display + "  Size: " + sizeText;
            }

            auto label =
                CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
            label->setID("nongd-id-and-size-label");
            label->setPosition(ccp(-139.f, -31.f));
            label->setAnchorPoint({0, 0.5f});
            label->setScale(0.4f);
            this->addChild(label);
            m_fields->sizeIdLabel = label;
            geode::cocos::handleTouchPriority(this);
        } else {
            if (m_fields->sizeIdLabel) {
                m_fields->sizeIdLabel->setVisible(false);
            }
            m_songIDLabel->setVisible(true);
            geode::cocos::handleTouchPriority(this);
        }
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
        auto layer = NongDropdownLayer::create(ids, this, id);
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
        if (Mod::get()->getSavedValue("show-tutorial", true) &&
            GameManager::get()->m_levelEditorLayer == nullptr) {
            auto popup =
                FLAlertLayer::create("Jukebox",
                                     "Thank you for using <co>Jukebox</c>! To "
                                     "begin swapping songs, click "
                                     "on the <cr>song name</c>!",
                                     "Ok");
            Mod::get()->setSavedValue("show-tutorial", false);
            popup->m_scene = this;
            popup->show();
        }
        return true;
    }
};
