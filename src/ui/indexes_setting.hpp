#pragma once

#include <matjson.hpp>
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/loader/SettingV3.hpp"

#include "../index/index.hpp"
#include "../index/index_serialize.hpp"

using namespace geode::prelude;

namespace jukebox {

using namespace jukebox::index;

struct Indexes {
    std::vector<IndexSource> indexes;
    bool operator==(const Indexes& other);
};

class IndexSetting : public SettingBaseValueV3<Indexes> {
public:
    static Result<std::shared_ptr<IndexSetting>> parse(
        const std::string& key, const std::string& modID,
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
    static bool is_json(const matjson::Value& value) {
        return value.is_array();
    }

    static matjson::Value to_json(const jukebox::Indexes& value) {
        matjson::Array arr;
        for (const jukebox::IndexSource& elem : value.indexes) {
            arr.push_back(
                matjson::Serialize<jukebox::IndexSource>::to_json(elem));
        }
        return arr;
    }

    static jukebox::Indexes from_json(const matjson::Value& value) {
        jukebox::Indexes ret;

        if (!value.is_array()) {
            return ret;
        }

        matjson::Array array = value.as_array();

        for (const matjson::Value& elem : array) {
            ret.indexes.push_back(
                matjson::Serialize<jukebox::IndexSource>::from_json(elem));
        }
        return ret;
    }
};
