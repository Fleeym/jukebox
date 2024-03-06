#pragma once

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <matjson.hpp>

#include "nong.hpp"

namespace nongd {

class SongData {
private:
    Nong* m_active;
    Nong* m_default;
    std::vector<std::unique_ptr<Nong>> m_songs;
    bool m_defaultValid;

    void deleteFileForNong(Nong* nong);
public:
    SongData(
        std::vector<std::unique_ptr<Nong>> songs,
        Nong* activeSong,
        Nong* defaultSong,
        bool defaultValid
    ) : m_active(activeSong),
        m_default(defaultSong),
        m_defaultValid(defaultValid) 
    {
        for (auto& song : songs) {
            m_songs.push_back(std::move(song));
        }
    }

    void addSong(std::unique_ptr<Nong> song);
    void deleteOne(Nong* song);
    void deleteAll();
     
    const Nong* getActive() const { return m_active; }
    const Nong* getDefault() const { return m_default; }
    const bool isDefaultValid() const { return m_defaultValid; }
    const std::vector<std::unique_ptr<Nong>>& getSongs() const { return m_songs; }
};

}

template<>
struct matjson::Serialize<nongd::SongData> {
    static std::optional<std::unique_ptr<nongd::SongData>> from_json(matjson::Value const& value) {
        std::vector<std::unique_ptr<nongd::Nong>> songs;
        auto jsonSongs = value["songs"].as_array();
        songs.reserve(jsonSongs.size());
        bool valid = value.try_get<bool>("defaultValid").value_or(true);

        if (!value.contains("defaultPath") || !value.contains("active")) {
            return std::nullopt;
        }

        std::filesystem::path defaultPath = std::move(value["defaultPath"].as_string());
        std::filesystem::path activePath = std::move(value["active"].as_string());

        nongd::Nong* defaultSong = nullptr;
        nongd::Nong* activeSong = nullptr;

        for (auto jsonSong : jsonSongs) {
            std::string songName = jsonSong.try_get<std::string>("songName").value_or("Unknown");
            std::string authorName = jsonSong.try_get<std::string>("authorName").value_or("Unknown");
            std::string levelName = jsonSong.try_get<std::string>("levelName").value_or("");
            std::string songUrl = jsonSong.try_get<std::string>("songUrl").value_or("");

            auto path = std::filesystem::path(jsonSong["path"].as_string());

            auto song = std::make_unique<nongd::Nong>(nongd::Nong {
                .path = std::move(path),
                .songName = std::move(songName),
                .authorName = std::move(authorName),
                .songUrl = std::move(songUrl),
                .levelName = std::move(levelName)
            });

            if (song->path == defaultPath) {
                defaultSong = song.get();
            }
            if (song->path == activePath) {
                activeSong = song.get();
            }

            songs.push_back(std::move(song));
        }

        return std::make_unique<nongd::SongData>(nongd::SongData {
            std::move(songs),
            activeSong,
            defaultSong,
            valid
        });
    }

    static matjson::Value to_json(nongd::SongData* value) {
        auto ret = matjson::Object();
        auto array = matjson::Array();
        ret["active"] = value->getActive()->path.string();
        ret["defaultPath"] = value->getDefault()->path.string();
        ret["defaultValid"] = value->isDefaultValid();
        for (auto& song : value->getSongs()) {
            auto obj = matjson::Object();
            obj["path"] = song->path.string();
            obj["songName"] = song->songName;
            obj["authorName"] = song->authorName;
            obj["songUrl"] = song->songUrl;
            obj["levelName"] = song->levelName;

            array.push_back(obj);
        }

        ret["songs"] = array;
        return ret;
    }
};