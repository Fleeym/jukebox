#pragma once

#include <map>

#include "song_info.hpp"
#include "../manifest.hpp"

struct NongState {
    int m_manifestVersion;
    std::map<int, NongData> m_nongs;
};

template<>
struct matjson::Serialize<NongState> {
    static NongState from_json(matjson::Value const& value) {
        NongState ret;
        ret.m_manifestVersion = value["version"].as_int();
        auto nongs = value["nongs"].as_object();
        for (auto const& kv : nongs) {
            int id = stoi(kv.first);
            NongData data = matjson::Serialize<NongData>::from_json(kv.second);
            ret.m_nongs[id] = data;
        }

        return ret;
    }

    static matjson::Value to_json(NongState const& value) {
        auto ret = matjson::Object();
        auto nongs = matjson::Object();
        ret["version"] = nongd::getManifestVersion();
        for (auto const& kv : value.m_nongs) {
            nongs[std::to_string(kv.first)] = matjson::Serialize<NongData>::to_json(kv.second);
        }
        ret["nongs"] = nongs;
        return ret;
    }
};