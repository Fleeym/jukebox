#pragma once

#include "Geode/loader/Log.hpp"
#include <Geode/binding/LevelTools.hpp>
#include <Geode/cocos/platform/CCFileUtils.h>
#include <Geode/loader/Mod.hpp>
#include <fmt/format.h>
#include <matjson.hpp>
#include <string>
#include <filesystem>
#include <system_error>

namespace jukebox {

struct Index {
    std::string url;
    bool userAdded;
    bool enabled;

    bool operator==(Index const& other) const {
        return url == other.url && userAdded == other.userAdded && enabled == other.enabled;
    }
};

}

template<>
struct matjson::Serialize<jukebox::Index> {
    static jukebox::Index from_json(matjson::Value const& value) {
        return jukebox::Index {
            .url = value["url"].as_string(),
            .userAdded = value["userAdded"].as_bool(),
            .enabled = value["enabled"].as_bool()
        };
    }

    static matjson::Value to_json(jukebox::Index const& value) {
        auto indexObject = matjson::Object();
        indexObject["url"] = value.url;
        indexObject["userAdded"] = value.userAdded;
        indexObject["enabled"] = value.enabled;
        return indexObject;
    }
};
