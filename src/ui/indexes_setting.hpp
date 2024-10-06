#pragma once

#include "../../include/index.hpp"
#include "../../include/index_serialize.hpp"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/loader/SettingV3.hpp"
#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/loader/Setting.hpp>
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/loader/SettingNode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <matjson.hpp>

using namespace geode::prelude;
using namespace jukebox;

namespace jukebox 
{

struct Indexes {
    std::vector<IndexSource> indexes;
    bool operator==(const Indexes& other);
};

class IndexSetting : public SettingBaseValueV3<Indexes> {
public:
    static Result<std::shared_ptr<IndexSetting>> parse(
        const std::string& key,
        const std::string& modID,
        const matjson::Value& json
    );

    SettingNodeV3* createNode(float width) override;
};

class IndexSettingNode : public SettingValueNodeV3<IndexSetting> {
protected:
    CCMenuItemSpriteExtra* m_resetBtn;

    bool init(std::shared_ptr<IndexSetting> setting, float width);
    void onView(CCObject* sender);
    void onToggle(CCObject* sender);
public:
    static IndexSettingNode* create(
        std::shared_ptr<IndexSetting> setting,
        float width
    );
};

}

template <>
struct geode::SettingTypeForValueType<jukebox::Indexes> {
    using SettingType = IndexSetting;
};

template <>
struct matjson::Serialize<jukebox::Indexes> {
    static bool is_json(const matjson::Value& value) {
        return value.is_array();
    }

    static matjson::Value to_json(const Indexes &value) {
        matjson::Array arr;
        for (const IndexSource& elem : value.indexes) {
            arr.push_back(matjson::Serialize<IndexSource>::to_json(elem));
        }
        return arr;
    }

    static Indexes from_json(const matjson::Value& value) {
        Indexes ret;

        if (!value.is_array()) {
            return ret;
        }

        matjson::Array array = value.as_array();

        for (const matjson::Value& elem : array) {
            ret.indexes.push_back(
                matjson::Serialize<IndexSource>::from_json(elem)
            );
        }
        return ret;
    }
};
