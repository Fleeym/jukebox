#pragma once

#include "Geode/loader/Mod.hpp"
#include <matjson.hpp>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

namespace jukebox {

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

}

template<>
struct matjson::Serialize<jukebox::NongData> {
    static jukebox::NongData from_json(matjson::Value const& value) {
        std::vector<jukebox::SongInfo> songs;
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
            std::string filename = "";
            if (!jsonSong.contains("filename") && jsonSong.contains("path")) {
                auto path = fs::path(jsonSong["path"].as_string());
                filename = path.filename().string();
            } else if (jsonSong.contains("filename")) {
                filename = jsonSong["filename"].as_string();
            }

            static const fs::path nongDir = fs::path(geode::Mod::get()->getSaveDir());
            fs::path path = nongDir / "nongs" / filename;

            jukebox::SongInfo song = {
                .path = path,
                .songName = jsonSong["songName"].as_string(),
                .authorName = jsonSong["authorName"].as_string(),
                .songUrl = jsonSong["songUrl"].as_string(),
                .levelName = levelName
            };
            songs.push_back(song);
        }

        return jukebox::NongData {
            .active = fs::path(value["active"].as_string()),
            .defaultPath = fs::path(value["defaultPath"].as_string()),
            .songs = songs,
            .defaultValid = valid
        };
    }

    static matjson::Value to_json(jukebox::NongData const& value) {
        auto ret = matjson::Object();
        auto array = matjson::Array();
        ret["active"] = value.active.string();
        ret["defaultPath"] = value.defaultPath.string();
        ret["defaultValid"] = value.defaultValid;
        for (auto song : value.songs) {
            auto obj = matjson::Object();
            obj["path"] = song.path.string();
            obj["filename"] = song.path.filename().string();
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