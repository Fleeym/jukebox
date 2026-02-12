#include <jukebox/ui/list/nong_cell_ui.hpp>

#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/cocos/platform/CCPlatformMacros.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <ccTypes.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/SimpleAxisLayout.hpp>

using namespace geode::prelude;

namespace jukebox {

bool NongCellUI::init(const cocos2d::CCSize& size, std::function<void()> onSelect, std::function<void()> onTrash,
                      std::function<void()> onFixDefault, std::function<void()> onDownload,
                      std::function<void()> onEdit) {
    if (!CCNode::init()) {
        return false;
    }

    m_size = size;
    m_onSelect = std::move(onSelect);
    m_onTrash = std::move(onTrash);
    m_onFixDefault = std::move(onFixDefault);
    m_onDownload = std::move(onDownload);
    m_onEdit = std::move(onEdit);

    return true;
}

void NongCellUI::build() {
    if (this->getChildrenCount() > 0) {
        this->removeAllChildrenWithCleanup(true);
    }

    this->setContentSize(m_size);
    this->setAnchorPoint({0.5f, 0.5f});
    constexpr float PADDING_X = 12.0f;
    constexpr float PADDING_Y = 6.0f;
    constexpr float OUTLINE_SIZE = 3.f;
    const CCSize maxSize = {m_size.width - 2 * PADDING_X, m_size.height - 2 * PADDING_Y};
    const float songInfoWidth = maxSize.width * (2.0f / 3.0f);
    const float buttonsWidth = maxSize.width - songInfoWidth;

    // Create outline around the song if it's verified
    if (m_isVerified) {
        // Create the "Verified For Level" text
        CCLabelBMFont* verifiedLabel = CCLabelBMFont::create("Verified For Level", "goldFont.fnt");
        verifiedLabel->setScale(0.35f);
        verifiedLabel->setID("verified-label");

        // Create outline shape
        CCScale9Sprite* outlineStencil = CCScale9Sprite::create("square02b_001.png");
        outlineStencil->setScale(0.4f);
        outlineStencil->setContentSize((m_size + CCPoint{OUTLINE_SIZE, OUTLINE_SIZE}) / outlineStencil->getScale());

        // Create text background shape
        CCScale9Sprite* textBgStencil = CCScale9Sprite::create("square02b_001.png");
        textBgStencil->setScale(0.4f);
        textBgStencil->setContentSize(
            (CCPoint{verifiedLabel->getScaledContentWidth() + 5, verifiedLabel->getScaledContentHeight() * 2 + 4}) /
            textBgStencil->getScale());

        // Combine the shapes into a single stencil node
        CCNode* stencilNode = CCNode::create();
        stencilNode->addChildAtPosition(outlineStencil, Anchor::Center);
        stencilNode->addChildAtPosition(
            textBgStencil, Anchor::TopRight,
            {(m_size.width + OUTLINE_SIZE - textBgStencil->getScaledContentWidth()) / 2.f, m_size.height / 2.f});

        // Create the clip node based on the sprite
        CCClippingNode* clipNode = CCClippingNode::create(stencilNode);
        clipNode->setAlphaThreshold(0.05f);
        clipNode->setInverted(false);
        clipNode->setID("gradient-clipping-node");

        // Draw a gradient that will be clipped by the clip node
        CCLayerGradient* gradientLayer = CCLayerGradient::create({253, 166, 16, 255}, {253, 225, 71, 255});
        gradientLayer->setID("gradient");

        // The size of the gradient will be larger that this because of the
        // scale, but it's fine.
        gradientLayer->setContentSize(outlineStencil->getContentSize());

        clipNode->addChildAtPosition(gradientLayer, Anchor::BottomLeft, outlineStencil->getContentSize() / -2.f);
        this->addChildAtPosition(clipNode, Anchor::Center);

        // Add the verified label on the text's background
        this->addChildAtPosition(verifiedLabel, Anchor::Center, textBgStencil->getPosition() + CCPoint(0.f, 7.f));
    }

    CCScale9Sprite* bg = CCScale9Sprite::create("square02b_001.png");
    bg->setColor({76, 42, 25});
    bg->setScale(0.3f);
    bg->setContentSize(m_size / bg->getScale());
    bg->setID("background");
    this->addChildAtPosition(bg, Anchor::Center);

    m_songInfoNode = CCNode::create();
    m_songInfoNode->setAnchorPoint({0.0f, 0.5f});
    m_songInfoNode->setContentSize({songInfoWidth, maxSize.height});
    m_songInfoNode->setID("song-info-node");

    m_songNameLabel = CCLabelBMFont::create(m_songName.c_str(), "bigFont.fnt");
    m_songNameLabel->setAnchorPoint({0.0f, 0.5f});
    m_songNameLabel->setScale(0.6f);
    m_songNameLabel->setID("song-info-label");

    if (m_isSelected) {
        m_songNameLabel->setColor({188, 254, 206});
    }

    m_authorNameLabel = CCLabelBMFont::create(m_authorName.c_str(), "goldFont.fnt");
    m_authorNameLabel->setAnchorPoint({0.0f, 0.5f});
    m_authorNameLabel->setScale(0.5f);
    m_authorNameLabel->setID("artist-label");

    m_metadataLabel = CCLabelBMFont::create(m_metadata.c_str(), "bigFont.fnt");
    m_metadataLabel->setAnchorPoint({0.0f, 0.5f});
    m_metadataLabel->setColor({.r = 162, .g = 191, .b = 255});
    m_metadataLabel->setScale(0.4f);
    m_metadataLabel->setID("metadata-label");

    m_songInfoNode->addChild(m_songNameLabel);
    m_songInfoNode->addChild(m_authorNameLabel);
    if (!m_metadata.empty()) {
        m_songInfoNode->addChild(m_metadataLabel);
    }
    m_songInfoNode->setLayout(SimpleColumnLayout::create()
                                  ->setMainAxisAlignment(MainAxisAlignment::Even)
                                  ->setCrossAxisAlignment(CrossAxisAlignment::Start)
                                  ->setMainAxisScaling(AxisScaling::ScaleDown)
                                  ->setCrossAxisScaling(AxisScaling::ScaleDown)
                                  ->setMinRelativeScale(0.1f));
    this->addChildAtPosition(m_songInfoNode, Anchor::Left, {PADDING_X, 0.0f});

    // Buttons menu
    m_buttonsMenu = CCMenu::create();
    m_buttonsMenu->setID("buttons-menu");
    m_buttonsMenu->setAnchorPoint({1.0f, 0.5f});
    m_buttonsMenu->setContentSize({buttonsWidth, maxSize.height});
    auto buttonsMenuLayout = SimpleRowLayout::create()->setGap(5.0f)->setMainAxisAlignment(MainAxisAlignment::End);
    buttonsMenuLayout->ignoreInvisibleChildren(true);
    m_buttonsMenu->setLayout(buttonsMenuLayout);

    // Edit button
    CCSprite* editSprite = CCSprite::createWithSpriteFrameName("JB_Edit.png"_spr);
    editSprite->setScale(0.7f);
    m_editButton = CCMenuItemSpriteExtra::create(editSprite, this, menu_selector(NongCellUI::onEdit));
    m_editButton->setID("edit-button");
    m_editButton->setAnchorPoint({0.5f, 0.5f});
    m_buttonsMenu->addChild(m_editButton);

    // Select button
    const char* selectSpriteName = m_isSelected ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png";

    CCSprite* selectSprite = CCSprite::createWithSpriteFrameName(selectSpriteName);
    selectSprite->setScale(0.7f);

    m_selectButton = CCMenuItemSpriteExtra::create(selectSprite, this, menu_selector(NongCellUI::onSelect));
    m_selectButton->setAnchorPoint({0.5f, 0.5f});
    m_selectButton->setID("set-button");

    // Make the width static so it doesn't change between ON/OFF
    m_selectButton->setContentSize({30.f, 30.f});
    selectSprite->setPosition(m_selectButton->getContentSize() / 2.f);

    m_buttonsMenu->addChild(m_selectButton);

    // Fix Default button
    CCSprite* fixDefaultSprite = CCSprite::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
    fixDefaultSprite->setScale(0.8f);
    if (!m_isSelected) {
        fixDefaultSprite->setColor({0x80, 0x80, 0x80});
    }
    m_fixButton = CCMenuItemSpriteExtra::create(fixDefaultSprite, this, menu_selector(NongCellUI::onFixDefault));
    m_fixButton->setID("fix-button");
    m_buttonsMenu->addChild(m_fixButton);

    // Download button
    CCSprite* downloadSprite = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
    CCSprite* downloadCancelSprite = CCSprite::createWithSpriteFrameName("GJ_cancelDownloadBtn_001.png");
    downloadSprite->setScale(0.7f);
    downloadCancelSprite->setScale(0.7f);
    m_downloadButton = CCMenuItemSpriteExtra::create(m_isDownloading ? downloadCancelSprite : downloadSprite, this,
                                                     menu_selector(NongCellUI::onDownload));
    m_downloadButton->setID("download-button");
    m_downloadButton->setVisible(!m_isDownloaded);
    m_buttonsMenu->addChild(m_downloadButton);

    CCSprite* progressBarBack = CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
    progressBarBack->setColor({50, 50, 50});
    progressBarBack->setScale(0.62f);

    CCSprite* spr = CCSprite::createWithSpriteFrameName("d_circle_01_001.png");
    spr->setColor({0, 255, 0});

    m_downloadProgressTimer = CCProgressTimer::create(spr);
    m_downloadProgressTimer->setType(CCProgressTimerType::kCCProgressTimerTypeRadial);
    m_downloadProgressTimer->setPercentage(50.f);
    m_downloadProgressTimer->setID("progress-bar");
    m_downloadProgressTimer->setScale(0.66f);

    m_downloadProgressContainer = CCMenu::create();

    m_downloadProgressContainer->addChildAtPosition(m_downloadProgressTimer, Anchor::Center);
    m_downloadProgressContainer->addChildAtPosition(progressBarBack, Anchor::Center);

    m_downloadProgressContainer->setZOrder(-1);
    m_downloadProgressContainer->setVisible(false);

    m_downloadButton->addChildAtPosition(m_downloadProgressContainer, Anchor::Center);

    // Trash button
    CCSprite* trashSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    trashSprite->setScale(0.6475f);
    m_trashButton = CCMenuItemSpriteExtra::create(trashSprite, this, menu_selector(NongCellUI::onTrash));
    m_trashButton->setID("trash-button");
    m_buttonsMenu->addChild(m_trashButton);

    m_selectButton->setVisible(m_showSelectButton);
    m_trashButton->setVisible(m_showTrashButton);
    m_fixButton->setVisible(m_showFixDefaultButton);
    m_downloadButton->setVisible(m_showDownloadButton);
    m_editButton->setVisible(m_showEditButton);

    m_downloadProgressContainer->setVisible(m_isDownloading);
    if (m_isDownloading) {
        m_downloadButton->setColor(ccc3(105, 105, 105));
    } else {
        m_downloadButton->setColor({255, 255, 255});
    }

    buildOnlyDownloadProgress();

    m_buttonsMenu->updateLayout();
    this->addChildAtPosition(m_buttonsMenu, Anchor::Right, {-PADDING_X, 0.0f});
}

void NongCellUI::buildOnlyDownloadProgress() {
    if (m_isDownloading && !m_downloadProgressContainer->isVisible()) {
        build();
    } else {
        m_downloadProgressTimer->setPercentage(m_downloadProgress);
    }
}

void NongCellUI::onSelect(CCObject*) { m_onSelect(); }
void NongCellUI::onTrash(CCObject*) { m_onTrash(); }
void NongCellUI::onFixDefault(CCObject*) { m_onFixDefault(); }
void NongCellUI::onDownload(CCObject*) { m_onDownload(); }
void NongCellUI::onEdit(CCObject*) { m_onEdit(); }

NongCellUI* NongCellUI::create(const cocos2d::CCSize& size, std::function<void()> onSelect,
                               std::function<void()> onTrash, std::function<void()> onFixDefault,
                               std::function<void()> onDownload, std::function<void()> onEdit) {
    auto ret = new NongCellUI();
    if (ret->init(size, std::move(onSelect), std::move(onTrash), std::move(onFixDefault), std::move(onDownload),
                  std::move(onEdit))) {
        return ret;
    }

    delete ret;
    return nullptr;
}

}  // namespace jukebox
