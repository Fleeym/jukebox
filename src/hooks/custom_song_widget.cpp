#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/LevelTools.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/CustomSongWidget.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/cocos.hpp>
#include <sstream>

#include "../types/song_info.hpp"
#include "../managers/nong_manager.hpp"
#include "../ui/nong_dropdown_layer.hpp"

using namespace geode::prelude;
using namespace jukebox;

class $modify(JBSongWidget, CustomSongWidget) {
    struct Fields {
        NongData nongs;
        CCMenu* menu;
        CCMenuItemSpriteExtra* songNameLabel;
        CCLabelBMFont* sizeIdLabel;
        std::string songIds = "";
        std::string sfxIds = "";
        bool fetchedAssetInfo = false;
        bool firstRun = true;
        bool searching = false;
        std::unordered_map<int, NongData> assetNongData;
        EventListener<NongManager::MultiAssetSizeTask> m_multiAssetListener;
    };

    bool init(
        SongInfoObject* songInfo,
        CustomSongDelegate* songDelegate,
        bool showSongSelect,
        bool showPlayMusic,
        bool showDownload,
        bool isRobtopSong,
        bool unk,
        bool isMusicLibrary,
        int unkInt
    ) {
        if (!CustomSongWidget::init(
            songInfo, 
            songDelegate, 
            showSongSelect, 
            showPlayMusic, 
            showDownload, 
            isRobtopSong, 
            unk, 
            isMusicLibrary, 
            unkInt
        )) {
            return false;
        }
        // log::info("CSW INIT");
        // log::info("{}, {}, {}, {}, {}", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songUrl, songInfo->m_isUnkownSong, songInfo->m_songID);
        // log::info("songselect {}, playmusic {}, download {}, robtop {}, unk {}, musiclib {}, int: {}", m_showSelectSongBtn, m_showPlayMusicBtn, m_showDownloadBtn, m_isRobtopSong, unk, m_isMusicLibrary, unkInt);
        m_songLabel->setVisible(false);
        this->setupJBSW();
        m_fields->firstRun = false;
        return true;
    }

    void updateWithMultiAssets(gd::string p1, gd::string p2, int p3) {
        CustomSongWidget::updateWithMultiAssets(p1, p2, p3);
        m_fields->songIds = std::string(p1);
        m_fields->sfxIds = std::string(p2);
        if (m_fields->fetchedAssetInfo) {
            this->fixMultiAssetSize();
        }
        if (m_isRobtopSong) {
            return;
        }
        this->createSongLabels();
    }

    void updateMultiAssetInfo(bool p) {
        CustomSongWidget::updateMultiAssetInfo(p);
        if (m_fields->fetchedAssetInfo) {
            this->fixMultiAssetSize();
        }
    }

    void fixMultiAssetSize() {
        auto flag = Mod::get()->getSettingValue<bool>("fix-empty-size");
        if ((m_fields->songIds.empty() && m_fields->sfxIds.empty()) || !flag) {
            return;
        }

        m_fields->m_multiAssetListener.bind([this](NongManager::MultiAssetSizeTask::Event* e) {
            if (!m_songIDLabel) {
                return;
            }
            if (const std::string* value = e->getValue()) {
                m_songIDLabel->setString(fmt::format("Songs: {}  SFX: {}  Size: {}", m_songs.size(), m_sfx.size(), *value).c_str());
            }
        });
        m_fields->m_multiAssetListener.setFilter(NongManager::get()->getMultiAssetSizes(m_fields->songIds, m_fields->sfxIds));
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
        if (obj == nullptr) return;
        if (!m_fields->fetchedAssetInfo && m_songs.size() != 0) {
            this->getMultiAssetSongInfo();
        }
        int id = obj->m_songID;
        if (m_isRobtopSong) {
            log::info("whaT?????");
            id++;
            id = -id;
        }
        if (id == 0) {
            this->restoreUI();
            return;
        }
        if (m_showSelectSongBtn && obj->m_artistName.empty() && obj->m_songUrl.empty()) {
            this->restoreUI();
            return;
        }
        m_songLabel->setVisible(false);
        if (obj->m_songUrl.empty() && obj->m_artistName.empty()) {
            // we have an invalid songID
            if (!NongManager::get()->getNongs(id).has_value()) {
                NongManager::get()->createUnknownDefault(id);
            }
        }
        std::optional<NongData> result = NongManager::get()->getNongs(id);
        if (!result.has_value()) {
            NongManager::get()->createDefault(m_songInfoObject, id, m_isRobtopSong);
            result = NongManager::get()->getNongs(id);
            if (!result.has_value()) {
                return;
            }
        }

        NongData nongData = result.value();

        SongInfo active = NongManager::get()->getActiveNong(id).value();
        if (
            !m_isRobtopSong
            && !nongData.defaultValid 
            && active.path == nongData.defaultPath
            && !NongManager::get()->isFixingDefault(id)
        ) {
            NongManager::get()->prepareCorrectDefault(id);
            this->template addEventListener<GetSongInfoEventFilter>(
                [this, id](SongInfoObject* obj) {
                    if (!NongManager::get()->isFixingDefault(id)) {
                        return ListenerResult::Propagate;
                    }
                    // log::info("update song object");
                    // log::info("{}, {}, {}, {}, {}", obj->m_songName, obj->m_artistName, obj->m_songUrl, obj->m_isUnkownSong, id);
                    NongManager::get()->fixDefault(obj);
                    m_sliderGroove->setVisible(false);
                    m_fields->nongs = NongManager::get()->getNongs(id).value();
                    this->createSongLabels();
                    return ListenerResult::Propagate;
                },
                id
            );
            auto result = NongManager::get()->getNongs(id);
            if (result) {
                nongData = result.value();
            }
        }
        m_fields->nongs = nongData;
        this->createSongLabels();
        if (active.path != nongData.defaultPath) {
            m_deleteBtn->setVisible(false);
        }
    }

    void updateSongInfo() {
        log::info("update song object");
        log::info("songselect {}, playmusic {}, download {}, robtop {}, unk {}, musiclib {}", m_showSelectSongBtn, m_showPlayMusicBtn, m_showDownloadBtn, m_isRobtopSong, m_unkBool2, m_isMusicLibrary);

        CustomSongWidget::updateSongInfo();
        if (!m_fields->firstRun) {
            this->setupJBSW();
        }
    }

    void getMultiAssetSongInfo() {
        bool allDownloaded = true;
        for (auto const& kv : m_songs) {
            auto result = NongManager::get()->getNongs(kv.first);
            if (!result.has_value()) {
                NongManager::get()->createDefault(nullptr, kv.first, false);
                result = NongManager::get()->getNongs(kv.first);
                if (!result.has_value()) {
                    // its downloading
                    allDownloaded = false;
                    continue;
                }
            }
            auto value = result.value();
            m_fields->assetNongData[kv.first] = value;
        }
        if (allDownloaded) {
            m_fields->fetchedAssetInfo = true;
        }
    }

    void createSongLabels() {
        int songID = m_songInfoObject->m_songID;
        if (m_isRobtopSong) {
            songID++;
            songID = -songID;
        }
        log::info("createsonglabel");
        auto res = NongManager::get()->getActiveNong(songID);
        if (!res) {
            return;
        }
        SongInfo active = res.value();
        if (m_fields->menu != nullptr) {
            m_fields->menu->removeFromParent();
        }
		auto menu = CCMenu::create();
		menu->setID("song-name-menu");
		auto label = CCLabelBMFont::create(active.songName.c_str(), "bigFont.fnt");
        if (!m_isMusicLibrary) {
		    label->limitLabelWidth(220.f, 0.8f, 0.1f);
        } else {
		    label->limitLabelWidth(130.f, 0.4f, 0.1f);
        }
		auto songNameMenuLabel = CCMenuItemSpriteExtra::create(
			label,
			this,
            menu_selector(JBSongWidget::addNongLayer)
		);
		songNameMenuLabel->setTag(songID);
// 		// I am not even gonna try and understand why this works, but this places the label perfectly in the menu
		auto labelScale = label->getScale();
		songNameMenuLabel->setID("song-name-label");
		songNameMenuLabel->setPosition(ccp(0.f, 0.f));
		songNameMenuLabel->setAnchorPoint(ccp(0.f, 0.5f));
        m_fields->songNameLabel = songNameMenuLabel;
		menu->addChild(songNameMenuLabel);
		menu->setContentSize(ccp(label->getContentSize().width * labelScale, 25.f));
        if (!m_isMusicLibrary) {
		    menu->setPosition(ccp(-140.f, 27.5f));
        } else {
		    menu->setPosition(ccp(-150.f, 9.f));
        }
		songNameMenuLabel->setContentSize({ label->getContentSize().width * labelScale, labelScale * 30 });
        m_fields->menu = menu;
		this->addChild(menu);
        if (m_songs.size() == 0 && m_sfx.size() == 0 && !m_isMusicLibrary) {
            if (m_fields->sizeIdLabel != nullptr) {
                m_fields->sizeIdLabel->removeFromParent();
            }
            auto data = NongManager::get()->getNongs(songID).value();

    		if (!fs::exists(active.path) && active.path == data.defaultPath) {
                m_songIDLabel->setVisible(true);
    			return;
                geode::cocos::handleTouchPriority(this);
    		} else if (m_songIDLabel) {
    			m_songIDLabel->setVisible(false);
    		}

    		std::string sizeText;
    		if (fs::exists(active.path)) {
    			sizeText = NongManager::get()->getFormattedSize(active);
    		} else {
    			sizeText = "NA";
    		}
    		std::string labelText;
    		if (active.path == data.defaultPath) {
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

            auto label = CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
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
        if (m_songs.size() > 1 && !m_fields->fetchedAssetInfo) {
            this->getMultiAssetSongInfo();
            if (!m_fields->fetchedAssetInfo) {
                return;
            }
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
                if (!NongManager::get()->getNongs(kv.first).has_value()) {
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
        if (Mod::get()->getSavedValue("show-tutorial", true) && GameManager::get()->m_levelEditorLayer == nullptr) {
            auto popup = FLAlertLayer::create("Jukebox", "Thank you for using <co>Jukebox</c>! To begin swapping songs, click on the <cr>song name</c>!", "Ok");
            Mod::get()->setSavedValue("show-tutorial", false);
            popup->m_scene = this;
            popup->show();
        }
        return true;
    }
};
