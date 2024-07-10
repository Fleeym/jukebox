#include "nong_manager.hpp"

#include "Geode/binding/LevelTools.hpp"
#include "Geode/binding/MusicDownloadManager.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Log.hpp"

#include <fmt/chrono.h>
#include <filesystem>
#include <fmt/core.h>
#include <matjson.hpp>
#include <memory>
#include <optional>
#include <string_view>
#include <system_error>
#include <string>

#include "../../include/nong.hpp"
#include "../../include/nong_serialize.hpp"

namespace jukebox {

std::optional<Nongs*> NongManager::getNongs(int songID) {
    log::info("manifest: {}", m_manifest.version());
    log::info("id: {}", songID);
    for (const auto& [id, nong] : m_manifest.m_nongs) {
        log::info("id: {}", id);
    }
    if (!m_manifest.m_nongs.contains(songID)) {
        return std::nullopt;
    }

    return m_manifest.m_nongs[songID].get();
}

int NongManager::getCurrentManifestVersion() {
    return m_manifest.m_version;
}

int NongManager::getStoredIDCount() {
    return m_manifest.m_nongs.size();
}

int NongManager::adjustSongID(int id, bool robtop) {
  return robtop ? (id < 0 ? id : -id - 1) : id;
}

void NongManager::initSongID(SongInfoObject* obj, int id, bool robtop) {
    if (m_manifest.m_nongs.contains(id)) {
        return;
    }

    if (!obj && robtop) {
        log::error("Critical. No song object for RobTop song");
        return;
    }

    if (obj && robtop) {
        int adjusted = adjustSongID(id, robtop);
        std::string filename = LevelTools::getAudioFileName(adjusted);
        std::filesystem::path gdDir = CCFileUtils::sharedFileUtils()->getWritablePath2();
        m_manifest.m_nongs.insert({adjusted, std::make_unique<Nongs>(Nongs {
            adjusted,
            LocalSong {
                SongMetadata {
                    adjusted,
                    obj->m_songName,
                    obj->m_artistName
                },
                gdDir / "Resources" / filename
            }
        })});

        return;
    }

    if (!obj) {
        // Try and maybe fetch it
        obj = MusicDownloadManager::sharedState()->getSongInfoObject(id);
    }

    if (!obj) {
        // Try fetch song info from servers
        MusicDownloadManager::sharedState()->getSongInfo(id, true);
        m_manifest.m_nongs.insert({id, std::make_unique<Nongs>(Nongs {
            id,
            LocalSong::createUnknown(id)
        })});
        return;
    }

    m_manifest.m_nongs.insert({id, std::make_unique<Nongs>(Nongs {
        id,
        LocalSong {
            SongMetadata {
                id,
                obj->m_songName,
                obj->m_artistName
            },
            MusicDownloadManager::sharedState()->pathForSong(id)
        }
    })});
}

std::string NongManager::getFormattedSize(const std::filesystem::path& path) {
    std::error_code code;
    auto size = std::filesystem::file_size(path, code);
    if (code) {
        return "N/A";
    }
    double toMegabytes = size / 1024.f / 1024.f;
    std::stringstream ss;
    ss << std::setprecision(3) << toMegabytes << "MB";
    return ss.str();
}

NongManager::MultiAssetSizeTask NongManager::getMultiAssetSizes(std::string songs, std::string sfx) {
    auto resources = std::filesystem::path(CCFileUtils::get()->getWritablePath2())
        / "Resources";
    auto songDir = std::filesystem::path(CCFileUtils::get()->getWritablePath());

    return MultiAssetSizeTask::run([this, songs, sfx, resources, songDir](auto progress, auto hasBeenCanceled) -> MultiAssetSizeTask::Result {
        float sum = 0.f;
        std::istringstream stream(songs);
        std::string s;
        while (std::getline(stream, s, ',')) {
            int id = std::stoi(s);
            auto result = this->getNongs(id);
            if (!result.has_value()) {
                continue;
            }
            auto path = result.value()->active()->path;
            if (path.string().starts_with("songs/")) {
                path = resources / path;
            }
            if (std::filesystem::exists(path)) {
                sum += std::filesystem::file_size(path);
            }
        }
        stream = std::istringstream(sfx);
        while (std::getline(stream, s, ',')) {
            std::stringstream ss;
            ss << "s" << s << ".ogg";
            std::string filename = ss.str();
            auto localPath = resources / "sfx" / filename;
            std::error_code _ec;
            if (std::filesystem::exists(localPath, _ec)) {
                sum += std::filesystem::file_size(localPath);
                continue;
            }
            auto path = songDir / filename;
            if (std::filesystem::exists(path, _ec)) {
                sum += std::filesystem::file_size(path);
            }
        }

        double toMegabytes = sum / 1024.f / 1024.f;
        std::stringstream ss;
        ss << std::setprecision(3) << toMegabytes << "MB";
        return ss.str();
    }, "Multiasset calculation");
}

bool NongManager::init() {
    if (m_initialized) {
        return true;
    }

    auto path = this->baseManifestPath();
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        return true;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() != ".json") {
            continue;
        }

        auto res = this->loadNongsFromPath(entry.path());
        if (res.isErr()) {
            log::error("{}", res.unwrapErr());
            // TODO Backup

            continue;
        }

        m_manifest.m_nongs.insert({ res.unwrap()->songID(), std::move(res.unwrap()) });
    }

    m_initialized = true;
    return true;
}

Result<std::unique_ptr<Nongs>> NongManager::loadNongsFromPath(const std::filesystem::path& path) {
    auto stem = path.stem().string();
    // Watch someone edit a json name and have the game crash
    int id = std::stoi(stem);
    if (id == 0) {
        return Err(fmt::format("Invalid filename {}", path.filename().string()));
    }

    std::ifstream input(path);
    if (!input.is_open()) {
        return Err(fmt::format("Couldn't open file: {}", path.filename().string()));
    }

    // Did some brief research, this seems to be the most efficient method
    // https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html

    std::string contents;
    input.seekg(0, std::ios::end);
    contents.resize(input.tellg());
    input.seekg(0, std::ios::beg);
    input.read(&contents[0], contents.size());
    input.close();

    std::string err;
    auto res = matjson::parse(std::string_view(contents), err);
    if (!res.has_value()) {
        return Err(fmt::format(
            "{}: Couldn't parse JSON from file: {}", id, err
        ));
    }

    auto json = res.value();
    auto nongs = matjson::Serialize<Nongs>::from_json(json, id);
    if (nongs.isErr()) {
        return Err(fmt::format(
            "{}: Failed to parse JSON: {}",
            id,
            nongs.unwrapErr()
        ));
    }

    return Ok(std::make_unique<Nongs>(std::move(nongs.unwrap())));
}

void NongManager::refetchDefault(int songID) {
    MusicDownloadManager::sharedState()->clearSong(songID);
    MusicDownloadManager::sharedState()->getSongInfo(songID, true);
}

Result<> NongManager::addNongs(Nongs&& nongs) {
    if (!m_manifest.m_nongs.contains(nongs.songID())) {
        return Err("Song not initialized in manifest");
    }
    return m_manifest.m_nongs.at(nongs.songID())->merge(std::move(nongs));
}

Result<> NongManager::setActiveSong(const Nongs::ActiveSong& song) {
    if (!m_manifest.m_nongs.contains(song.metadata->m_gdID)) {
        return Err("Song not initialized in manifest");
    }
    return m_manifest.m_nongs.at(song.metadata->m_gdID)->setActive(song.path);
}

Result<> NongManager::deleteSong(const Nongs::ActiveSong& song) {
    if (!m_manifest.m_nongs.contains(song.metadata->m_gdID)) {
        return Err("Song not initialized in manifest");
    }
    return m_manifest.m_nongs.at(song.metadata->m_gdID)->deleteSong(song.path);
}

};
