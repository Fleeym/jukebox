#include "nong_add_popup.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <system_error>
#include <utility>

#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <ccTypes.h>
#include <fmod_common.h>
#include <fmt/core.h>
#include <fmod.hpp>
#include "Geode/binding/ButtonSprite.hpp"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/binding/FLAlertLayer.hpp"
#include "Geode/binding/FLAlertLayerProtocol.hpp"
#include "Geode/binding/FMODAudioEngine.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/base_nodes/Layout.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Log.hpp"
#include "Geode/loader/Mod.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/MiniFunction.hpp"
#include "Geode/utils/Result.hpp"
#include "Geode/utils/Task.hpp"
#include "Geode/utils/file.hpp"
#include "Geode/utils/string.hpp"

#include "../managers/index_manager.hpp"
#include "../utils/random_string.hpp"
#include "index_choose_popup.hpp"

std::optional<std::string> parseFromFMODTag(const FMOD_TAG& tag) {
    std::string ret = "";
#ifdef GEODE_IS_WINDOWS
    if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF16) {
        return geode::utils::string::wideToUtf8(
            reinterpret_cast<const wchar_t*>(tag.data));
    } else if (tag.datatype == FMOD_TAGDATATYPE_STRING_UTF16BE) {
        // I'm a big endian hater, whachu gonna do?
        return std::nullopt;
    }
#endif

    return std::string(reinterpret_cast<const char*>(tag.data), tag.datalen);
}

class IndexDisclaimerPopup : public FLAlertLayer, public FLAlertLayerProtocol {
protected:
    MiniFunction<void(FLAlertLayer*, bool)> m_selected;

    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        m_selected(layer, btn2);
    }

public:
    static IndexDisclaimerPopup* create(
        char const* title, std::string const& content, char const* btn1,
        char const* btn2, float width,
        MiniFunction<void(FLAlertLayer*, bool)> selected) {
        auto inst = new IndexDisclaimerPopup;
        inst->m_selected = selected;
        if (inst->init(inst, title, content, btn1, btn2, width, true, .0f,
                       1.0f)) {
            inst->autorelease();
            return inst;
        }

        delete inst;
        return nullptr;
    }
};

namespace jukebox {

bool NongAddPopup::setup(NongDropdownLayer* parent, int songID,
                         std::optional<Song*> replacedNong) {
    this->setTitle("Add Song");
    m_parentPopup = parent;
    m_songID = songID;
    m_replacedNong = replacedNong;

    auto spr =
        CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    spr->setScale(0.7f);

    auto buttonsContainer = CCMenu::create();

    m_addSongButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create(m_replacedNong.has_value() ? "Edit" : "Add"), this,
        menu_selector(NongAddPopup::addSong));
    m_addSongMenu = CCMenu::create();
    m_addSongMenu->setID("add-song-menu");
    m_addSongButton->setID("add-song-button");
    m_addSongMenu->addChild(this->m_addSongButton);
    m_addSongMenu->setAnchorPoint({0.5f, 0.0f});
    m_addSongMenu->setContentSize(m_addSongButton->getContentSize());
    m_addSongMenu->setContentHeight(20.f);
    m_addSongMenu->setLayout(ColumnLayout::create());

    buttonsContainer->addChild(m_addSongMenu);

    if (m_replacedNong.has_value()) {
        {
            std::vector<std::string> indexIDs;

            for (const std::pair<const std::string,
                                 std::unique_ptr<IndexMetadata>>& pair :
                 IndexManager::get()->m_loadedIndexes) {
                if (!pair.second->m_features.m_submit.has_value()) {
                    continue;
                }
                IndexMetadata::Features::Submit& submit =
                    pair.second->m_features.m_submit.value();

                bool shouldSubmit = false;

                switch (m_replacedNong.value()->type()) {
                    case NongType::LOCAL:
                        shouldSubmit = submit.m_supportedSongTypes.at(
                            IndexMetadata::Features::SupportedSongType::local);
                        break;
                    case NongType::YOUTUBE:
                        shouldSubmit = submit.m_supportedSongTypes.at(
                            IndexMetadata::Features::SupportedSongType::
                                youtube);
                        break;
                    case NongType::HOSTED:
                        shouldSubmit = submit.m_supportedSongTypes.at(
                            IndexMetadata::Features::SupportedSongType::hosted);
                        break;
                }

                if (!shouldSubmit) {
                    continue;
                }

                indexIDs.push_back(pair.first);
            }

            std::stable_sort(
                indexIDs.begin(), indexIDs.end(),
                [](const std::string& a, const std::string& b) {
                    // Ensure "song-file-hub-index" comes first
                    if (a == "song-file-hub-index") {
                        return true;
                    }
                    if (b == "song-file-hub-index") {
                        return false;
                    }
                    return false;  // maintain relative order for other strings
                });

            m_publishableIndexes = std::move(indexIDs);
        }

        if (!m_publishableIndexes.empty()) {
            auto publishSongButton = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Publish"), this,
                menu_selector(NongAddPopup::onPublish));
            CCMenu* publishSongMenu = CCMenu::create();
            publishSongMenu->setID("publish-song-menu");
            publishSongButton->setID("publish-song-button");
            publishSongMenu->addChild(publishSongButton);
            publishSongMenu->setAnchorPoint({0.5f, 0.0f});
            publishSongMenu->setContentSize(
                publishSongButton->getContentSize());
            publishSongMenu->setContentHeight(20.f);
            publishSongMenu->setLayout(ColumnLayout::create());

            buttonsContainer->addChild(publishSongMenu);
        }
    }

    buttonsContainer->setLayout(
        RowLayout::create()->setAxisAlignment(AxisAlignment::Center));
    buttonsContainer->setContentWidth(m_mainLayer->getContentWidth());
    buttonsContainer->updateLayout();
    m_mainLayer->addChildAtPosition(buttonsContainer, Anchor::Bottom,
                                    {0.f, 25.f});

    this->createInputs();

    if (m_replacedNong.has_value()) {
        Song* replacedNong = m_replacedNong.value();
        switch (replacedNong->type()) {
            case NongType::LOCAL:
                this->setSongType(NongAddPopupSongType::local);
                m_localLinkInput->setString(
                    replacedNong->path().value().string());
                break;
            case NongType::YOUTUBE:
                this->setSongType(NongAddPopupSongType::yt);
                m_ytLinkInput->setString(
                    static_cast<YTSong*>(replacedNong)->youtubeID());
                break;
            case NongType::HOSTED:
                this->setSongType(NongAddPopupSongType::hosted);
                m_ytLinkInput->setString(
                    static_cast<HostedSong*>(replacedNong)->url());
                break;
        }

        m_songNameInput->setString(replacedNong->metadata()->name);
        m_artistNameInput->setString(replacedNong->metadata()->artist);
        if (replacedNong->metadata()->level.has_value()) {
            m_levelNameInput->setString(
                replacedNong->metadata()->level.value());
        }
        m_startOffsetInput->setString(
            std::to_string(replacedNong->metadata()->startOffset));
    }

    return true;
}

NongAddPopup* NongAddPopup::create(NongDropdownLayer* parent, int songID,
                                   std::optional<Song*> replacedNong) {
    auto ret = new NongAddPopup();
    auto size = ret->getPopupSize();
    if (ret && ret->initAnchored(size.width, size.height, parent, songID,
                                 std::move(replacedNong))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCSize NongAddPopup::getPopupSize() { return {320.f, 240.f}; }

void NongAddPopup::openFile(CCObject* target) {
#ifdef GEODE_IS_WINDOWS
    file::FilePickOptions::Filter filter = {
        .description = "Songs", .files = {"*.mp3", "*.flac", "*.wav", "*.ogg"}};
#else
    file::FilePickOptions::Filter filter = {};
#endif
    file::FilePickOptions options = {std::nullopt, {filter}};

    auto callback = [this](Result<std::filesystem::path> result) {};
    auto failedCallback = []() {
        FLAlertLayer::create("Error", "Failed to open file", "Ok")->show();
    };

    m_pickListener.bind(this, &NongAddPopup::onFileOpen);
    m_pickListener.setFilter(file::pick(file::PickMode::OpenFile, options));
}

void NongAddPopup::onFileOpen(
    Task<Result<std::filesystem::path>>::Event* event) {
    if (event->isCancelled()) {
        FLAlertLayer::create("Error", "Failed to open file (Task Cancelled)",
                             "Ok")
            ->show();
        return;
    }
    if (auto result = event->getValue()) {
        if (result->isErr()) {
            FLAlertLayer::create(
                "Error",
                fmt::format("Failed to open file. Error: {}", result->err()),
                "Ok")
                ->show();
            return;
        }
        auto path = result->unwrap();
#ifdef GEODE_IS_WINDOWS
        auto strPath = geode::utils::string::wideToUtf8(path.c_str());
#else
        std::string strPath = path.c_str();
#endif

        std::string extension = path.extension().string();

        if (extension != ".mp3" && extension != ".ogg" && extension != ".wav" &&
            extension != ".flac") {
            FLAlertLayer::create("Error",
                                 "The selected file must be one of the "
                                 "following: <cb>mp3, wav, flac, ogg</c>.",
                                 "Ok")
                ->show();
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
                        ss << fmt::format("Artist: \"{}\". ",
                                          meta->artist.value());
                    }

                    ss << "Do you want to set those values for the song?";

                    createQuickPopup(
                        "Metadata found", ss.str(), "No", "Yes",
                        [this, meta](auto, bool btn2) {
                            if (!btn2) {
                                return;
                            }
                            if (meta->artist.has_value()) {
                                m_artistNameInput->setString(
                                    meta->artist.value());
                            }
                            if (meta->name.has_value()) {
                                m_songNameInput->setString(meta->name.value());
                            }
                        });
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

    m_switchLocalButtonSprite->updateBGImage(type == NongAddPopupSongType::local
                                                 ? "GJ_button_01.png"
                                                 : "GJ_button_04.png");
    m_switchYTButtonSprite->updateBGImage(type == NongAddPopupSongType::yt
                                              ? "GJ_button_01.png"
                                              : "GJ_button_04.png");
    m_switchHostedButtonSprite->updateBGImage(
        type == NongAddPopupSongType::hosted ? "GJ_button_01.png"
                                             : "GJ_button_04.png");

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
    songInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B{108, 153, 216});
    songInput->getInputNode()->setMaxLabelScale(0.7f);
    songInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_songNameInput = songInput;

    auto artistInput = TextInput::create(350.f, "Artist name*", "bigFont.fnt");
    artistInput->setID("artist-name-input");
    artistInput->setCommonFilter(CommonFilter::Any);
    artistInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B{108, 153, 216});
    artistInput->getInputNode()->setMaxLabelScale(0.7f);
    artistInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_artistNameInput = artistInput;

    auto levelNameInput = TextInput::create(350.f, "Level name", "bigFont.fnt");
    levelNameInput->setID("artist-name-input");
    levelNameInput->setCommonFilter(CommonFilter::Any);
    levelNameInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B{108, 153, 216});
    levelNameInput->getInputNode()->setMaxLabelScale(0.7f);
    levelNameInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_levelNameInput = levelNameInput;

    auto startOffsetInput =
        TextInput::create(350.f, "Start offset (ms)", "bigFont.fnt");
    startOffsetInput->setID("start-offset-input");
    startOffsetInput->setCommonFilter(CommonFilter::Int);
    startOffsetInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B{108, 153, 216});
    startOffsetInput->getInputNode()->setMaxLabelScale(0.7f);
    startOffsetInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_startOffsetInput = startOffsetInput;

    m_switchLocalButtonSprite =
        ButtonSprite::create("Local", "bigFont.fnt", "GJ_button_01.png");
    m_switchYTButtonSprite =
        ButtonSprite::create("YouTube", "bigFont.fnt", "GJ_button_01.png");
    m_switchHostedButtonSprite =
        ButtonSprite::create("Hosted", "bigFont.fnt", "GJ_button_01.png");
    m_switchLocalButton = CCMenuItemSpriteExtra::create(
        m_switchLocalButtonSprite, this,
        menu_selector(NongAddPopup::onSwitchToLocal));
    m_switchYTButton = CCMenuItemSpriteExtra::create(
        m_switchYTButtonSprite, this,
        menu_selector(NongAddPopup::onSwitchToYT));
    m_switchHostedButton = CCMenuItemSpriteExtra::create(
        m_switchHostedButtonSprite, this,
        menu_selector(NongAddPopup::onSwitchToHosted));
    m_switchLocalMenu = CCMenu::create();
    m_switchYTMenu = CCMenu::create();
    m_switchHostedMenu = CCMenu::create();
    m_switchLocalMenu->setContentSize(
        {m_switchLocalButton->getContentWidth(), 30.f});
    m_switchLocalMenu->addChildAtPosition(m_switchLocalButton, Anchor::Center);
    m_switchYTMenu->setContentSize({m_switchYTButton->getContentWidth(), 30.f});
    m_switchYTMenu->addChildAtPosition(m_switchYTButton, Anchor::Center);
    m_switchHostedMenu->setContentSize(
        {m_switchHostedButton->getContentWidth(), 30.f});
    m_switchHostedMenu->addChildAtPosition(m_switchHostedButton,
                                           Anchor::Center);

    m_switchButtonsMenu = CCMenu::create();
    m_switchButtonsMenu->addChild(m_switchLocalMenu);
    m_switchButtonsMenu->addChild(m_switchYTMenu);
    m_switchButtonsMenu->addChild(m_switchHostedMenu);
    m_switchButtonsMenu->setContentSize({280.f, 50.f});
    m_switchButtonsMenu->setLayout(
        RowLayout::create()->setAxisAlignment(AxisAlignment::Center));

    m_specificInputsMenu = CCMenu::create();
    m_specificInputsMenu->setContentSize({350.f, 70.f});
    m_specificInputsMenu->setLayout(
        ColumnLayout::create()->setAxisAlignment(AxisAlignment::End));

    m_localMenu = CCMenu::create();
    m_localMenu->setContentSize(m_specificInputsMenu->getContentSize());
    m_localMenu->setLayout(
        RowLayout::create()->setAxisAlignment(AxisAlignment::End));
    auto spr =
        CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    spr->setScale(0.7f);
    m_localSongButton = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(NongAddPopup::openFile));
    m_localSongButton->setID("select-file-button");
    m_localSongButtonMenu = CCMenu::create();
    m_localSongButtonMenu->setID("select-file-menu");
    m_localSongButtonMenu->addChild(this->m_localSongButton);
    m_localSongButtonMenu->setContentSize(
        m_localSongButton->getScaledContentSize());
    m_localSongButtonMenu->setAnchorPoint({1.0f, 0.0f});
    m_localSongButtonMenu->setPosition(
        m_mainLayer->getContentSize().width - 10.f, 10.f);
    m_localSongButtonMenu->setLayout(ColumnLayout::create());
    m_localLinkInput = TextInput::create(350.f, "Local Path", "bigFont.fnt");
    m_localLinkInput->setID("local-link-input");
    m_localLinkInput->setCommonFilter(CommonFilter::Any);
    m_localLinkInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B{108, 153, 216});
    m_localLinkInput->getInputNode()->setMaxLabelScale(0.7f);
    m_localLinkInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_localMenu->addChild(m_localLinkInput);
    m_localMenu->addChild(m_localSongButtonMenu);
    m_localMenu->updateLayout();
    m_localMenu->retain();

    m_ytLinkInput = TextInput::create(350.f, "YouTube Link", "bigFont.fnt");
    m_ytLinkInput->setID("yt-link-input");
    m_ytLinkInput->setCommonFilter(CommonFilter::Any);
    m_ytLinkInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B{108, 153, 216});
    m_ytLinkInput->getInputNode()->setMaxLabelScale(0.7f);
    m_ytLinkInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_ytLinkInput->retain();

    m_hostedLinkInput = TextInput::create(350.f, "Hosted Link", "bigFont.fnt");
    m_hostedLinkInput->setID("hosted-link-input");
    m_hostedLinkInput->setCommonFilter(CommonFilter::Any);
    m_hostedLinkInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B{108, 153, 216});
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
    inputParent->setContentSize({250.f, 150.f});
    inputParent->setAnchorPoint({0.5f, 1.0f});
    inputParent->setLayout(ColumnLayout::create()->setAxisReverse(true));
    m_mainLayer->addChildAtPosition(inputParent, Anchor::Top, {0.0f, -40.f});

    this->setSongType(NongAddPopupSongType::local);
}

void NongAddPopup::onPublish(CCObject* target) {
    IndexChoosePopup::create(m_publishableIndexes, [this](std::string id) {
        auto index = IndexManager::get()->m_loadedIndexes.at(id).get();
        auto name = IndexManager::get()->getIndexName(id).value();
        auto submit = index->m_features.m_submit.value();

        auto submitFunc = [this, index, submit](FLAlertLayer* _,
                                                bool confirmed) {
            if (submit.m_requestParams.has_value()) {
                if (!submit.m_requestParams.value().m_params) {
                    CCApplication::get()->openURL(
                        submit.m_requestParams.value().m_url.c_str());
                    return;
                }

                Song* replacedNong = m_replacedNong.value();

                std::string songSpecificParams = "";
                switch (replacedNong->type()) {
                    case NongType::LOCAL:
                        songSpecificParams =
                            fmt::format("&path={}&source=local",
                                        replacedNong->path().value());
                        break;
                    case NongType::YOUTUBE:
                        songSpecificParams = fmt::format(
                            "&yt-id={}&source=youtube",
                            static_cast<YTSong*>(replacedNong)->youtubeID());
                        break;
                    case NongType::HOSTED:
                        songSpecificParams = fmt::format(
                            "&url={}&source=youtube",
                            static_cast<HostedSong*>(replacedNong)->url());
                        break;
                }

                auto metadata = replacedNong->metadata();
                CCApplication::get()->openURL(
                    fmt::format("{}song-name={}&artist-name={}&start-offset={}&"
                                "song-id={}&level-id={}&level-name={}{}",
                                submit.m_requestParams.value().m_url.c_str(),
                                metadata->name, metadata->artist,
                                metadata->startOffset, metadata->gdID, 0,
                                metadata->level.value_or(""),
                                songSpecificParams)
                        .c_str());
            }
        };

        if (submit.m_preSubmitMessage.has_value()) {
            auto popup = IndexDisclaimerPopup::create(
                fmt::format("{} Disclaimer", name).c_str(),
                submit.m_preSubmitMessage.value(), "Back", "Continue", 420.f,
                submitFunc);
            popup->m_scene = this;
            popup->show();
        } else {
            submitFunc(nullptr, true);
        }
    })->show();
}

void NongAddPopup::addSong(CCObject* target) {
    auto artistName = std::string(m_artistNameInput->getString());
    auto songName = std::string(m_songNameInput->getString());
    std::optional<std::string> levelName =
        m_levelNameInput->getString().size() > 0
            ? std::optional<std::string>(m_levelNameInput->getString())
            : std::nullopt;
    auto startOffsetStr = m_startOffsetInput->getString();

    int startOffset = 0;

    if (startOffsetStr != "") {
        startOffset = std::stoi(startOffsetStr);
    }

    if (m_songType == NongAddPopupSongType::local) {
        auto res =
            this->addLocalSong(songName, artistName, levelName, startOffset);
        if (res.isErr()) {
            FLAlertLayer::create("Error", res.error(), "Ok")->show();
        }
    } else if (m_songType == NongAddPopupSongType::yt) {
        auto res =
            this->addYTSong(songName, artistName, levelName, startOffset);
        if (res.isErr()) {
            FLAlertLayer::create("Error", res.error(), "Ok")->show();
        }
    } else if (m_songType == NongAddPopupSongType::hosted) {
        auto res =
            this->addHostedSong(songName, artistName, levelName, startOffset);
        if (res.isErr()) {
            FLAlertLayer::create("Error", res.error(), "Ok")->show();
        }
    }

    FLAlertLayer::create("Success", "Song was added successfuly!", "Ok")
        ->show();
    this->onClose(this);
}

geode::Result<> NongAddPopup::addLocalSong(
    const std::string& songName, const std::string& artistName,
    const std::optional<std::string> levelName, int offset) {
    std::filesystem::path songPath = m_localLinkInput->getString();
#ifdef GEODE_IS_WINDOWS
    if (wcslen(songPath.c_str()) == 0) {
#else
    if (strlen(songPath.c_str()) == 0) {
#endif
        return Err("No file selected.");
    }
    if (!std::filesystem::exists(songPath)) {
        return Err(fmt::format("The selected file ({}) does not exist.",
                               songPath.string()));
    }

    if (std::filesystem::is_directory(songPath)) {
        return Err("You selected a directory.");
    }

    std::string extension = songPath.extension().string();

    // make the extension lowercase, who knows what people got on their
    // computers
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (extension != ".mp3" && extension != ".ogg" && extension != ".wav" &&
        extension != ".flac") {
        return Err(
            "The selected file must be one of the "
            "following: <cb>mp3, wav, flac, ogg</c>.");
    }

    if (songName == "") {
        return Err("Song name is empty");
    }

    if (artistName == "") {
        return Err("Artist name is empty");
    }

    std::string id = jukebox::random_string(16);
    std::string unique = fmt::format("{}.{}", id, extension);
    std::filesystem::path destination = Mod::get()->getSaveDir() / "nongs";
    if (!std::filesystem::exists(destination)) {
        std::filesystem::create_directory(destination);
    }
    unique += songPath.extension().string();
    destination /= unique;
    std::error_code error_code;
    bool result = std::filesystem::copy_file(songPath, destination, error_code);
    if (error_code) {
        return Err(fmt::format(
            "Failed to save song. Please try again! Error category: {}, "
            "message: {}",
            error_code.category().name(),
            error_code.category().message(error_code.value())));
    }
    if (!result) {
        return Err(
            "Failed to copy song to Jukebox's songs folder. Please try again.");
    }

    std::string error = "";

    LocalSong song = LocalSong{
        SongMetadata{m_songID, id, songName, artistName, levelName, offset},
        destination};

    Nongs* nongs = NongManager::get()->getNongs(m_songID).value();

    if (m_replacedNong.has_value()) {
        auto res = nongs->replaceSong(
            m_replacedNong.value()->metadata()->uniqueID, std::move(song));

        if (res.isErr()) {
            return Err(fmt::format("Failed to add song: {}", res.error()));
        }
    } else {
        auto res = nongs->add(std::move(song));
        if (res.isErr()) {
            return Err(fmt::format("Failed to add song: {}", res.error()));
        }
    }

    (void)nongs->commit();

    return Ok();
}

geode::Result<> NongAddPopup::addYTSong(
    const std::string& songName, const std::string& artistName,
    const std::optional<std::string> levelName, int offset) {
    constexpr size_t YOUTUBE_VIDEO_ID_SIZE = 11;

    if (songName == "") {
        return Err("Song name is empty");
    }
    if (artistName == "") {
        return Err("Artist name is empty");
    }
    std::string ytLink = m_ytLinkInput->getString();
    if (strlen(ytLink.c_str()) == 0) {
        return Err("No URL specified");
    }

    std::string ytID;

    if (ytLink.size() == YOUTUBE_VIDEO_ID_SIZE) {
        ytID = ytLink;
    }

    // I'll trust you on this one Flafy :)
    const static std::regex youtube_regex(
        "(?:youtube\\.com\\/.*[?&]v=|youtu\\.be\\/)([a-zA-Z0-9_-]{11})");
    std::smatch match;

    if (std::regex_search(ytLink, match, youtube_regex) && match.size() > 1) {
        ytID = match.str(1);
    }

    if (ytID.size() != YOUTUBE_VIDEO_ID_SIZE) {
        return Err("Invalid YouTube video ID");
    }

    Nongs* nongs = NongManager::get()->getNongs(m_songID).value();
    YTSong song = YTSong{
        SongMetadata{
            m_songID,
            jukebox::random_string(16),
            songName,
            artistName,
            levelName,
            offset,
        },
        ytID,
        std::nullopt,
    };

    if (m_replacedNong.has_value()) {
        auto res = nongs->replaceSong(
            m_replacedNong.value()->metadata()->uniqueID, std::move(song));

        if (res.isErr()) {
            return Err(fmt::format("Failed to update song: {}", res.error()));
        }
    } else {
        auto res = nongs->add(std::move(song));

        if (res.isErr()) {
            return Err(fmt::format("Failed to add song: {}", res.error()));
        }
    }

    return Ok();
}

geode::Result<> NongAddPopup::addHostedSong(
    const std::string& songName, const std::string& artistName,
    const std::optional<std::string> levelName, int offset) {
    if (songName == "") {
        return Err("Song name is empty");
    }
    if (artistName == "") {
        return Err("Artist name is empty");
    }

    std::string hostedLink = m_hostedLinkInput->getString();
    if (strlen(hostedLink.c_str()) == 0) {
        return Err("No URL specified");
    }

    std::string id;

    if (m_replacedNong.has_value()) {
        id = m_replacedNong.value()->metadata()->uniqueID;
    } else {
        id = jukebox::random_string(16);
    }

    Nongs* nongs = NongManager::get()->getNongs(m_songID).value();

    HostedSong song = HostedSong{
        SongMetadata{
            m_songID,
            id,
            songName,
            artistName,
            levelName,
            offset,
        },
        m_hostedLinkInput->getString(),
        std::nullopt,
    };

    if (m_replacedNong.has_value()) {
        auto res = nongs->replaceSong(id, std::move(song));

        if (res.isErr()) {
            return Err(fmt::format("Failed to update song: {}", res.error()));
        }
    } else {
        auto res = nongs->add(std::move(song));

        if (res.isErr()) {
            return Err(fmt::format("Failed to createsong: {}", res.error()));
        }
    }
    return Ok();
}

void NongAddPopup::onClose(CCObject* target) {
    m_localMenu->release();
    m_ytLinkInput->release();
    m_hostedLinkInput->release();
    Popup::onClose(target);
}

std::optional<NongAddPopup::ParsedMetadata> NongAddPopup::tryParseMetadata(
    std::filesystem::path path) {
    const char* MP3_NAME_TAG = "TIT2";
    const char* MP3_ARTIST_TAG = "TPE1";

    const char* OGG_FLAC_NAME_TAG = "TITLE";
    const char* OGG_FLAC_ARTIST_TAG = "ARTIST";

    const char* WAV_NAME_TAG = "INAM";
    const char* WAV_ARTIST_TAG = "IART";

    // Thanks to undefined06855 for most of this stuff
    // https://github.com/undefined06855/EditorMusic/blob/main/src/AudioManager.cpp

    NongAddPopup::ParsedMetadata ret{};
    FMOD::Sound* sound;
    FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;

    system->createSound(path.string().c_str(), FMOD_LOOP_NORMAL, nullptr,
                        &sound);

    if (!sound) {
        return std::nullopt;
    }

    FMOD_TAG nameTag = {};
    FMOD_TAG artistTag = {};
    FMOD_RESULT nameResult;
    FMOD_RESULT artistResult;

    const std::string extension = path.extension().string();
    if (extension == ".mp3") {
        nameResult = sound->getTag(MP3_NAME_TAG, 0, &nameTag);
        artistResult = sound->getTag(MP3_ARTIST_TAG, 0, &artistTag);
    } else if (extension == ".ogg" || extension == ".flac") {
        nameResult = sound->getTag(OGG_FLAC_NAME_TAG, 0, &nameTag);
        artistResult = sound->getTag(OGG_FLAC_ARTIST_TAG, 0, &artistTag);
    } else if (extension == ".wav") {
        nameResult = sound->getTag(WAV_NAME_TAG, 0, &nameTag);
        artistResult = sound->getTag(WAV_ARTIST_TAG, 0, &artistTag);
    }

    if (nameResult != FMOD_ERR_TAGNOTFOUND) {
        ret.name = parseFromFMODTag(nameTag);
    }
    if (artistResult != FMOD_ERR_TAGNOTFOUND) {
        ret.artist = parseFromFMODTag(artistTag);
    }

    return ret;
}

}  // namespace jukebox
