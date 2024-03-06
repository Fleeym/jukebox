#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include <matjson.hpp>
#include <utility>

#include "song_data.hpp"
#include "../manifest.hpp"

namespace nongd {

struct ModState {
    int m_manifestVersion;
    std::unordered_map<int, std::unique_ptr<SongData>> m_nongs;
};

}

template<>
struct matjson::Serialize<nongd::ModState> {
    static std::unique_ptr<nongd::ModState> from_json(matjson::Value const& value) {
        auto ret = std::make_unique<nongd::ModState>();
        ret->m_manifestVersion = value["version"].as_int();
        auto nongs = value["nongs"].as_object();
        for (auto const& kv : nongs) {
            int id = stoi(kv.first);
            if (std::optional<std::unique_ptr<nongd::SongData>> data = matjson::Serialize<nongd::SongData>::from_json(kv.second)) {
                ret->m_nongs.insert(std::make_pair(id, std::move(data.value())));
            } else {
                // TODO write code for song IDs not being loaded
            }
        }

        return ret;
    }

    static matjson::Value to_json(nongd::ModState* value) {
        auto ret = matjson::Object();
        auto nongs = matjson::Object();
        ret["version"] = nongd::getManifestVersion();
        for (auto const& kv : value->m_nongs) {
            nongs.insert(std::make_pair(
                std::to_string(kv.first),
                matjson::Serialize<nongd::SongData>::to_json(kv.second.get())
            ));
        }
        ret["nongs"] = nongs;
        return ret;
    }
};
