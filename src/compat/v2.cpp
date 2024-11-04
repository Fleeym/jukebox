#include "compat/v2.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <matjson.hpp>
#include <vector>
#include "Geode/loader/Log.hpp"
#include "Geode/loader/Mod.hpp"
#include "Geode/utils/Result.hpp"

#include "compat/compat.hpp"
#include "nong.hpp"
#include "utils/random_string.hpp"

using namespace geode::prelude;

namespace jukebox {

namespace compat {

namespace v2 {

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
    if (!i.contains("songName") || !i.contains("authorName") ||
        !i.contains("path")) {
        return Err("Invalid JSON");
    }
    std::filesystem::path path = i["path"].as_string();
    return Ok(LocalSong(
        SongMetadata(id, jukebox::random_string(16), i["songName"].as_string(),
                     i["authorName"].as_string(), std::nullopt,
                     i.contains("startOffset") && i["startOffset"].is_number()
                         ? i["startOffset"].as_int()
                         : 0),
        path));
}

Result<LocalSong> getDefault(int id, const std::filesystem::path& defaultPath,
                             const matjson::Array& songs) {
    for (const matjson::Value& i : songs) {
        if (!i.contains("path") || !i["path"].is_string()) {
            continue;
        }

        std::filesystem::path path = i["path"].as_string();
        if (path == defaultPath) {
            return Ok(LocalSong(
                SongMetadata(
                    id, jukebox::random_string(16), i["songName"].as_string(),
                    i["authorName"].as_string(), std::nullopt,
                    i.contains("startOffset") && i["startOffset"].is_number()
                        ? i["startOffset"].as_int()
                        : 0),
                path));
        }
    }

    return Err("Default song not found");
}

Result<LocalSong> getActive(int id, const std::filesystem::path& activePath,
                            const matjson::Array& songs) {
    for (const matjson::Value& i : songs) {
        if (!i.contains("path") || !i["path"].is_string()) {
            continue;
        }

        std::filesystem::path path = i["path"].as_string();
        if (path == activePath) {
            return Ok(LocalSong(
                SongMetadata(
                    id, jukebox::random_string(16), i["songName"].as_string(),
                    i["authorName"].as_string(), std::nullopt,
                    i.contains("startOffset") && i["startOffset"].is_number()
                        ? i["startOffset"].as_int()
                        : 0),
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

    std::string err;
    std::optional<matjson::Value> res =
        matjson::parse(std::string_view(contents), err);
    if (!res.has_value()) {
        return Err(fmt::format("Couldn't parse JSON from file: {}", err));
    }

    matjson::Value json = res.value();

    if (!json.contains("version") || !json["version"].is_number()) {
        return Err("Invalid JSON");
    }

    int version = json["version"].as_int();
    if (version < 1 || version > 3) {
        return Err("Invalid JSON");
    }

    if (!json.contains("nongs") || !json["nongs"].is_object()) {
        return Err("Invalid JSON");
    }

    std::unordered_map<int, CompatManifest> ret;

    for (const std::pair<std::string, matjson::Value>& kv :
         json["nongs"].as_object()) {
        int id = std::stoi(kv.first);
        matjson::Value data = kv.second;

        if (!data.contains("defaultPath")) {
            log::warn("Skipping id {}, invalid data", id);
            continue;
        }

        if (!data.contains("songs") || !data["songs"].is_array()) {
            log::warn("Skipping id {}, invalid data", id);
            continue;
        }

        std::filesystem::path defaultPath = data["defaultPath"].as_string();
        std::filesystem::path activePath = data["active"].as_string();
        matjson::Array songs = data["songs"].as_array();

        Result<LocalSong> defaultRes = getDefault(id, defaultPath, songs);
        if (defaultRes.isErr()) {
            log::warn("{}", defaultRes.error());
            continue;
        }

        Result<LocalSong> activeRes = getActive(id, activePath, songs);
        if (activeRes.isErr()) {
            log::warn("{}", activeRes.error());
            continue;
        }

        LocalSong defaultSong = std::move(defaultRes.unwrap());
        LocalSong activeSong = std::move(activeRes.unwrap());
        std::vector<LocalSong> manifestSongs;

        for (const matjson::Value& i : songs) {
            if (!i.contains("songName") || !i.contains("authorName") ||
                !i.contains("path")) {
                log::warn("Found invalid song. Skipping...");
                continue;
            }

            std::filesystem::path path = i["path"].as_string();
            std::string unique;

            if (path == defaultPath) {
                unique = defaultSong.metadata()->uniqueID;
            } else if (path == activePath) {
                unique = activeSong.metadata()->uniqueID;
            } else {
                unique = jukebox::random_string(16);
            }

            manifestSongs.push_back(LocalSong(
                SongMetadata(
                    id, unique, i["songName"].as_string(),
                    i["authorName"].as_string(), std::nullopt,
                    i.contains("startOffset") && i["startOffset"].is_number()
                        ? i["startOffset"].as_int()
                        : 0),
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
