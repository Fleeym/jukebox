#include <Geode/binding/CustomSongWidget.hpp>
#include <Geode/modify/CustomSongWidget.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/binding/SongInfoObject.hpp>
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
        if (obj->m_songUrl.empty()) {
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
        if (active.path == result.value().defaultPath) {
            NongManager::get()->fixDefault(obj);
            m_sliderBar->setVisible(false);
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

// class $modify(NongSongWidget, CustomSongWidget) {
// 	NongData nongData;
// 	int nongdSong;

// 	bool hasDefaultSong = false;
// 	bool firstRun = true;

// 	bool init(SongInfoObject* songInfo, LevelSettingsObject* levelSettings, bool p2, bool p3, bool p4, bool hasDefaultSong, bool hideBackground) {
// 		if (!CustomSongWidget::init(songInfo, levelSettings, p2, p3, p4, hasDefaultSong, hideBackground)) return false;

// 		if (!songInfo) {
// 			return true;
// 		}

// 		m_fields->firstRun = false;

// 		if (hasDefaultSong) {
// 			m_fields->hasDefaultSong = true;
// 			this->updateSongObject(m_songInfo);
// 			return true;
// 		}

// 		auto songNameLabel = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("song-name-label"));
// 		songNameLabel->setVisible(false);

// 		auto idAndSizeLabel = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("id-and-size-label"));
// 		idAndSizeLabel->setVisible(false);
// 		auto newLabel = CCLabelBMFont::create("new", "bigFont.fnt");
// 		newLabel->setID("nongd-id-and-size-label");
// 		newLabel->setPosition(ccp(0.f, -32.f));
// 		newLabel->setScale(0.4f);
// 		this->addChild(newLabel);

// 		m_fields->nongdSong = songInfo->m_songID;

// 		if (!NongManager::get()->checkIfNongsExist(songInfo->m_songID)) {
// 			auto strPath = std::string(MusicDownloadManager::sharedState()->pathForSong(songInfo->m_songID));

// 			SongInfo defaultSong = {
// 				.path = ghc::filesystem::path(strPath),
// 				.songName = songInfo->m_songName,
// 				.authorName = songInfo->m_artistName,
// 				.songUrl = songInfo->m_songURL,
// 			};

// 			NongManager::get()->createDefaultSongIfNull(defaultSong, songInfo->m_songID);
// 		}

// 		auto invalidSongs = NongManager::get()->validateNongs(songInfo->m_songID);

// 		if (invalidSongs.size() > 0) {
// 			std::string invalidSongList = "";
// 			for (auto &song : invalidSongs) {
// 				invalidSongList += song.songName + ", ";
// 			}

// 			invalidSongList = invalidSongList.substr(0, invalidSongList.size() - 2);
// 			// If anyone asks this was mat's idea
// 			Loader::get()->queueInMainThread([this, invalidSongList]() {
// 				auto alert = FLAlertLayer::create("Invalid NONGs", "The NONGs [<cr>" + invalidSongList + "</c>] have been deleted, because their paths were invalid.", "Ok");
// 				alert->m_scene = this->getParent();
// 				alert->show();
// 			});
// 		}

// 		m_fields->nongData = NongManager::get()->getNongs(m_songInfo->m_songID);
// 		SongInfo nong;
// 		for (auto song : m_fields->nongData.songs) {
// 			if (song.path == m_fields->nongData.active) {
// 				nong = song;
// 			}
// 		}

// 		m_songInfo->m_artistName = nong.authorName;
// 		m_songInfo->m_songName = nong.songName;
// 		this->updateSongObject(m_songInfo);
// 		if (auto found = this->getChildByID("song-name-menu")) {
// 			this->updateSongNameLabel(m_songInfo->m_songName, m_songInfo->m_songID);
// 		} else {
// 			this->addMenuItemLabel(m_songInfo->m_songName, m_songInfo->m_songID);
// 		}
// 		if (nong.path == m_fields->nongData.defaultPath) {
// 			this->updateIDAndSizeLabel(nong, m_songInfo->m_songID);
// 		} else {
// 			this->updateIDAndSizeLabel(nong);
// 		}

// 		return true;
// 	}
		
// 	void addMenuItemLabel(std::string const& text, int songID) {
// 		auto menu = CCMenu::create();
// 		menu->setID("song-name-menu");

// 		auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
// 		label->limitLabelWidth(220.f, 0.8f, 0.1f);
// 		auto info = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
// 		info->setScale(0.5f);
// 		auto songNameMenuLabel = CCMenuItemSpriteExtra::create(
// 			label,
// 			this,
// 			menu_selector(NongSongWidget::addNongLayer)
// 		);
// 		songNameMenuLabel->addChild(info);
// 		songNameMenuLabel->setTag(songID);
// 		// I am not even gonna try and understand why this works, but this places the label perfectly in the menu
// 		auto labelScale = label->getScale();
// 		songNameMenuLabel->setID("song-name-label");
// 		songNameMenuLabel->setPosition(ccp(0.f, 0.f));
// 		songNameMenuLabel->setAnchorPoint(ccp(0.f, 0.5f));
// 		menu->addChild(songNameMenuLabel);
// 		menu->setContentSize(ccp(220.f, 25.f));
// 		menu->setPosition(ccp(-140.f, 27.5f));
// 		auto layout = RowLayout::create();
// 		layout->setAxisAlignment(AxisAlignment::Start);
// 		layout->setAutoScale(false);
// 		songNameMenuLabel->setLayout(layout);
// 		songNameMenuLabel->setContentSize({ 220.f, labelScale * 30 });

// 		this->addChild(menu);
// 	}

// 	void updateSongNameLabel(std::string const& text, int songID) {
// 		auto menu = this->getChildByID("song-name-menu");
// 		auto labelMenuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(menu->getChildByID("song-name-label"));
// 		labelMenuItem->setTag(songID);
// 		auto child = typeinfo_cast<CCLabelBMFont*>(labelMenuItem->getChildren()->objectAtIndex(0));
// 		child->setString(text.c_str());
// 		child->limitLabelWidth(220.f, 0.8f, 0.1f);
// 		auto labelScale = child->getScale();
// 		labelMenuItem->setContentSize({ 220.f, labelScale * 30 });
// 		labelMenuItem->updateLayout();
// 	}

// 	void updateIDAndSizeLabel(SongInfo const& song, int songID = 0) {
// 		auto label = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("nongd-id-and-size-label"));
// 		auto normalLabel = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("id-and-size-label"));
// 		auto defaultPath = m_fields->nongData.defaultPath;

// 		if (!ghc::filesystem::exists(song.path) && song.path == defaultPath) {
// 			label->setVisible(false);
// 			this->getChildByID("id-and-size-label")->setVisible(true);
// 			return;
// 		} else if (normalLabel && normalLabel->isVisible()) {
// 			normalLabel->setVisible(false);
// 			label->setVisible(true);
// 		}

// 		std::string sizeText;
// 		if (ghc::filesystem::exists(song.path)) {
// 			sizeText = NongManager::get()->getFormattedSize(song);
// 		} else {
// 			sizeText = "NA";
// 		}
// 		std::string labelText;
// 		if (songID != 0) {
// 			labelText = "SongID: " + std::to_string(songID) + "  Size: " + sizeText;
// 		} else {
// 			labelText = "SongID: NONG  Size: " + sizeText;
// 		}

// 		if (label) {
// 			label->setString(labelText.c_str());
// 		}
// 	}

// 	void updateSongObject(SongInfoObject* song) {
// 		if (m_fields->firstRun) {
// 			CustomSongWidget::updateSongObject(song);
// 			return;
// 		}

// 		if (m_fields->hasDefaultSong) {
// 			CustomSongWidget::updateSongObject(song);
// 			if (auto found = this->getChildByID("song-name-menu")) {
// 				found->setVisible(false);
// 				this->getChildByID("nongd-id-and-size-label")->setVisible(false);
// 			}
// 			this->getChildByID("id-and-size-label")->setVisible(true);
// 			return;
// 		}

// 		m_fields->nongdSong = song->m_songID;
// 		if (!NongManager::get()->checkIfNongsExist(song->m_songID)) {
// 			auto strPath = std::string(MusicDownloadManager::sharedState()->pathForSong(song->m_songID));

// 			SongInfo defaultSong = {
// 				.path = ghc::filesystem::path(strPath),
// 				.songName = song->m_songName,
// 				.authorName = song->m_artistName,
// 				.songUrl = song->m_songURL,
// 			};

// 			NongManager::get()->createDefaultSongIfNull(defaultSong, song->m_songID);
// 		}
// 		SongInfo active;
// 		auto nongData = NongManager::get()->getNongs(song->m_songID);
// 		for (auto nong : nongData.songs) {
// 			if (nong.path == nongData.active) {
// 				active = nong;
// 				song->m_songName = nong.songName;
// 				song->m_artistName = nong.authorName;
// 				if (nong.songUrl != "local") {
// 					song->m_songURL = nong.songUrl;
// 				}
// 			}
// 		}
// 		CustomSongWidget::updateSongObject(song);
// 		if (auto found = this->getChildByID("song-name-menu")) {
// 			this->updateSongNameLabel(song->m_songName, song->m_songID);
// 		} else {
// 			this->addMenuItemLabel(song->m_songName, song->m_songID);
// 		}
// 		if (active.path == nongData.defaultPath) {
// 			this->updateIDAndSizeLabel(active, song->m_songID);
// 		} else {
// 			this->updateIDAndSizeLabel(active);
// 		}
// 	}

// 	void addNongLayer(CCObject* target) {
// 		auto scene = CCDirector::sharedDirector()->getRunningScene();
// 		auto layer = NongDropdownLayer::create(m_fields->nongdSong, this);
// 		// based robtroll
// 		layer->setZOrder(106);
// 		scene->addChild(layer);
// 		layer->showLayer(false);
// 	}

// };