#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/ProfilePage.hpp>

#include <jukebox/ui/supporter_badge.hpp>

using namespace geode::prelude;

class $modify(JBProfilePage, ProfilePage) {
    $override void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);
        const auto username =
            typeinfo_cast<CCMenu*>(m_mainLayer->querySelector("username-menu"));

        if (!username) {
            return;
        }

        CCMenuItemSpriteExtra* badge = jukebox::createSupporterBadge(
            this, menu_selector(JBProfilePage::onSupporterBadge));
        if (!badge) {
            return;
        }

        username->addChild(badge);
        username->updateLayout();
    }

    void onSupporterBadge(CCObject*) {
        FLAlertLayer::create(
            "Supporter",
            "This player is a <co>Song File Hub</c> <cb>supporter</c>!", "Ok")
            ->show();
    }
};
