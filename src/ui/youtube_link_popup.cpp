#include "youtube_link_popup.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/cocos/base_nodes/Layout.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/ui/TextInput.hpp>
#include <Geode/cocos/platform/CCPlatformMacros.h>

#include <functional>

namespace jukebox {

using UrlCallback = std::function<void(std::string)>;

bool YoutubeLinkPopup::setup(UrlCallback callback) {
    m_urlCallback = callback;
    m_urlInput = TextInput::create(200.0f, "URL");
    m_urlInput->setID("url-input");
    m_urlInput->setCommonFilter(CommonFilter::Any);
    m_urlInput->getInputNode()->setLabelPlaceholderColor(
        ccColor3B {108, 153, 216}
    );
    m_urlInput->getInputNode()->setMaxLabelScale(0.7f);
    m_urlInput->getInputNode()->setLabelPlaceholderScale(0.7f);

    m_mainLayer->addChildAtPosition(
        m_urlInput,
        Anchor::Center
    );

    auto spr = ButtonSprite::create("Ok");
    spr->setScale(0.7f);

    m_okButton = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(YoutubeLinkPopup::onSubmit)
    );
    m_buttonMenu->addChildAtPosition(
        m_okButton,
        Anchor::Bottom,
        { 0.0f, 10.0f }
    );

    return true;
}

void YoutubeLinkPopup::onSubmit(CCObject*) {
    std::string input = m_urlInput->getString();

    if (m_urlCallback) {
        m_urlCallback(input);
    }

    this->onClose(nullptr);
}

YoutubeLinkPopup* YoutubeLinkPopup::create(UrlCallback callback) {
    auto ret = new YoutubeLinkPopup();
    if (ret->initAnchored(
        YoutubeLinkPopup::s_size.width, 
        YoutubeLinkPopup::s_size.height,
        callback
    )) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

}