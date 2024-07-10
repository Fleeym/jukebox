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

bool NongAddPopup::setup(NongDropdownLayer* parent, int songID) {
    this->setTitle("Add Song");
    m_parentPopup = parent;
    m_songID = songID;

    auto spr = CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    spr->setScale(0.7f);
    m_selectSongButton = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(NongAddPopup::openFile)
    );
    m_selectSongMenu = CCMenu::create();
    m_selectSongMenu->setID("select-file-menu");
    m_selectSongButton->setID("select-file-button");
    m_selectSongMenu->addChild(this->m_selectSongButton);
    m_selectSongMenu->setContentSize(m_selectSongButton->getScaledContentSize());
    m_selectSongMenu->setAnchorPoint({1.0f, 0.0f});
    m_selectSongMenu->setPosition(m_mainLayer->getContentSize().width - 10.f, 10.f);
    m_selectSongMenu->setLayout(ColumnLayout::create());

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

    auto selectedContainer = CCNode::create();
    selectedContainer->setID("selected-container");
    m_selectedContainer = selectedContainer;
    auto selectedLabel = CCLabelBMFont::create("Selected file:", "goldFont.fnt");
    selectedLabel->setScale(0.75f);
    selectedLabel->setID("selected-label");
    selectedContainer->addChild(selectedLabel);
    auto layout = ColumnLayout::create();
    layout->setAutoScale(false);
    layout->setAxisReverse(true);
    layout->setAxisAlignment(AxisAlignment::End);
    selectedContainer->setContentSize({ 250.f, 50.f });
    selectedContainer->setPosition(m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 25.f);
    selectedContainer->setAnchorPoint({0.5f, 1.0f});
    selectedContainer->setLayout(layout);
    m_mainLayer->addChild(selectedContainer);

    m_mainLayer->addChild(this->m_selectSongMenu);
    m_mainLayer->addChild(this->m_addSongMenu);
    this->createInputs();

    return true;
}

NongAddPopup* NongAddPopup::create(NongDropdownLayer* parent, int songID) {
    auto ret = new NongAddPopup();
    auto size = ret->getPopupSize();
    if (ret && ret->initAnchored(size.width, size.height, parent, songID)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCSize NongAddPopup::getPopupSize() {
    return { 320.f, 240.f };
}

void NongAddPopup::addPathLabel(std::string const& path) {
    if (m_songPathContainer != nullptr) {
        m_songPathContainer->removeFromParent();
        m_songPathContainer = nullptr;
        m_songPathLabel = nullptr;
    }
    auto container = CCNode::create();
    container->setID("song-path-container");
    auto label = CCLabelBMFont::create(path.c_str(), "bigFont.fnt");
    label->limitLabelWidth(240.f, 0.6f, 0.1f);
    label->setID("song-path-label");
    m_songPathLabel = label;
    m_songPathContainer = container;

    auto bgSprite = CCScale9Sprite::create(
        "square02b_001.png",
        { 0.0f, 0.0f, 80.0f, 80.0f }
    );
    bgSprite->setID("song-path-bg");
    bgSprite->setColor({ 0, 0, 0 });
    bgSprite->setOpacity(75);
    bgSprite->setScale(0.4f);
    bgSprite->setContentSize(CCPoint { 250.f, 25.f } / bgSprite->getScale());
    container->setContentSize(bgSprite->getScaledContentSize());
    bgSprite->setPosition(container->getScaledContentSize() / 2);
    label->setPosition(container->getScaledContentSize() / 2);
    container->addChild(bgSprite);
    container->addChild(label);
    container->setAnchorPoint({0.5f, 0.5f});
    container->setPosition(m_mainLayer->getContentSize() / 2);
    m_selectedContainer->addChild(container);
    m_selectedContainer->updateLayout();
}

void NongAddPopup::openFile(CCObject* target) {
    auto task = Task<Result<std::filesystem::path>>::run([](auto progress, auto hasBeenCancelled) -> Task<Result<std::filesystem::path>>::Result {
        return Ok(std::filesystem::path("C:\\users\\flafy\\AppData\\Local\\GeometryDash\\10101.mp3"));
    }, "My epic task that sums up numbers for some reason");

    file::FilePickOptions::Filter filter = {};
    m_pickListener.bind(this, &NongAddPopup::onFileOpen);
    m_pickListener.setFilter(task);
    // #ifdef GEODE_IS_WINDOWS
    // file::FilePickOptions::Filter filter = {
    //     .description = "Songs",
    //     .files = { "*.mp3", "*.flac", "*.wav", "*.ogg" }
    // };
    // #else
    // file::FilePickOptions::Filter filter = {};
    // #endif
    // file::FilePickOptions options = {
    //     std::nullopt,
    //     {filter}
    // };

    // auto callback = [this](Result<std::filesystem::path> result) {
    // };
    // auto failedCallback = []() {
    //     FLAlertLayer::create("Error", "Failed to open file", "Ok")->show();
    // };

    // m_pickListener.bind(this, &NongAddPopup::onFileOpen);
    // m_pickListener.setFilter(file::pick(file::PickMode::OpenFile, options));
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
                        ss << fmt::format("\nName: \"{}\". ", meta->name.value());
                    }
                    if (meta->artist.has_value()) {
                        ss << fmt::format("\nArtist: \"{}\". ", meta->artist.value());
                    }

                    ss << "\nDo you want to set those values for the song?";

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

        this->addPathLabel(strPath);
        m_songPath = path;
    }
}

void NongAddPopup::createInputs() {
    auto inputParent = CCNode::create();
    inputParent->setID("input-parent");
    auto songInput = TextInput::create(250.f, "Song name*", "bigFont.fnt");
    songInput->setID("song-name-input");
    songInput->setCommonFilter(CommonFilter::Any);
    songInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    songInput->getInputNode()->setMaxLabelScale(0.7f);
    songInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_songNameInput = songInput;

    auto artistInput = TextInput::create(250.f, "Artist name*", "bigFont.fnt");
    artistInput->setID("artist-name-input");
    artistInput->setCommonFilter(CommonFilter::Any);
    artistInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    artistInput->getInputNode()->setMaxLabelScale(0.7f);
    artistInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_artistNameInput = artistInput;

    auto levelNameInput = TextInput::create(250.f, "Level name", "bigFont.fnt");
    levelNameInput->setID("artist-name-input");
    levelNameInput->setCommonFilter(CommonFilter::Any);
    levelNameInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    levelNameInput->getInputNode()->setMaxLabelScale(0.7f);
    levelNameInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_levelNameInput = levelNameInput;

    auto startOffsetInput = TextInput::create(250.f, "Start offset (ms)", "bigFont.fnt");
    startOffsetInput->setID("start-offset-input");
    startOffsetInput->setCommonFilter(CommonFilter::Int);
    startOffsetInput->getInputNode()->setLabelPlaceholderColor(ccColor3B {108, 153, 216});
    startOffsetInput->getInputNode()->setMaxLabelScale(0.7f);
    startOffsetInput->getInputNode()->setLabelPlaceholderScale(0.7f);
    m_startOffsetInput = startOffsetInput;

    float inputHeight = songInput->getContentSize().height;

    inputParent->addChild(songInput);
    inputParent->addChild(artistInput);
    inputParent->addChild(levelNameInput);
    inputParent->addChild(startOffsetInput);
    inputParent->setContentSize({ 250.f, 100.f });
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
}

void NongAddPopup::addSong(CCObject* target) {
    auto artistName = std::string(m_artistNameInput->getString());
    auto songName = std::string(m_songNameInput->getString());
    std::string levelName = m_levelNameInput->getString();
    auto startOffsetStr = m_startOffsetInput->getString();
    #ifdef GEODE_IS_WINDOWS
    if (wcslen(m_songPath.c_str()) == 0) {
    #else
    if (strlen(m_songPath.c_str()) == 0) {
    #endif
        FLAlertLayer::create("Error", "No file selected.", "Ok")->show();
        return;
    }
    if (!fs::exists(m_songPath)) {
        std::stringstream ss;
        ss << "The selected file (" << m_songPath.string() << ") does not exist.";
        FLAlertLayer::create("Error", ss.str().c_str(), "Ok")->show();
        return;
    }

    if (fs::is_directory(m_songPath)) {
        FLAlertLayer::create("Error", "You selected a directory.", "Ok")->show();
        return;
    }

    std::string extension = m_songPath.extension().string();

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
    unique += m_songPath.extension().string();
    destination = destination / unique;
    bool result;
    std::error_code error_code;
    result = fs::copy_file(m_songPath, destination, error_code);
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

    Nong nong = {
        LocalSong {
          SongMetadata {
            m_songID,
            songName,
            artistName,
            levelName,
            startOffset,
          },
          destination,
        },
    };

    m_parentPopup->addSong(std::move(nong.toNongs().unwrap()));
    this->onClose(this);
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
