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
    static jukebox::NongData from_json(matjson::Value const& value, int songID) {
        bool mainSong = songID < 0;
        std::vector<jukebox::SongInfo> songs;
        auto jsonSongs = value["songs"].as_array();
        bool valid = true;
        if (value.contains("defaultValid")) {
            valid = value["defaultValid"].as_bool();
        }

        std::string defaultFilename = "";
        if (mainSong) {
            geode::log::info("{}", songID);
            defaultFilename = LevelTools::getAudioFileName(-songID - 1);
        } else {
            if (songID < 10000000) {
                // newgrounds songs
                defaultFilename = fmt::format("{}.mp3", songID);
            } else {
                // music library songs
                defaultFilename = fmt::format("{}.ogg", songID);
            }
        }

        std::string activeFilename = "";
        if (!value.contains("activeFilename") && value.contains("active")) {
            auto path = fs::path(value["active"].as_string());
            activeFilename = path.filename().string();
        } else if (value.contains("activeFilename")) {
            activeFilename = value["activeFilename"].as_string();
        } else {
            activeFilename = "nongd:invalid";
        }

        static const fs::path nongDir = geode::Mod::get()->getSaveDir();
        static const fs::path customSongPath = fs::path(
            cocos2d::CCFileUtils::get()->getWritablePath().c_str()
        );
        static const fs::path gdDir = fs::path(
            cocos2d::CCFileUtils::get()->getWritablePath2().c_str()
        );

        fs::path active;
        fs::path defaultPath;
        if (mainSong) {
            defaultPath = gdDir / "Resources" / defaultFilename;
        } else {
            defaultPath = customSongPath / defaultFilename;
        }

        if (activeFilename == defaultFilename) {
            active = defaultPath;
        } else {
            active = nongDir / "nongs" / activeFilename;
        }

        bool activeFound = false;

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
            } else {
                filename = "nongd:invalid";
            }

            fs::path path;
            if (filename == defaultFilename) {
                if (mainSong) {
                    path = gdDir / "Resources" / filename;
                } else {
                    path = customSongPath / filename;
                }
            } else {
                path = nongDir / "nongs" / filename;
            }

            if (filename != "nongd:invalid" && filename == activeFilename) {
                activeFound = true;
                std::error_code ec;
                if (!fs::exists(path, ec) || ec) {
                    activeFound = false;
                }
            }

            jukebox::SongInfo song = {
                .path = path,
                .songName = jsonSong["songName"].as_string(),
                .authorName = jsonSong["authorName"].as_string(),
                .songUrl = jsonSong["songUrl"].as_string(),
                .levelName = levelName
            };
            songs.push_back(song);
        }

        if (!activeFound) {
            active = defaultPath;
        }

        return jukebox::NongData {
            .active = active,
            .defaultPath = defaultPath,
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
        ret["activeFilename"] = value.active.filename().string();
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