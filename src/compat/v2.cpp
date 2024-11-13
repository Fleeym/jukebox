#include "compat/v2.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <matjson.hpp>
#include "Geode/Result.hpp"
#include "Geode/loader/Log.hpp"
#include "Geode/loader/Mod.hpp"

#include "compat/compat.hpp"
#include "nong.hpp"
#include "utils/random_string.hpp"

using namespace geode::prelude;

namespace jukebox {

namespace compat {

namespace v2 {

bool isSongValid(const matjson::Value& s) {
    return s.contains("songName") && s["songName"].isString() &&
           s.contains("authorName") && s["authorName"].isString() &&
           s.contains("path") && s["path"].isString();
}

bool manifestExists() { return std::filesystem::exists(manifestPath()); }

std::filesystem::path manifestPath() {
    return Mod::get()->getSaveDir() / "nong_data.json";
}

void backupManifest(bool deleteOrig) {
    if (!manifestExists()) {
        return;
    }

    const std::filesystem::path backupDir =
        Mod::get()->getSaveDir() / ".v2-compat-backup";
    bool exists = std::filesystem::exists(backupDir);

    if (exists && !std::filesystem::is_directory(backupDir)) {
        std::error_code ec;
        std::filesystem::remove_all(backupDir, ec);

        exists = false;
    }

    if (!exists) {
        std::filesystem::create_directory(backupDir);
    }

    std::error_code ec;
    const std::filesystem::path filepath = backupDir / "nong_data.json";
    if (std::filesystem::exists(filepath)) {
        std::filesystem::remove(filepath, ec);
    }
    std::filesystem::copy_file(manifestPath(), filepath, ec);
    if (deleteOrig) {
        std::filesystem::remove(manifestPath(), ec);
    }
}

Result<LocalSong> parseSong(const matjson::Value& i, int id) {
    if (!isSongValid(i)) {
        return Err("Invalid JSON");
    }

    std::filesystem::path path =
        i["path"]
            .asString()
            .map([](std::string v) { return std::filesystem::path(v); })
            .unwrap();

    return Ok(LocalSong(
        SongMetadata(id, jukebox::random_string(16),
                     i["songName"].asString().unwrap(),
                     i["authorName"].asString().unwrap(), std::nullopt,
                     i["startOffset"].asInt().unwrapOr(0)),
        path));
}

Result<LocalSong> getDefault(int id, const std::filesystem::path& defaultPath,
                             const matjson::Value& songs) {
    for (const matjson::Value& i : songs) {
        if (!isSongValid(i)) {
            continue;
        }

        std::filesystem::path path = i["path"].asString().unwrap();
        if (path == defaultPath) {
            return Ok(LocalSong(
                SongMetadata(id, jukebox::random_string(16),
                             i["songName"].asString().unwrap(),
                             i["authorName"].asString().unwrap(), std::nullopt,
                             i["startOffset"].asInt().unwrapOr(0)),
                path));
        }
    }

    return Err("Default song not found");
}

Result<LocalSong> getActive(int id, const std::filesystem::path& activePath,
                            const matjson::Value& songs) {
    for (const matjson::Value& i : songs) {
        if (!isSongValid(i)) {
            continue;
        }

        std::filesystem::path path = i["path"].asString().unwrap();
        if (path == activePath) {
            return Ok(LocalSong(
                SongMetadata(id, jukebox::random_string(16),
                             i["songName"].asString().unwrap(),
                             i["authorName"].asString().unwrap(), std::nullopt,
                             i["startOffset"].asInt().unwrapOr(0)),
                path));
        }
    }

    return Err("Active song not found");
}

Result<std::unordered_map<int, CompatManifest>> parseManifest() {
    if (!manifestExists()) {
        return Err("No manifest exists for V2");
    }

    std::filesystem::path path = manifestPath();

    std::ifstream input(path);
    if (!input.is_open()) {
        return Err(
            fmt::format("Couldn't open file: {}", path.filename().string()));
    }

    std::string contents;
    input.seekg(0, std::ios::end);
    contents.resize(input.tellg());
    input.seekg(0, std::ios::beg);
    input.read(&contents[0], contents.size());
    input.close();

    GEODE_UNWRAP_INTO(matjson::Value json,
                      matjson::parse(std::string_view(contents))
                          .mapErr([](matjson::ParseError err) {
                              return fmt::format(
                                  "Couldn't parse JSON from file: {}", err);
                          }));

    if (!json.contains("version") || !json["version"].isNumber()) {
        return Err("Invalid JSON");
    }

    int version = json["version"].asInt().unwrap();
    if (version < 1 || version > 3) {
        return Err("Invalid JSON");
    }

    if (!json.contains("nongs") || !json["nongs"].isObject()) {
        return Err("Invalid JSON");
    }

    std::unordered_map<int, CompatManifest> ret;

    for (const auto& [key, data] : json["nongs"]) {
        int id = std::stoi(key);

        if (!data.contains("defaultPath") || !data["defaultPath"].isString() ||
            !data.contains("active") || !data["active"].isString() ||
            !data.contains("songs") || !data["songs"].isArray()) {
            log::warn("Skipping id {}, invalid data", id);
            continue;
        }

        std::filesystem::path defaultPath =
            data["defaultPath"].asString().unwrap();
        std::filesystem::path activePath = data["active"].asString().unwrap();
        matjson::Value songs = data["songs"].asArray().unwrap();

        Result<LocalSong> defaultRes = getDefault(id, defaultPath, songs);
        if (defaultRes.isErr()) {
            log::warn("{}", defaultRes.unwrapErr());
            continue;
        }

        Result<LocalSong> activeRes = getActive(id, activePath, songs);
        if (activeRes.isErr()) {
            log::warn("{}", activeRes.unwrapErr());
            continue;
        }

        LocalSong defaultSong = std::move(defaultRes.unwrap());
        LocalSong activeSong = std::move(activeRes.unwrap());
        std::vector<LocalSong> manifestSongs;

        for (const matjson::Value& i : songs) {
            if (!isSongValid(i)) {
                log::warn("Found invalid song. Skipping...");
                continue;
            }

            std::filesystem::path path = i["path"].asString().unwrap();
            std::string unique;

            if (path == defaultPath) {
                unique = defaultSong.metadata()->uniqueID;
            } else if (path == activePath) {
                unique = activeSong.metadata()->uniqueID;
            } else {
                unique = jukebox::random_string(16);
            }

            manifestSongs.push_back(LocalSong(
                SongMetadata(id, unique, i["songName"].asString().unwrap(),
                             i["authorName"].asString().unwrap(), std::nullopt,
                             i["startOffset"].asInt().unwrapOr(0)),
                path));
        }

        ret.insert({id, CompatManifest{.id = id,
                                       .defaultSong = defaultSong,
                                       .active = activeSong,
                                       .songs = std::move(manifestSongs)}});
    }

    return Ok(ret);
}

}  // namespace v2

}  // namespace compat

}  // namespace jukebox
