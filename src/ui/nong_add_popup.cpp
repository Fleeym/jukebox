#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/cocos/CCDirector.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/MiniFunction.hpp>
#include <Geode/utils/Result.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/ui/TextInput.hpp>
#include <ccTypes.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>

#include <filesystem>
#include <fmt/core.h>
#include <fmod.hpp>
#include <fmod_common.h>
#include <optional>
#include <sstream>
#include <system_error>

#include "nong_add_popup.hpp"
#include "../random_string.hpp"

std::optional<std::string> parseFromFMODTag(const FMOD_TAG& tag) {
    std::string ret = "";
    #ifdef GEODE_IS_WINDOWS
    if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF16) {
        return geode::utils::string::wideToUtf8(
            reinterpret_cast<const wchar_t*>(tag.data)
        );
    } else if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF16BE) {
        // I'm a big endian hater, whachu gonna do?
        return std::nullopt;
    }
    #endif

    return std::string(
        reinterpret_cast<const char*>(tag.data),
        tag.datalen
    );
}

namespace jukebox {

bool NongAddPopup::setup(NongDropdownLayer* parent) {
    this->setTitle("Add Song");
    m_parentPopup = parent;

    auto spr = CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    spr->setScale(0.7f);

    m_addSongButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Add"),
        this,
        menu_selector(NongAddPopup::addSong)
    );
    m_addSongMenu = CCMenu::create();
    m_addSongMenu->setID("add-song-menu");
    m_addSongButton->setID("add-song-button");
    m_addSongMenu->addChild(this->m_addSongButton);
    m_addSongMenu->setPosition(m_mainLayer->getContentSize().width / 2, 10.f);
    m_addSongMenu->setAnchorPoint({0.5f, 0.0f});
    m_addSongMenu->setContentSize(m_addSongButton->getContentSize());
    m_addSongMenu->setLayout(ColumnLayout::create());

    m_mainLayer->addChild(this->m_addSongMenu);
    this->createInputs();

    return true;
}

NongAddPopup* NongAddPopup::create(NongDropdownLayer* parent) {
    auto ret = new NongAddPopup();
    auto size = ret->getPopupSize();
    if (ret && ret->initAnchored(size.width, size.height, parent)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCSize NongAddPopup::getPopupSize() {
    return { 320.f, 240.f };
}

void NongAddPopup::openFile(CCObject* target) {
    #ifdef GEODE_IS_WINDOWS
    file::FilePickOptions::Filter filter = {
        .description = "Songs",
        .files = { "*.mp3", "*.flac", "*.wav", "*.ogg" }
    };
    #else
    file::FilePickOptions::Filter filter = {};
    #endif
    file::FilePickOptions options = {
        std::nullopt,
        {filter}
    };

    auto callback = [this](Result<std::filesystem::path> result) {
    };
    auto failedCallback = []() {
        FLAlertLayer::create("Error", "Failed to open file", "Ok")->show();
    };

    m_pickListener.bind(this, &NongAddPopup::onFileOpen);
    m_pickListener.setFilter(file::pick(file::PickMode::OpenFile, options));
}

void NongAddPopup::onFileOpen(Task<Result<std::filesystem::path>>::Event* event) {
    if (event->isCancelled()) {
        FLAlertLayer::create(
            "Error",
            "Failed to open file (Task Cancelled)",
            "Ok"
        )->show();
        return;
    }
    if (auto result = event->getValue()) {
        if (result->isErr()) {
            FLAlertLayer::create(
                "Error",
                fmt::format("Failed to open file. Error: {}", result->err()),
                "Ok"
            )->show();
            return;
        }
        auto path = result->unwrap();
        #ifdef GEODE_IS_WINDOWS
        auto strPath = geode::utils::string::wideToUtf8(path.c_str());
        #else
        std::string strPath = path.c_str();
        #endif

        std::string extension = path.extension().string();

        if (
            extension != ".mp3"
            && extension != ".ogg"
            && extension != ".wav"
            && extension != ".flac"
        ) {
            FLAlertLayer::create("Error", "The selected file must be one of the following: <cb>mp3, wav, flac, ogg</c>.", "Ok")->show();
            return;
        }

        if (Mod::get()->getSettingValue<bool>("autocomplete-metadata")) {
            auto meta = this->tryParseMetadata(path);
            if (meta && (meta->artist.has_value() || meta->name.has_value())) {
                auto artistName = m_artistNameInput->getString();
                auto songName = m_songNameInput->getString();

                if (artistName.size() > 0 || songName.size() > 0) {
                    // We should ask before replacing stuff
                    std::stringstream ss;

                    ss << "Found metadata for the imported song: ";
                    if (meta->name.has_value()) {
                        ss << fmt::format("Name: \"{}\". ", meta->name.value());
                    }
                    if (meta->artist.has_value()) {
                        ss << fmt::format("Artist: \"{}\". ", meta->artist.value());
                    }

                    ss << "Do you want to set those values for the song?";

                    createQuickPopup(
                        "Metadata found",
                        ss.str(),
                        "No",
                        "Yes",
                        [this, meta](auto, bool btn2) {
                            if (!btn2) {
                                return;
                            }
                            if (meta->artist.has_value()) {
                                m_artistNameInput->setString(meta->artist.value());
                            }
                            if (meta->name.has_value()) {
                                m_songNameInput->setString(meta->name.value());
                            }
                        }
                    );
                } else {
                    if (meta->artist.has_value()) {
                        m_artistNameInput->setString(meta->artist.value());
                    }
                    if (meta->name.has_value()) {
                        m_songNameInput->setString(meta->name.value());
                    }
                }
            }
        }

        m_localLinkInput->setString(strPath);
    }
}

void NongAddPopup::setSongType(NongAddPopupSongType type) {
    m_songType = type;

    m_switchLocalButtonSprite->updateBGImage(type == NongAddPopupSongType::local ? "GJ_button_01.png" : "GJ_button_04.png");
    m_switchYTButtonSprite->updateBGImage(type == NongAddPopupSongType::yt ? "GJ_button_01.png" : "GJ_button_04.png");
    m_switchHostedButtonSprite->updateBGImage(type == NongAddPopupSongType::hosted ? "GJ_button_01.png" : "GJ_button_04.png");

    m_specificInputsMenu->removeAllChildren();
    if (type == NongAddPopupSongType::local) {
        m_specificInputsMenu->addChild(m_localMenu);
    } else if (type == NongAddPopupSongType::yt) {
        m_specificInputsMenu->addChild(m_ytLinkInput);
    } else if (type == NongAddPopupSongType::hosted) {
        m_specificInputsMenu->addChild(m_hostedLinkInput);
    }
    m_specificInputsMenu->updateLayout();
}

void NongAddPopup::onSwitchToLocal(CCObject*) {
    this->setSongType(NongAddPopupSongType::local);
}

void NongAddPopup::onSwitchToYT(CCObject*) {
    this->setSongType(NongAddPopupSongType::yt);
}

void NongAddPopup::onSwitchToHosted(CCObject*) {
    this->setSongType(NongAddPopupSongType::hosted);
}


void NongAddPopup::createInputs() {
    auto inputParent = CCNode::create();
    inputParent->setID("input-parent");
    auto songInput = TextInput::create(350.f, "Song name*", "bigFont.fnt");
    songInput->setID("song-name-input");
    songInput->setCommonFilter(CommonFilter::Any);
    songInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    songInput->getInputNode()->setMaxLabelScale(0.7f);
    songInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_songNameInput = songInput;

    auto artistInput = TextInput::create(350.f, "Artist name*", "bigFont.fnt");
    artistInput->setID("artist-name-input");
    artistInput->setCommonFilter(CommonFilter::Any);
    artistInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    artistInput->getInputNode()->setMaxLabelScale(0.7f);
    artistInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_artistNameInput = artistInput;

    auto levelNameInput = TextInput::create(350.f, "Level name", "bigFont.fnt");
    levelNameInput->setID("artist-name-input");
    levelNameInput->setCommonFilter(CommonFilter::Any);
    levelNameInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    levelNameInput->getInputNode()->setMaxLabelScale(0.7f);
    levelNameInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_levelNameInput = levelNameInput;

    auto startOffsetInput = TextInput::create(350.f, "Start offset (ms)", "bigFont.fnt");
    startOffsetInput->setID("start-offset-input");
    startOffsetInput->setCommonFilter(CommonFilter::Int);
    startOffsetInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    startOffsetInput->getInputNode()->setMaxLabelScale(0.7f);
    startOffsetInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_startOffsetInput = startOffsetInput;


    m_switchLocalButtonSprite = ButtonSprite::create("Local", "bigFont.fnt", "GJ_button_01.png");
    m_switchYTButtonSprite = ButtonSprite::create("YouTube", "bigFont.fnt", "GJ_button_01.png");
    m_switchHostedButtonSprite = ButtonSprite::create("Hosted", "bigFont.fnt", "GJ_button_01.png");
    m_switchLocalButton = CCMenuItemSpriteExtra::create(
        m_switchLocalButtonSprite,
        this,
        menu_selector(NongAddPopup::onSwitchToLocal)
    );
    m_switchYTButton = CCMenuItemSpriteExtra::create(
        m_switchYTButtonSprite,
        this,
        menu_selector(NongAddPopup::onSwitchToYT)
    );
    m_switchHostedButton = CCMenuItemSpriteExtra::create(
        m_switchHostedButtonSprite,
        this,
        menu_selector(NongAddPopup::onSwitchToHosted)
    );
    m_switchLocalMenu = CCMenu::create();
    m_switchYTMenu = CCMenu::create();
    m_switchHostedMenu = CCMenu::create();
    m_switchLocalMenu->setContentSize({ m_switchLocalButton->getContentWidth(), 30.f });
    m_switchLocalMenu->addChildAtPosition(m_switchLocalButton, Anchor::Center);
    m_switchYTMenu->setContentSize({ m_switchYTButton->getContentWidth(), 30.f });
    m_switchYTMenu->addChildAtPosition(m_switchYTButton, Anchor::Center);
    m_switchHostedMenu->setContentSize({ m_switchHostedButton->getContentWidth(), 30.f });
    m_switchHostedMenu->addChildAtPosition(m_switchHostedButton, Anchor::Center);

    m_switchButtonsMenu = CCMenu::create();
    m_switchButtonsMenu->addChild(m_switchLocalMenu);
    m_switchButtonsMenu->addChild(m_switchYTMenu);
    m_switchButtonsMenu->addChild(m_switchHostedMenu);
    m_switchButtonsMenu->setContentSize({ 280.f, 50.f });
    m_switchButtonsMenu->setLayout(
        RowLayout::create()
            ->setAxisAlignment(AxisAlignment::Center)
    );

    m_specificInputsMenu = CCMenu::create();
    m_specificInputsMenu->setContentSize({ 350.f, 70.f });
    m_specificInputsMenu->setLayout(
        ColumnLayout::create()
            ->setAxisAlignment(AxisAlignment::End)
    );

    m_localMenu = CCMenu::create();
    m_localMenu->setContentSize(m_specificInputsMenu->getContentSize());
    m_localMenu->setLayout(
        RowLayout::create()
            ->setAxisAlignment(AxisAlignment::End)
    );
    auto spr = CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    spr->setScale(0.7f);
    m_localSongButton = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongAddPopup::openFile)
    );
    m_localSongButton->setID("select-file-button");
    m_localSongButtonMenu = CCMenu::create();
    m_localSongButtonMenu->setID("select-file-menu");
    m_localSongButtonMenu->addChild(this->m_localSongButton);
    m_localSongButtonMenu->setContentSize(m_localSongButton->getScaledContentSize());
    m_localSongButtonMenu->setAnchorPoint({1.0f, 0.0f});
    m_localSongButtonMenu->setPosition(m_mainLayer->getContentSize().width - 10.f, 10.f);
    m_localSongButtonMenu->setLayout(ColumnLayout::create());
    m_localLinkInput = TextInput::create(350.f, "Local Path", "bigFont.fnt");
    m_localLinkInput->setID("local-link-input");
    m_localLinkInput->setCommonFilter(CommonFilter::Any);
    m_localLinkInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    m_localLinkInput->getInputNode()->setMaxLabelScale(0.7f);
    m_localLinkInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_localMenu->addChild(m_localLinkInput);
    m_localMenu->addChild(m_localSongButtonMenu);
    m_localMenu->updateLayout();
    m_localMenu->retain();

    m_ytLinkInput = TextInput::create(350.f, "YouTube Link", "bigFont.fnt");
    m_ytLinkInput->setID("yt-link-input");
    m_ytLinkInput->setCommonFilter(CommonFilter::Any);
    m_ytLinkInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    m_ytLinkInput->getInputNode()->setMaxLabelScale(0.7f);
    m_ytLinkInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_ytLinkInput->retain();

    m_hostedLinkInput = TextInput::create(350.f, "Hosted Link", "bigFont.fnt");
    m_hostedLinkInput->setID("hosted-link-input");
    m_hostedLinkInput->setCommonFilter(CommonFilter::Any);
    m_hostedLinkInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    m_hostedLinkInput->getInputNode()->setMaxLabelScale(0.7f);
    m_hostedLinkInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_hostedLinkInput->retain();

    float inputHeight = songInput->getContentSize().height;

    inputParent->addChild(songInput);
    inputParent->addChild(artistInput);
    inputParent->addChild(levelNameInput);
    inputParent->addChild(startOffsetInput);
    inputParent->addChild(m_switchButtonsMenu);
    inputParent->addChild(m_specificInputsMenu);
    inputParent->setContentSize({ 250.f, 150.f });
    inputParent->setAnchorPoint({ 0.5f, 1.0f });
    inputParent->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
    );
    m_mainLayer->addChildAtPosition(
        inputParent,
        Anchor::Top,
        { 0.0f, -40.f }
    );

    this->setSongType(NongAddPopupSongType::local);
}

void NongAddPopup::addSong(CCObject* target) {
    auto artistName = std::string(m_artistNameInput->getString());
    auto songName = std::string(m_songNameInput->getString());
    std::string levelName = m_levelNameInput->getString();
    auto startOffsetStr = m_startOffsetInput->getString();
    fs::path songPath = m_localLinkInput->getString();
    #ifdef GEODE_IS_WINDOWS
    if (wcslen(songPath.c_str()) == 0) {
    #else
    if (strlen(m_songPath.c_str()) == 0) {
    #endif
        FLAlertLayer::create("Error", "No file selected.", "Ok")->show();
        return;
    }
    if (!fs::exists(songPath)) {
        std::stringstream ss;
        ss << "The selected file (" << songPath.string() << ") does not exist.";
        FLAlertLayer::create("Error", ss.str().c_str(), "Ok")->show();
        return;
    }

    if (fs::is_directory(songPath)) {
        FLAlertLayer::create("Error", "You selected a directory.", "Ok")->show();
        return;
    }

    std::string extension = songPath.extension().string();

    if (
        extension != ".mp3"
        && extension != ".ogg"
        && extension != ".wav"
        && extension != ".flac"
    ) {
        FLAlertLayer::create("Error", "The selected file must be one of the following: <cb>mp3, wav, flac, ogg</c>.", "Ok")->show();
        return;
    }

    if (songName == "") {
        FLAlertLayer::create("Error", "Song name is empty.", "Ok")->show();
        return;
    }

    if (artistName == "") {
        FLAlertLayer::create("Error", "Artist name is empty.", "Ok")->show();
        return;
    }

    int startOffset = 0;

    if (startOffsetStr != "") {
        startOffset = std::stoi(startOffsetStr);
    }

    auto unique = jukebox::random_string(16);
    auto destination = Mod::get()->getSaveDir() / "nongs";
    if (!fs::exists(destination)) {
        fs::create_directory(destination);
    }
    unique += songPath.extension().string();
    destination = destination / unique;
    bool result;
    std::error_code error_code;
    result = fs::copy_file(songPath, destination, error_code);
    if (error_code) {
        std::stringstream ss;
        ss << "Failed to save song. Please try again! Error category: " << error_code.category().name() << ", message: " << error_code.category().message(error_code.value());
        FLAlertLayer::create("Error", ss.str().c_str(), "Ok")->show();
        return;
    }
    if (!result) {
        FLAlertLayer::create("Error", "Failed to save song. Please try again!", "Ok")->show();
        return;
    }

    SongInfo song = {
        .path = destination,
        .songName = songName,
        .authorName = artistName,
        .songUrl = "local",
        .levelName = levelName,
        .startOffset = startOffset,
    };

    m_parentPopup->addSong(song);
    this->onClose(this);
}

void NongAddPopup::onClose(CCObject* target) {
    m_localMenu->release();
    m_ytLinkInput->release();
    m_hostedLinkInput->release();
    this->Popup::onClose(target);
}

std::optional<NongAddPopup::ParsedMetadata> NongAddPopup::tryParseMetadata(
    std::filesystem::path path
) {
    // Thanks to undefined06855 for most of this stuff
    // https://github.com/undefined06855/EditorMusic/blob/main/src/AudioManager.cpp

    NongAddPopup::ParsedMetadata ret {};
    FMOD::Sound* sound;
    FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;

    system->createSound(
        path.string().c_str(),
        FMOD_LOOP_NORMAL,
        nullptr,
        &sound
    );

    if (!sound) {
        return std::nullopt;
    }

    FMOD_TAG nameTag = {};
    FMOD_TAG artistTag = {};
    FMOD_RESULT nameResult;
    FMOD_RESULT artistResult;

    const std::string extension = path.extension().string();
    if (extension == ".mp3") {
        nameResult = sound->getTag("TIT2", 0, &nameTag);
        artistResult = sound->getTag("TPE1", 0, &artistTag);
    } else if (extension == ".ogg" || extension == ".flac") {
        nameResult = sound->getTag("TITLE", 0, &nameTag);
        artistResult = sound->getTag("ARTIST", 0, &artistTag);
    } else if (extension == ".wav") {
        nameResult = sound->getTag("INAM", 0, &nameTag);
        artistResult = sound->getTag("IART", 0, &artistTag);
    }

    if (nameResult != FMOD_ERR_TAGNOTFOUND) {
        ret.name = parseFromFMODTag(nameTag);
    }
    if (artistResult != FMOD_ERR_TAGNOTFOUND) {
        ret.artist = parseFromFMODTag(artistTag);
    }

    return ret;
}

}
