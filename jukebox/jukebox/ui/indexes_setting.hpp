#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/Result.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <matjson.hpp>

#include <jukebox/nong/index.hpp>
#include <jukebox/nong/index_serialize.hpp>

using namespace geode::prelude;

namespace jukebox {

using namespace jukebox::index;

struct Indexes {
    std::vector<IndexSource> indexes;
    bool operator==(const Indexes& other);
};

class IndexSetting : public SettingBaseValueV3<Indexes> {
public:
    static Result<std::shared_ptr<SettingV3>> parse(const std::string& key,
                                                    const std::string& modID,
                                                    const matjson::Value& json);

    SettingNodeV3* createNode(float width) override;
};

class IndexSettingNode : public SettingValueNodeV3<IndexSetting> {
protected:
    CCMenuItemSpriteExtra* m_resetBtn;

    bool init(std::shared_ptr<IndexSetting> setting, float width);
    void onView(CCObject* sender);
    void onToggle(CCObject* sender);

public:
    static IndexSettingNode* create(std::shared_ptr<IndexSetting> setting,
                                    float width);
};

}  // namespace jukebox

template <>
struct geode::SettingTypeForValueType<jukebox::Indexes> {
    using SettingType = jukebox::IndexSetting;
};

template <>
struct matjson::Serialize<jukebox::Indexes> {
    static matjson::Value toJson(const jukebox::Indexes& value) {
        matjson::Value arr = matjson::Value::array();
        for (const jukebox::IndexSource& elem : value.indexes) {
            arr.push(matjson::Serialize<jukebox::IndexSource>::toJson(elem));
        }
        return arr;
    }

    static geode::Result<jukebox::Indexes> fromJson(
        const matjson::Value& value) {
        jukebox::Indexes ret;

        for (const matjson::Value& elem : value) {
            GEODE_UNWRAP_INTO(
                jukebox::IndexSource source,
                matjson::Serialize<jukebox::IndexSource>::fromJson(elem));
            ret.indexes.push_back(source);
        }
        return Ok(ret);
    }
};
