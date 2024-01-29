#include "nong_add_popup.hpp"

bool NongAddPopup::setup(NongDropdownLayer* parent) {
    this->setTitle("Add NONG");
    m_parentPopup = parent;

    auto center = CCDirector::sharedDirector()->getWinSize() / 2;

    m_selectSongButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Select mp3..."),
        this,
        menu_selector(NongAddPopup::openFile)
    );
    m_selectSongMenu = CCMenu::create();
    m_selectSongMenu->setID("select-file-menu");
    m_selectSongButton->setID("select-file-button");
    m_selectSongMenu->addChild(this->m_selectSongButton);
    m_selectSongMenu->setPosition(center.width, center.height + this->getPopupSize().height / 2 - 75.f);

    m_addSongButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Add"),
        this,
        menu_selector(NongAddPopup::addSong)
    );
    m_addSongMenu = CCMenu::create();
    m_addSongMenu->setID("add-song-menu");
    m_addSongButton->setID("add-song-button");
    m_addSongMenu->addChild(this->m_addSongButton);
    m_addSongMenu->setPosition(center.width, center.height - this->getPopupSize().height / 2 + 25.f);

    m_containerLayer = CCLayer::create();

    m_containerLayer->addChild(this->m_selectSongMenu);
    m_containerLayer->addChild(this->m_addSongMenu);
    m_mainLayer->addChild(this->m_containerLayer);
    this->createInputs();

    return true;
}

NongAddPopup* NongAddPopup::create(NongDropdownLayer* parent) {
    auto ret = new NongAddPopup();
    auto size = ret->getPopupSize();
    if (ret && ret->init(size.width, size.height, parent)) {
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
    file::FilePickOptions options = {
        std::nullopt,
        {}
    };

    file::pickFile(file::PickMode::OpenFile , options, [this](ghc::filesystem::path result) {
        auto path = fs::path(result.c_str());
        m_songPath = path;
    }, []() {
        FLAlertLayer::create("Error", "Failed to open file", "Ok")->show();
    });
}

void NongAddPopup::createInputs() {
    auto centered = CCDirector::sharedDirector()->getWinSize() / 2;
    auto bgSprite = CCScale9Sprite::create(
        "square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
    );
    bgSprite->setScale(0.7f);
    bgSprite->setColor({ 0, 0, 0 });
    bgSprite->setOpacity(75);
    bgSprite->setPosition(centered);
    bgSprite->setContentSize({ 409.f, 54.f });
    bgSprite->setID("song-name-bg");

    m_songNameInput = CCTextInputNode::create(250.f, 30.f, "Song name", "bigFont.fnt");
    m_songNameInput->setID("song-name-input");
    m_songNameInput->setPosition(centered);
    m_songNameInput->ignoreAnchorPointForPosition(true);
    m_songNameInput->m_textField->setAnchorPoint({ 0.5f, 0.5f });
    m_songNameInput->m_placeholderLabel->setAnchorPoint({ 0.5f, 0.5f });
    m_songNameInput->setMaxLabelScale(0.7f);
    m_songNameInput->setLabelPlaceholderColor(ccc3(108, 153, 216));
    m_songNameInput->setMouseEnabled(true);
    m_songNameInput->setTouchEnabled(true);

    m_containerLayer->addChild(bgSprite);
    m_containerLayer->addChild(this->m_songNameInput);

    m_artistNameInput = CCTextInputNode::create(250.f, 30.f, "Artist name", "bigFont.fnt");
    m_artistNameInput->setID("artist-name-input");
    m_artistNameInput->setPosition(centered.width, centered.height - 50.f);
    m_artistNameInput->ignoreAnchorPointForPosition(true);
    m_artistNameInput->setMaxLabelScale(0.7f);
    m_artistNameInput->m_textField->setAnchorPoint({ 0.5f, 0.5f });
    m_artistNameInput->m_placeholderLabel->setAnchorPoint({ 0.5f, 0.5f });
    m_artistNameInput->setLabelPlaceholderColor(ccc3(108, 153, 216));

    auto bgSprite_artist = CCScale9Sprite::create(
        "square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
    );
    bgSprite_artist->setScale(0.7f);
    bgSprite_artist->setColor({ 0, 0, 0 });
    bgSprite_artist->setOpacity(75);
    bgSprite_artist->setPosition(centered.width, centered.height - 50.f);
    bgSprite_artist->setContentSize({ 409.f, 54.f });
    bgSprite_artist->setID("artist-name-bg");

    m_containerLayer->addChild(bgSprite_artist);
    m_containerLayer->addChild(this->m_artistNameInput);
}

void NongAddPopup::addSong(CCObject* target) {
    auto artistName = std::string(m_artistNameInput->getString());
    auto songName = std::string(m_songNameInput->getString());
    if (m_songPath.empty()) {
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

    auto unique = nongd::random_string(16);
    auto destination = fs::path(Mod::get()->getSaveDir().c_str()) / "nongs";
    if (!fs::exists(destination)) {
        fs::create_directory(destination);
    }
    unique += ".mp3";
    destination = destination / unique;
    bool result;
    try {
        result = fs::copy_file(m_songPath, destination);
    } catch (fs::filesystem_error e) {
        std::stringstream ss;
        ss << "Failed to save song. Please try again! Error: " << e.what();
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