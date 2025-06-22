#pragma once

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

namespace jukebox {

CCMenuItemSpriteExtra* createSupporterBadge(cocos2d::CCObject* target,
                                            cocos2d::SEL_MenuHandler handler);

}
