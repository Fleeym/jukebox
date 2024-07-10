// #pragma once
//
// #include <map>
//
// #include "song_info.hpp"
// #include "../manifest.hpp"
//
// namespace jukebox {
//
// struct NongState {
//     int m_manifestVersion;
//     std::map<int, Nongs> m_nongs;
// };
//
// }
//
// template<>
// struct matjson::Serialize<jukebox::NongState> {
//     static jukebox::NongState from_json(matjson::Value const& value) {
//         jukebox::NongState ret;
//         ret.m_manifestVersion = value["version"].as_int();
//         auto nongs = value["nongs"].as_object();
//         for (auto const& kv : nongs) {
//             int id = stoi(kv.first);
//             jukebox::Nongs data = matjson::Serialize<jukebox::Nongs>::from_json(kv.second, id);
//             ret.m_nongs[id] = data;
//         }
//
//         return ret;
//     }
//
//     static matjson::Value to_json(jukebox::NongState const& value) {
//         auto ret = matjson::Object();
//         auto nongs = matjson::Object();
//         ret["version"] = jukebox::getManifestVersion();
//         for (auto const& kv : value.m_nongs) {
//             nongs[std::to_string(kv.first)] = matjson::Serialize<jukebox::Nongs>::to_json(kv.second);
//         }
//         ret["nongs"] = nongs;
//         return ret;
//     }
// };
