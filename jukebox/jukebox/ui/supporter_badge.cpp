#include <jukebox/ui/supporter_badge.hpp>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

namespace jukebox {

CCMenuItemSpriteExtra* createSupporterBadge(CCObject* target,
                                            SEL_MenuHandler handler) {
    CCSprite* badgeSpr =
        CCSprite::createWithSpriteFrameName("JB_SupporterBadge.png"_spr);
    if (!badgeSpr) {
        return nullptr;
    }
    badgeSpr->setScale(0.5f);

    auto badge = CCMenuItemSpriteExtra::create(badgeSpr, target, handler);
    if (!badge) {
        return nullptr;
    }

    badge->setID("supporter-badge"_spr);
    return badge;
}

}  // namespace jukebox
