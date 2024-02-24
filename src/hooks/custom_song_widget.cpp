#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/modify/CustomSongWidget.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <sstream>

#include "../types/song_info.hpp"
#include "../managers/nong_manager.hpp"
#include "../ui/nong_dropdown_layer.hpp"

using namespace geode::prelude;

class $modify(JBSongWidget, CustomSongWidget) {
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

    bool init(
        SongInfoObject* songInfo,
        CustomSongDelegate* songDelegate,
        bool showSongSelect,
        bool showPlayMusic,
        bool showDownload,
        bool isRobtopSong,
        bool unk,
        bool isMusicLibrary
    ) {
        if (!CustomSongWidget::init(songInfo, songDelegate, showSongSelect, showPlayMusic, showDownload, isRobtopSong, unk, isMusicLibrary)) {
            return false;
        }
        if (isRobtopSong) {
            return true;
        }
        // log::info("songselect {}, playmusic {}, download {}, robtop {}, unk {}, musiclib {}", m_showSelectSongBtn, m_showPlayMusicBtn, m_showDownloadBtn, m_isRobtopSong, m_unkBool1, m_isMusicLibrary);
        
        m_songLabel->setVisible(false);
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
        NongManager::get()->getMultiAssetSizes(m_fields->songIds, m_fields->sfxIds, [this](std::string result) {
            std::stringstream ss;
            ss << "Songs: " << m_songs.size() << "  SFX: " << m_sfx.size() << "  Size: " << result;
            m_songIDLabel->setString(ss.str().c_str());
        });
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

    void updateSongObject(SongInfoObject* obj) {
        // log::info("update song object");
        // log::info("{}, {}, {}, {}, {}", obj->m_songName, obj->m_artistName, obj->m_songUrl, obj->m_isUnkownSong, obj->m_songID);
        // log::info("songselect {}, playmusic {}, download {}, robtop {}, unk {}, musiclib {}", m_showSelectSongBtn, m_showPlayMusicBtn, m_showDownloadBtn, m_isRobtopSong, m_unkBool1, m_isMusicLibrary);

        CustomSongWidget::updateSongObject(obj);
        if (obj->m_songID == 0) {
            this->restoreUI();
            return;
        }
        if (m_showSelectSongBtn && obj->m_artistName.empty() && obj->m_songUrl.empty()) {
            this->restoreUI();
            return;
        }
        if (m_isRobtopSong) {
            return;
        }
        m_songLabel->setVisible(false);
        if (obj->m_songUrl.empty() && obj->m_artistName.empty()) {
            // we have an invalid songID
            if (!NongManager::get()->getNongs(obj->m_songID).has_value()) {
                NongManager::get()->createUnknownDefault(obj->m_songID);
            }
        }
        auto result = NongManager::get()->getNongs(obj->m_songID);
        if (!result.has_value()) {
            NongManager::get()->createDefault(obj->m_songID);
            result = NongManager::get()->getNongs(obj->m_songID);
            if (!result.has_value()) {
                return;
            }
        }
        auto active = NongManager::get()->getActiveNong(obj->m_songID).value();
        if (!result.value().defaultValid && active.path == result.value().defaultPath) {
            NongManager::get()->fixDefault(obj);
            m_sliderGroove->setVisible(false);
        }
        if (!result.value().defaultValid && active.path == result.value().defaultPath) {
            NongManager::get()->prepareCorrectDefault(obj->m_songID);
            this->template addEventListener<GetSongInfoEventFilter>(
                [this](SongInfoObject* obj) {
                    this->createSongLabels();
                    return ListenerResult::Propagate;
                },
                obj->m_songID
            );
        }
        m_fields->nongs = result.value();
        this->createSongLabels();
        auto data = NongManager::get()->getNongs(obj->m_songID).value();
        if (active.path != data.defaultPath) {
            m_deleteBtn->setVisible(false);
        }
    }

    void updateSongInfo() {
        CustomSongWidget::updateSongInfo();
        if (m_isRobtopSong || m_songInfoObject == nullptr || m_songInfoObject->m_songID == 0) {
            return;
        }
        if (!m_fields->fetchedAssetInfo && m_songs.size() != 0) {
            this->getMultiAssetSongInfo();
        }
    }

    void getMultiAssetSongInfo() {
        bool allDownloaded = true;
        for (auto const& kv : m_songs) {
            auto result = NongManager::get()->getNongs(kv.first);
            if (!result.has_value()) {
                NongManager::get()->createDefault(kv.first);
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
        auto active = NongManager::get()->getActiveNong(songID).value();
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
    			labelText = "SongID: " + std::to_string(songID) + "  Size: " + sizeText;
    		} else {
    			labelText = "SongID: NONG  Size: " + sizeText;
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
        if (m_songs.size() > 1) {
            for (auto const& kv : m_songs) {
                if (!NongManager::get()->getNongs(kv.first).has_value()) {
                    return;
                }
                ids.push_back(kv.first);
            }
        } else {
            ids.push_back(m_songInfoObject->m_songID);
        }
		auto layer = NongDropdownLayer::create(ids, this, m_songInfoObject->m_songID);
        layer->m_noElasticity = true;
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
