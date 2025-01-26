#include <jukebox/ui/indexes_setting.hpp>

#include <memory>
#include <string>
#include <vector>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/cocos/platform/CCPlatformMacros.h>
#include <Geode/Result.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <matjson.hpp>

#include <jukebox/ui/indexes_popup.hpp>

using namespace geode::prelude;

namespace jukebox {

using namespace jukebox::index;

bool Indexes::operator==(const Indexes& other) {
    return this->indexes == other.indexes;
}

SettingNodeV3* IndexSetting::createNode(float width) {
    return IndexSettingNode::create(
        std::static_pointer_cast<IndexSetting>(shared_from_this()), width);
}

bool IndexSettingNode::init(std::shared_ptr<IndexSetting> setting,
                            float width) {
    if (!SettingValueNodeV3::init(setting, width)) {
        return false;
    }

    float height = 40.0f;
    this->setContentSize({width, height});
    CCMenu* menu = CCMenu::create();
    menu->setPosition({0.0f, 0.0f});
    menu->setID("inputs-menu");

    auto viewSpr = ButtonSprite::create("View");
    viewSpr->setScale(0.72f);
    auto viewBtn = CCMenuItemSpriteExtra::create(
        viewSpr, this, menu_selector(IndexSettingNode::onView));
    viewBtn->setPosition(width - 40.f, height - 20.f);
    menu->addChild(viewBtn);

    this->addChild(menu);
    handleTouchPriority(this);

    /*updateVisuals();*/

    return true;
}

void IndexSettingNode::onView(CCObject*) {
    auto setting = this->getSetting();
    IndexesPopup::create(setting->getValue().indexes,
                         [this](std::vector<IndexSource> indexes) {
                             this->setValue(Indexes{indexes}, this);
                         })
        ->show();
}

IndexSettingNode* IndexSettingNode::create(
    std::shared_ptr<IndexSetting> setting, float width) {
    IndexSettingNode* ret = new IndexSettingNode();

    if (ret->init(setting, width)) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

Result<std::shared_ptr<SettingV3>> IndexSetting::parse(
    const std::string& key, const std::string& modID,
    const matjson::Value& json) {
    auto ret = std::make_shared<IndexSetting>();
    auto root = checkJson(json, "IndexSetting");
    ret->parseBaseProperties(key, modID, root);

    root.checkUnknownKeys();

    return root.ok(ret).map([](std::shared_ptr<IndexSetting> value) {
        return std::static_pointer_cast<SettingV3>(value);
    });
}

}  // namespace jukebox
