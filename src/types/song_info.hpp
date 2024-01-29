#pragma once

#include <Geode/Geode.hpp>
#include <string>

#include "../filesystem.hpp"

using namespace geode::prelude;

struct SongInfo {
    fs::path path;
    std::string songName;
    std::string authorName;
    std::string songUrl;
    std::string levelName;
};

struct NongData {
    fs::path active;
    fs::path defaultPath;
    std::vector<SongInfo> songs;
    bool defaultValid;
};

template<>
struct matjson::Serialize<NongData> {
    static NongData from_json(matjson::Value const& value) {
        std::vector<SongInfo> songs;
        auto jsonSongs = value["songs"].as_array();
        bool valid = true;
        if (value.contains("defaultValid")) {
            valid = value["defaultValid"].as_bool();
        }

        for (auto jsonSong : jsonSongs) {
            std::string levelName = "";
            if (jsonSong.contains("levelName")) {
                levelName = jsonSong["levelName"].as_string();
            }
            auto path = fs::path(jsonSong["path"].as_string());

            SongInfo song = {
                .path = path,
                .songName = jsonSong["songName"].as_string(),
                .authorName = jsonSong["authorName"].as_string(),
                .songUrl = jsonSong["songUrl"].as_string(),
                .levelName = levelName
            };
            songs.push_back(song);
        }

        return NongData {
            .active = fs::path(value["active"].as_string()),
            .defaultPath = fs::path(value["defaultPath"].as_string()),
            .songs = songs,
            .defaultValid = valid
        };
    }

    static matjson::Value to_json(NongData const& value) {
        auto ret = matjson::Object();
        auto array = matjson::Array();
        ret["active"] = value.active.string();
        ret["defaultPath"] = value.defaultPath.string();
        ret["defaultValid"] = value.defaultValid;
        for (auto song : value.songs) {
            auto obj = matjson::Object();
            obj["path"] = song.path.string();
            obj["songName"] = song.songName;
            obj["authorName"] = song.authorName;
            obj["songUrl"] = song.songUrl;
            obj["levelName"] = song.levelName;

            array.push_back(obj);
        }

        ret["songs"] = array;
        return ret;
    }
};