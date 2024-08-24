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
#include "../random_string.hpp"
#include "index_manager.hpp"


namespace jukebox {

std::optional<Nongs*> NongManager::getNongs(int songID) {
    if (!m_manifest.m_nongs.contains(songID)) {
        return std::nullopt;
    }

    // Try to update saved songs from the index in case it changed
    auto localNongs = m_manifest.m_nongs[songID].get();

    if (!IndexManager::get()->m_indexNongs.contains(songID)) return localNongs;

    Nongs* indexNongs = &IndexManager::get()->m_indexNongs.at(songID);

    bool changed = false;
    for (std::unique_ptr<YTSong>& song : localNongs->youtube()) {
        // Check if song is from an index
        if (!song->indexID().has_value()) continue;
        for (std::unique_ptr<YTSong>& indexSong : indexNongs->youtube()) {
            if (song->indexID() != indexSong->indexID()) continue;
            if (*song->metadata() == *indexSong->metadata() && song->youtubeID() == indexSong->youtubeID()) continue;

            bool deleteAudio = song->youtubeID() != indexSong->youtubeID();

            if (auto res = localNongs->replaceSong(song->metadata()->m_uniqueID, Nong {
                 YTSong {
                    SongMetadata(*indexSong->metadata()),
                    indexSong->youtubeID(),
                    indexSong->indexID(),
                    deleteAudio ? std::nullopt : song->path(),
                },
            }); res.isErr()) {
                SongError(false, "Failed to replace song while updating saved songs from index: {}", res.error()).post();
                continue;
            }
            changed = true;
        }
    }

    for (std::unique_ptr<HostedSong>& song : localNongs->hosted()) {
        // Check if song is from an index
        if (!song->indexID().has_value()) continue;
        for (std::unique_ptr<HostedSong>& indexSong : indexNongs->hosted()) {
            if (song->indexID() != indexSong->indexID()) continue;
            if (*song->metadata() == *indexSong->metadata() && song->url() == indexSong->url()) continue;

            bool deleteAudio = song->url() != indexSong->url();

            if (auto res = localNongs->replaceSong(song->metadata()->m_uniqueID, Nong {
                HostedSong {
                    SongMetadata(*indexSong->metadata()),
                    indexSong->url(),
                    indexSong->indexID(),
                    deleteAudio ? std::nullopt : song->path(),
                },
            }); res.isErr()) {
                SongError(false, "Failed to replace song while updating saved songs from index: {}", res.error()).post();
                continue;
            }
            changed = true;
        }
    }

    if (changed) {
        saveNongs(songID);
    }

    return m_manifest.m_nongs[songID].get();
}

Result<Nong> NongManager::getNongFromManifest(int gdSongID, std::string uniqueID) {
  auto nongs = getNongs(gdSongID);
  if (nongs.has_value()) {
      return Err("Song not initialized in manifest");
  }
  auto nong = nongs.value()->getNongFromID(uniqueID);
  if (!nong.has_value()) {
      return Err("Nong {} not found in song {}", uniqueID, gdSongID);
  }
  return Ok(std::move(nong.value()));
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
                    jukebox::random_string(16),
                    obj->m_songName,
                    obj->m_artistName
                },
                gdDir / "Resources" / filename
            }
        })});
        saveNongs(adjusted);

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
        saveNongs(id);
        return;
    }

    m_manifest.m_nongs.insert({id, std::make_unique<Nongs>(Nongs {
        id,
        LocalSong {
            SongMetadata {
                id,
                jukebox::random_string(16),
                obj->m_songName,
                obj->m_artistName
            },
            MusicDownloadManager::sharedState()->pathForSong(id)
        }
    })});
    saveNongs(id);
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
            auto nongs = result.value();
            auto path = nongs->getNongFromID(nongs->active()).value().path().value();
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

    m_songErrorListener = std::make_unique<EventListener<EventFilter<SongError>>>([this](SongError* event){
        log::error("{}", event->m_error);
        return ListenerResult::Propagate;
    });

    auto path = this->baseManifestPath();
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        return true;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() != ".json") {
            continue;
        }
        log::info("Loading nong from {}", entry.path().string());

        auto res = this->loadNongsFromPath(entry.path());
        if (res.isErr()) {
            log::error("{}", res.unwrapErr());
            std::filesystem::rename(entry.path(), path / fmt::format("{}.bak", entry.path().filename().string()));
            continue;
        }

        m_manifest.m_nongs.insert({ res.unwrap()->songID(), std::move(res.unwrap()) });
    }

    m_initialized = true;
    return true;
}

Result<> NongManager::saveNongs(std::optional<int> saveID) {
    auto path = this->baseManifestPath();

    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }

    for (const auto& entry : m_manifest.m_nongs) {
        auto id = entry.first;

        if (saveID.has_value() && saveID.value() != id) continue;

        auto& nong = entry.second;

        auto json = matjson::Serialize<Nongs>::to_json(*nong);
        if (json.isErr()) {
            return Err(json.error());
        }

        auto filepath = fmt::format("{}/{}.json", path.string(), id);
        std::ofstream output(filepath);
        if (!output.is_open()) {
            return Err(fmt::format("Couldn't open file: {}", filepath));
        }
        output << json.unwrap().dump(matjson::NO_INDENTATION);
        output.close();
    }

    if (saveID.has_value()) {
      SongStateChanged(saveID.value()).post();
    }

    return Ok();
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

    auto& manifestNongs = m_manifest.m_nongs.at(nongs.songID());
    if (auto err = manifestNongs->merge(std::move(nongs)); err.isErr()) {
        return err;
    }
    return saveNongs(manifestNongs->songID());
}

Result<> NongManager::setActiveSong(int gdSongID, std::string uniqueID) {
    auto nongs = getNongs(gdSongID);
    if (!nongs.has_value()) {
        return Err("Song not initialized in manifest");
    }
    if (auto err = nongs.value()->setActive(uniqueID); err.isErr()) {
        return err;
    }
    return saveNongs(gdSongID);
}


Result<> NongManager::deleteAllSongs(int gdSongID) {
  auto nongs = getNongs(gdSongID);
  if (!nongs.has_value()) {
      return Err("Song not initialized in manifest");
  }
  if (auto err = m_manifest.m_nongs.at(gdSongID)->deleteAllSongs(); err.isErr()) {
      return err;
  }
  return saveNongs(gdSongID);
}

Result<> NongManager::deleteSongAudio(int gdSongID, std::string uniqueID) {
    auto nongs = getNongs(gdSongID);
    if (!nongs.has_value()) {
        return Err("Song not initialized in manifest");
    }

    if (auto err = nongs.value()->deleteSongAudio(uniqueID); err.isErr()) {
        return Err("Couldn't delete Nong: {}", err.error());
    }

    return saveNongs(gdSongID);
}

Result<> NongManager::deleteSong(int gdSongID, std::string uniqueID) {
    auto nongs = getNongs(gdSongID);
    if (!nongs.has_value()) {
        return Err("Song not initialized in manifest");
    }

    if (auto err = nongs.value()->deleteSong(uniqueID); err.isErr()) {
        return Err("Couldn't delete Nong: {}", err.error());
    }

    return saveNongs(gdSongID);
}

std::filesystem::path NongManager::generateSongFilePath(const std::string& extension, std::optional<std::string> filename) {
    auto unique = filename.value_or(jukebox::random_string(16));
    auto destination = Mod::get()->getSaveDir() / "nongs";
    if (!std::filesystem::exists(destination)) {
        std::filesystem::create_directory(destination);
    }
    unique += extension;
    destination = destination / unique;
    return destination;
}

};
