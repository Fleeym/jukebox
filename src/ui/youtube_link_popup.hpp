#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Popup.hpp>
#include <functional>

using namespace geode::prelude;

namespace jukebox {

class YoutubeLinkPopup : public Popup<std::function<void(std::string)>> {
protected:
    using UrlCallback = std::function<void(std::string)>;

    static inline CCSize s_size = { 300.f, 120.f };

    UrlCallback m_urlCallback = {};
    TextInput* m_urlInput;
    CCMenuItemSpriteExtra* m_okButton;

    bool setup(UrlCallback callback) override;
    void onSubmit(CCObject*);
public:
    static YoutubeLinkPopup* create(UrlCallback callback);
};

}