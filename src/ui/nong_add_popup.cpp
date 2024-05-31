#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/cocos/CCDirector.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/MiniFunction.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/ui/TextInput.hpp>
#include <ccTypes.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>

#include <system_error>

#include "nong_add_popup.hpp"
#include "../random_string.hpp"
#include "Geode/utils/file.hpp"

namespace jukebox {

bool NongAddPopup::setup(NongDropdownLayer* parent) {
    this->setTitle("Add Song");
    m_parentPopup = parent;

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
    bgSprite->setContentSize({ 250.f, 55.f });
    bgSprite->setScaleY(0.4f);
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
    #ifdef GEODE_IS_WINDOWS
    file::FilePickOptions::Filter filter = {
        .description = "Songs",
        .files = { ".mp3" }
    };
    #else
    file::FilePickOptions::Filter filter = {};
    #endif
    file::FilePickOptions options = {
        std::nullopt,
        {filter}
    };

    auto callback = [this](ghc::filesystem::path result) {
        auto path = fs::path(result.c_str());
        #ifdef GEODE_IS_WINDOWS
        auto strPath = geode::utils::string::wideToUtf8(result.c_str());
        #else
        std::string strPath = result.c_str();
        #endif
        this->addPathLabel(strPath);
        m_songPath = path;
    };
    auto failedCallback = []() {
        FLAlertLayer::create("Error", "Failed to open file", "Ok")->show();
    };

    file::pickFile(file::PickMode::OpenFile, options, callback, failedCallback);
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

    uint32_t inputs = 3;
    uint32_t gaps = 2;
    float inputHeight = songInput->getContentSize().height;

    inputParent->addChild(songInput);
    inputParent->addChild(artistInput);
    inputParent->addChild(levelNameInput);
    auto layout = ColumnLayout::create();
    layout->setAxisReverse(true);
    inputParent->setContentSize({ 250.f, inputs * inputHeight + gaps * layout->getGap()});
    inputParent->setAnchorPoint({ 0.5f, 1.0f });
    inputParent->setPosition(m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 40.f);
    inputParent->setLayout(layout);
    m_mainLayer->addChild(inputParent);
}

void NongAddPopup::addSong(CCObject* target) {
    auto artistName = std::string(m_artistNameInput->getString());
    auto songName = std::string(m_songNameInput->getString());
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

    if (m_songPath.extension().string() != ".mp3") {
        FLAlertLayer::create("Error", "The selected file must be an MP3.", "Ok")->show();
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

    auto unique = jukebox::random_string(16);
    auto destination = fs::path(Mod::get()->getSaveDir().c_str()) / "nongs";
    if (!fs::exists(destination)) {
        fs::create_directory(destination);
    }
    unique += ".mp3";
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

    SongInfo song = {
        .path = destination,
        .songName = songName,
        .authorName = artistName,
        .songUrl = "local",
    };

    m_parentPopup->addSong(song);
    this->onClose(this);
}

}