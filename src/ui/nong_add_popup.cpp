#include "nong_add_popup.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <system_error>
#include <utility>

#include <fmt/core.h>
#include "Geode/Result.hpp"
#include "Geode/binding/ButtonSprite.hpp"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/binding/FLAlertLayer.hpp"
#include "Geode/binding/FLAlertLayerProtocol.hpp"
#include "Geode/binding/FMODAudioEngine.hpp"
#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/loader/Log.hpp"
#include "Geode/loader/Mod.hpp"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/Popup.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/Task.hpp"
#include "Geode/utils/file.hpp"
#include "Geode/utils/string.hpp"
#include "events/manual_song_added.hpp"
#include "fmod.hpp"
#include "fmod_common.h"

#include "managers/index_manager.hpp"
#include "nong.hpp"
#include "ui/index_choose_popup.hpp"
#include "utils/random_string.hpp"

using namespace jukebox::index;

void setInputProps(TextInput* i) {
    i->getInputNode()->setLabelPlaceholderColor({108, 153, 216});
    i->getInputNode()->setMaxLabelScale(0.7f);
    i->getInputNode()->setLabelPlaceholderScale(0.7f);
}

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
    std::function<void(FLAlertLayer*, bool)> m_selected;

    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        m_selected(layer, btn2);
    }

public:
    static IndexDisclaimerPopup* create(
        char const* title, std::string const& content, char const* btn1,
        char const* btn2, float width,
        std::function<void(FLAlertLayer*, bool)> selected) {
        auto inst = new IndexDisclaimerPopup();
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

bool NongAddPopup::setup(int songID, std::optional<Song*> replacedNong) {
    this->setTitle("Add Song");
    m_songID = songID;
    m_replacedNong = replacedNong;

    if (replacedNong.has_value() &&
        replacedNong.value()->type() == NongType::YOUTUBE) {
        m_mainLayer->addChildAtPosition(
            CCLabelBMFont::create(
                "Sorry, YouTube songs are not available at the moment.",
                "bigFont.fnt"),
            Anchor::Center);
        return true;
    }

    const CCSize size = this->getPopupSize();
    const CCSize usefulSize = {size.width, size.height - 50.0f};
    const float inputWidth = size.width;

    m_container = CCNode::create();
    m_container->setContentSize(usefulSize);
    m_container->setAnchorPoint({0.5f, 0.5f});
    m_container->setID("container");

    // <SWITCHER>

    m_switchMenu = CCMenu::create();
    m_switchMenu->setID("switch-menu");
    m_switchMenu->ignoreAnchorPointForPosition(false);
    m_switchMenu->setContentSize({inputWidth, 30.0f});

    m_switchLocalSpr =
        ButtonSprite::create("Local", "bigFont.fnt", "GJ_button_01.png");
    m_switchLocalButton = CCMenuItemSpriteExtra::create(
        m_switchLocalSpr, this, menu_selector(NongAddPopup::onSwitchToLocal));
    m_switchLocalButton->setID("switch-local-button");

    m_switchHostedSpr =
        ButtonSprite::create("Hosted", "bigFont.fnt", "GJ_button_04.png");
    m_switchHostedButton = CCMenuItemSpriteExtra::create(
        m_switchHostedSpr, this, menu_selector(NongAddPopup::onSwitchToHosted));
    m_switchLocalButton->setID("switch-hosted-button");

    m_switchYTSpr =
        ButtonSprite::create("YouTube", "bigFont.fnt", "GJ_button_04.png");
    m_switchYTButton = CCMenuItemSpriteExtra::create(
        m_switchYTSpr, this, menu_selector(NongAddPopup::onSwitchToYT));
    m_switchYTButton->setID("switch-yt-button");

    m_switchMenu->addChild(m_switchLocalButton);
    m_switchMenu->addChild(m_switchHostedButton);
    m_switchMenu->addChild(m_switchYTButton);
    m_switchMenu->setLayout(
        RowLayout::create()->setAxisAlignment(AxisAlignment::Center));

    m_container->addChild(m_switchMenu);

    // </SWITCHER>

    // <MAIN_METADATA>

    TextInput* songInput =
        TextInput::create(inputWidth, "Song name*", "bigFont.fnt");
    songInput->setID("song-name-input");
    songInput->setCommonFilter(CommonFilter::Any);
    setInputProps(songInput);
    m_songNameInput = songInput;

    TextInput* artistInput =
        TextInput::create(inputWidth, "Artist name*", "bigFont.fnt");
    artistInput->setID("artist-name-input");
    artistInput->setCommonFilter(CommonFilter::Any);
    setInputProps(artistInput);
    m_artistNameInput = artistInput;

    TextInput* levelNameInput =
        TextInput::create(inputWidth, "Level name", "bigFont.fnt");
    levelNameInput->setID("artist-name-input");
    levelNameInput->setCommonFilter(CommonFilter::Any);
    setInputProps(levelNameInput);
    m_levelNameInput = levelNameInput;

    auto startOffsetInput =
        TextInput::create(inputWidth, "Start offset (ms)", "bigFont.fnt");
    startOffsetInput->setID("start-offset-input");
    startOffsetInput->setCommonFilter(CommonFilter::Int);
    setInputProps(startOffsetInput);
    m_startOffsetInput = startOffsetInput;

    m_container->addChild(m_songNameInput);
    m_container->addChild(m_artistNameInput);
    m_container->addChild(m_levelNameInput);
    m_container->addChild(m_startOffsetInput);

    // </MAIN_METADATA>

    // <SPECIAL>

    m_specialInfoNode = CCNode::create();
    m_specialInfoNode->setAnchorPoint({0.5f, 0.5f});
    m_specialInfoNode->setContentSize({usefulSize.width, 40.0f});
    m_specialInfoNode->setID("special-info-node");

    std::string placeholder;

    switch (m_songType) {
        case NongAddPopup::SongType::HOSTED:
            placeholder = "Enter a song URL";
            break;
        case NongAddPopup::SongType::LOCAL:
            placeholder = "Select a file";
            break;
        case NongAddPopup::SongType::YOUTUBE:
            placeholder = "YouTube URL";
            break;
    }

    m_specialInput =
        TextInput::create(inputWidth, placeholder.c_str(), "bigFont.fnt");
    m_specialInput->setID("special-input");
    m_specialInput->setCommonFilter(CommonFilter::Any);
    setInputProps(m_specialInput);

    m_localSongMenu = CCMenu::create();
    m_localSongMenu->ignoreAnchorPointForPosition(true);
    m_localSongMenu->setAnchorPoint({0.5f, 0.5f});
    m_localSongMenu->setID("local-song-menu");
    CCSprite* spr =
        CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    spr->setScale(0.7f);
    m_localSongButton = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(NongAddPopup::openFile));
    m_localSongMenu->setContentSize(m_localSongButton->getScaledContentSize());
    m_localSongMenu->addChild(m_localSongButton);
    m_localSongMenu->setLayout(RowLayout::create());

    m_specialInfoNode->addChild(m_specialInput);
    m_specialInfoNode->addChild(m_localSongMenu);
    AxisLayout* specialNodeLayout = RowLayout::create();
    specialNodeLayout->ignoreInvisibleChildren(true);
    m_specialInfoNode->setLayout(specialNodeLayout);

    m_container->addChild(m_specialInfoNode);

    // </SPECIAL>

    // <BOTTOM_MENU>

    m_addSongMenu = CCMenu::create();
    m_addSongMenu->setID("add-song-menu");
    m_addSongButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create(m_replacedNong.has_value() ? "Edit" : "Add"), this,
        menu_selector(NongAddPopup::addSong));

    if (m_replacedNong.has_value()) {
        m_publishSongButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Publish"), this,
            menu_selector(NongAddPopup::onPublish));
        m_publishSongButton->setID("publish-song-button");
        m_addSongMenu->addChild(m_publishSongButton);
    }

    m_addSongMenu->addChild(m_addSongButton);
    m_addSongMenu->setAnchorPoint({0.5f, 0.5f});
    m_addSongMenu->setContentSize({m_size.width, 20.0f});
    AxisLayout* layout = RowLayout::create();
    layout->ignoreInvisibleChildren(true);
    m_addSongMenu->setLayout(layout);

    m_container->addChild(m_addSongMenu);

    // </BOTTOM_MENU>

    m_container->setContentSize(usefulSize);
    m_container->setAnchorPoint({0.5f, 1.0f});
    m_container->setLayout(ColumnLayout::create()->setAxisReverse(true));
    m_mainLayer->addChildAtPosition(m_container, Anchor::Top, {0.0f, -40.f});

    if (!m_replacedNong.has_value()) {
        return true;
    }

    // edit mode

    Song* edit = m_replacedNong.value();

    m_songNameInput->setString(edit->metadata()->name);
    m_artistNameInput->setString(edit->metadata()->artist);
    if (edit->metadata()->level.has_value()) {
        m_levelNameInput->setString(edit->metadata()->level.value());
    }
    m_startOffsetInput->setString(
        std::to_string(edit->metadata()->startOffset));

    switch (edit->type()) {
        case NongType::LOCAL:
            m_specialInput->setString(edit->path().value().string());
            this->setSongType(SongType::LOCAL);
            break;
        case NongType::YOUTUBE:
            m_specialInput->setString(static_cast<YTSong*>(edit)->youtubeID());
            this->setSongType(SongType::YOUTUBE);
            break;
        case NongType::HOSTED:
            m_specialInput->setString(static_cast<HostedSong*>(edit)->url());
            this->setSongType(SongType::HOSTED);
            break;
    }

    std::vector<std::string> indexIDs;

    for (const std::pair<const std::string, std::unique_ptr<IndexMetadata>>&
             pair : IndexManager::get().m_loadedIndexes) {
        if (!pair.second->m_features.m_submit.has_value()) {
            continue;
        }
        IndexMetadata::Features::Submit& submit =
            pair.second->m_features.m_submit.value();

        bool shouldSubmit = false;

        switch (m_replacedNong.value()->type()) {
            case NongType::LOCAL:
                this->setSongType(SongType::LOCAL);
                shouldSubmit = submit.m_supportedSongTypes.at(
                    IndexMetadata::Features::SupportedSongType::LOCAL);
                break;
            case NongType::YOUTUBE:
                this->setSongType(SongType::YOUTUBE);
                shouldSubmit = submit.m_supportedSongTypes.at(
                    IndexMetadata::Features::SupportedSongType::YOUTUBE);
                break;
            case NongType::HOSTED:
                this->setSongType(SongType::HOSTED);
                shouldSubmit = submit.m_supportedSongTypes.at(
                    IndexMetadata::Features::SupportedSongType::HOSTED);
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

    return true;
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

    m_pickListener.bind(this, &NongAddPopup::onFileOpen);
    m_pickListener.setFilter(file::pick(file::PickMode::OpenFile, options));
}

void NongAddPopup::onFileOpen(
    Task<Result<std::filesystem::path>>::Event* event) {
    if (m_songType != SongType::LOCAL) {
        return;
    }

    if (event->isCancelled()) {
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

        m_specialInput->setString(strPath);
    }
}

void NongAddPopup::setSongType(SongType type) {
    if (m_songType == type) {
        return;
    }

    m_songType = type;

    const char* on = "GJ_button_01.png";
    const char* off = "GJ_button_04.png";

    m_switchLocalSpr->updateBGImage(type == SongType::LOCAL ? on : off);
    m_switchHostedSpr->updateBGImage(type == SongType::HOSTED ? on : off);

    switch (type) {
        case SongType::LOCAL: {
            m_memoizedHostedInput = m_specialInput->getString();
            m_specialInput->setString("");
            m_localSongMenu->setVisible(true);
            m_specialInfoNode->updateLayout();
            m_specialInput->getInputNode()->getPlaceholderLabel()->setString(
                "Select a file");
            if (!m_memoizedLocalInput.empty()) {
                m_specialInput->setString(m_memoizedLocalInput);
            }
            break;
        }
        case SongType::HOSTED: {
            m_memoizedLocalInput = m_specialInput->getString();
            m_specialInput->setString("");
            m_localSongMenu->setVisible(false);
            m_specialInfoNode->updateLayout();
            m_specialInput->getInputNode()->getPlaceholderLabel()->setString(
                "Enter a song URL");
            if (!m_memoizedHostedInput.empty()) {
                m_specialInput->setString(m_memoizedHostedInput);
            }
            break;
        }
        case SongType::YOUTUBE: {
            return;
        }
    }
}

void NongAddPopup::onSwitchToLocal(CCObject*) {
    this->setSongType(SongType::LOCAL);
}

void NongAddPopup::onSwitchToYT(CCObject*) {
    FLAlertLayer::create("Unavailable",
                         "YouTube songs are currently <cr>unavailable</c>. "
                         "They will be enabled in a future <cb>update</c> :)",
                         "Ok")
        ->show();
    /*this->setSongType(SongType::YOUTUBE);*/
}

void NongAddPopup::onSwitchToHosted(CCObject*) {
    this->setSongType(SongType::HOSTED);
}

void NongAddPopup::onPublish(CCObject* target) {
    IndexChoosePopup::create(m_publishableIndexes, [this](std::string id) {
        auto index = IndexManager::get().m_loadedIndexes.at(id).get();
        auto name = IndexManager::get().getIndexName(id).value();
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

    if (m_songType == SongType::LOCAL) {
        auto res =
            this->addLocalSong(songName, artistName, levelName, startOffset);
        if (res.isErr()) {
            FLAlertLayer::create("Error", res.unwrapErr(), "Ok")->show();
            return;
        }
    } else if (m_songType == SongType::YOUTUBE) {
        auto res =
            this->addYTSong(songName, artistName, levelName, startOffset);
        if (res.isErr()) {
            FLAlertLayer::create("Error", res.unwrapErr(), "Ok")->show();
            return;
        }
    } else if (m_songType == SongType::HOSTED) {
        auto res =
            this->addHostedSong(songName, artistName, levelName, startOffset);
        if (res.isErr()) {
            FLAlertLayer::create("Error", res.unwrapErr(), "Ok")->show();
            return;
        }
    }

    FLAlertLayer::create("Success", "Song was added successfuly!", "Ok")
        ->show();
    this->onClose(this);
}

geode::Result<> NongAddPopup::addLocalSong(
    const std::string& songName, const std::string& artistName,
    const std::optional<std::string> levelName, int offset) {
    std::filesystem::path songPath = m_specialInput->getString();
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

    std::string id = m_replacedNong.has_value()
                         ? m_replacedNong.value()->metadata()->uniqueID
                         : jukebox::random_string(16);
    std::string unique = fmt::format("{}{}", id, extension);
    std::filesystem::path destination = Mod::get()->getSaveDir() / "nongs";
    std::error_code error_code;
    if (!std::filesystem::exists(destination, error_code)) {
        if (!std::filesystem::create_directory(destination, error_code)) {
            return Err("Failed to create nongs directory.");
        }
    }
    destination /= unique;

    if (destination.compare(songPath) != 0) {
        bool result = std::filesystem::copy_file(
            songPath,
            destination,
            std::filesystem::copy_options::overwrite_existing,
            error_code);
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
    }

    LocalSong song = LocalSong{
        SongMetadata{m_songID, id, songName, artistName, levelName, offset},
        destination};

    Nongs* nongs = NongManager::get().getNongs(m_songID).value();

    if (m_replacedNong.has_value()) {
        Result<> res = nongs->replaceSong(
            m_replacedNong.value()->metadata()->uniqueID, std::move(song));

        if (res.isErr()) {
            return Err(fmt::format("Failed to add song: {}", res.unwrapErr()));
        }
    } else {
        Result<LocalSong*> res = nongs->add(std::move(song));
        if (res.isErr()) {
            return Err(fmt::format("Failed to add song: {}", res.unwrapErr()));
        }

        event::ManualSongAdded(nongs, res.unwrap()).post();
    }

    (void)nongs->commit();

    return Ok();
}

geode::Result<> NongAddPopup::addYTSong(
    const std::string& songName, const std::string& artistName,
    const std::optional<std::string> levelName, int offset) {
    return Err("YouTube songs are coming soon :)");

    constexpr size_t YOUTUBE_VIDEO_ID_SIZE = 11;

    if (songName == "") {
        return Err("Song name is empty");
    }
    if (artistName == "") {
        return Err("Artist name is empty");
    }
    std::string ytLink = m_specialInput->getString();
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

    Nongs* nongs = NongManager::get().getNongs(m_songID).value();
    YTSong song = YTSong{
        SongMetadata{
            m_songID,
            m_replacedNong.has_value()
                ? m_replacedNong.value()->metadata()->uniqueID
                : jukebox::random_string(16),
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
            return Err(
                fmt::format("Failed to update song: {}", res.unwrapErr()));
        }
    } else {
        auto res = nongs->add(std::move(song));

        if (res.isErr()) {
            return Err(fmt::format("Failed to add song: {}", res.unwrapErr()));
        }

        event::ManualSongAdded(nongs, res.unwrap()).post();
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

    std::string hostedLink = m_specialInput->getString();
    if (strlen(hostedLink.c_str()) == 0) {
        return Err("No URL specified");
    }

    std::string id;

    if (m_replacedNong.has_value()) {
        id = m_replacedNong.value()->metadata()->uniqueID;
    } else {
        id = jukebox::random_string(16);
    }

    Nongs* nongs = NongManager::get().getNongs(m_songID).value();

    HostedSong song = HostedSong{
        SongMetadata{
            m_songID,
            id,
            songName,
            artistName,
            levelName,
            offset,
        },
        m_specialInput->getString(),
        std::nullopt,
    };

    if (m_replacedNong.has_value()) {
        auto res = nongs->replaceSong(id, std::move(song));

        if (res.isErr()) {
            return Err(
                fmt::format("Failed to update song: {}", res.unwrapErr()));
        }
    } else {
        auto res = nongs->add(std::move(song));

        if (res.isErr()) {
            return Err(
                fmt::format("Failed to createsong: {}", res.unwrapErr()));
        }

        event::ManualSongAdded(nongs, res.unwrap()).post();
    }
    return Ok();
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

NongAddPopup* NongAddPopup::create(int songID,
                                   std::optional<Song*> replacedNong) {
    auto ret = new NongAddPopup();
    auto size = ret->getPopupSize();
    if (ret && ret->initAnchored(size.width, size.height, songID,
                                 std::move(replacedNong))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

}  // namespace jukebox
