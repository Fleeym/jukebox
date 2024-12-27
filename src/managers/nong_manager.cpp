#include "managers/nong_manager.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <matjson.hpp>
#include "Geode/Result.hpp"
#include "Geode/binding/LevelTools.hpp"
#include "Geode/binding/MusicDownloadManager.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Log.hpp"

#include "compat/compat.hpp"
#include "compat/v2.hpp"
#include "managers/index_manager.hpp"
#include "nong.hpp"
#include "nong_serialize.hpp"
#include "utils/random_string.hpp"

namespace jukebox {

std::optional<Nongs*> NongManager::getNongs(int songID) {
    if (!m_manifest.m_nongs.contains(songID)) {
        return std::nullopt;
    }

    return m_manifest.m_nongs[songID].get();
}

int NongManager::getCurrentManifestVersion() { return m_manifest.m_version; }

int NongManager::getStoredIDCount() { return m_manifest.m_nongs.size(); }

int NongManager::adjustSongID(int id, bool robtop) {
    return robtop ? (id < 0 ? id : -id - 1) : id;
}

bool NongManager::hasSongID(int id) { return m_manifest.m_nongs.contains(id); }

Result<Nongs*> NongManager::initSongID(SongInfoObject* obj, int id,
                                       bool robtop) {
    int adjusted = this->adjustSongID(id, robtop);

    if (this->hasSongID(adjusted)) {
        return Err("Song already exists");
    }

    if (!obj && robtop) {
        return Err("Critical. No song object for RobTop song");
    }

    if (obj && robtop) {
        std::string filename = LevelTools::getAudioFileName(id);
        std::filesystem::path gdDir = std::filesystem::path(
            CCFileUtils::sharedFileUtils()->getWritablePath2().c_str());
        std::unique_ptr<Nongs> nongs = std::make_unique<Nongs>(
            Nongs{adjusted,
                  LocalSong{SongMetadata{adjusted, jukebox::random_string(16),
                                         obj->m_songName, obj->m_artistName},
                            gdDir / "Resources" / filename}});

        Nongs* n = nongs.get();
        m_manifest.m_nongs.insert({adjusted, std::move(nongs)});
        IndexManager::get().registerIndexNongs(n);

        return Ok(n);
    }

    if (!obj) {
        // See if we already have it stored in the savefile
        obj = MusicDownloadManager::sharedState()->getSongInfoObject(id);
    }

    if (!obj) {
        // Try fetch song info from servers
        MusicDownloadManager::sharedState()->getSongInfo(id, true);
        std::unique_ptr<Nongs> nongs =
            std::make_unique<Nongs>(Nongs{id, LocalSong::createUnknown(id)});

        Nongs* n = nongs.get();
        m_manifest.m_nongs.insert({adjusted, std::move(nongs)});

        IndexManager::get().registerIndexNongs(n);
        return Ok(n);
    }

    // Finally, if obj exists just insert normally
    std::unique_ptr<Nongs> nongs = std::make_unique<Nongs>(Nongs{
        adjusted,
        LocalSong{SongMetadata{adjusted, jukebox::random_string(16),
                               obj->m_songName, obj->m_artistName},
                  std::filesystem::path(MusicDownloadManager::sharedState()
                                            ->pathForSong(id)
                                            .c_str())}});

    Nongs* n = nongs.get();
    m_manifest.m_nongs.insert({adjusted, std::move(nongs)});

    IndexManager::get().registerIndexNongs(n);
    return Ok(n);
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

NongManager::MultiAssetSizeTask NongManager::getMultiAssetSizes(
    std::string songs, std::string sfx) {
    auto resources =
        std::filesystem::path(CCFileUtils::get()->getWritablePath2()) /
        "Resources";
    auto songDir = std::filesystem::path(CCFileUtils::get()->getWritablePath());

    return MultiAssetSizeTask::run(
        [this, songs, sfx, resources, songDir](
            auto progress, auto hasBeenCanceled) -> MultiAssetSizeTask::Result {
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
                auto path = nongs->active()->path().value();
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
        },
        "Multiasset calculation");
}

bool NongManager::init() {
    if (m_initialized) {
        return true;
    }

    m_songErrorListener.bind([this](event::SongError* event) {
        log::error("{}", event->error());
        return ListenerResult::Propagate;
    });

    m_songInfoListener.bind([this](event::GetSongInfo* event) {
        std::optional<Nongs*> nongs = this->getNongs(event->gdSongID());
        if (!nongs.has_value()) {
            return ListenerResult::Stop;
        }
        SongMetadata* defaultSongMetadata =
            nongs.value()->defaultSong()->metadata();
        if (event->songName() == defaultSongMetadata->name &&
            event->artistName() == defaultSongMetadata->artist) {
            return ListenerResult::Stop;
        }

        defaultSongMetadata->name = event->songName();
        defaultSongMetadata->artist = event->artistName();

        (void)this->saveNongs(event->gdSongID());

        return ListenerResult::Propagate;
    });

    log::info("Starting NONG read");

    auto path = this->baseManifestPath();
    if (!std::filesystem::exists(path)) {
        log::info("No manifest directory found. Creating...");
        std::filesystem::create_directory(path);
    }
    auto nongsPath = this->baseNongsPath();
    if (!std::filesystem::exists(nongsPath)) {
        std::filesystem::create_directory(nongsPath);
    }

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() != ".json") {
            continue;
        }

        auto res = this->loadNongsFromPath(entry.path());
        if (res.isErr()) {
            log::error("Failed to read file {}: {}", entry.path().filename(),
                       res.unwrapErr());
            std::filesystem::rename(
                entry.path(),
                path / fmt::format("{}.bak", entry.path().filename().string()));
            continue;
        }

        std::unique_ptr<Nongs> ptr = std::move(res.unwrap());
        int id = ptr->songID();

        m_manifest.m_nongs.insert({id, std::move(ptr)});
    }

    log::info("Read {} files successfuly!", m_manifest.m_nongs.size());

    Result<> res = this->migrateV2();
    if (res.isErr()) {
        log::error("{}", res.unwrapErr());
    }

    m_initialized = true;
    return true;
}

Result<> NongManager::migrateV2() {
    bool migrate = compat::v2::manifestExists();

    if (!migrate) {
        log::info("Nothing to migrate from V2!");
        return Ok();
    }

    using CompatMap = std::unordered_map<int, compat::CompatManifest>;
    GEODE_UNWRAP_INTO(CompatMap manifest, compat::v2::parseManifest());

    size_t i = 0;

    for (const auto& kv : manifest) {
        if (!m_manifest.m_nongs.contains(kv.first)) {
            LocalSong defaultSong = kv.second.defaultSong;
            int id = kv.first;
            Nongs nongs = Nongs(kv.first, std::move(defaultSong));
            m_manifest.m_nongs.insert(
                {id, std::make_unique<Nongs>(std::move(nongs))});

            Nongs* n = m_manifest.m_nongs[kv.first].get();
            IndexManager::get().registerIndexNongs(n);
        }

        Nongs* nongs = m_manifest.m_nongs[kv.first].get();

        for (const LocalSong& i : kv.second.songs) {
            if (i.path().value() == kv.second.defaultSong.path().value()) {
                continue;
            }
            bool found = false;
            for (std::unique_ptr<LocalSong>& stored : nongs->locals()) {
                if (stored->metadata()->startOffset ==
                        stored->metadata()->startOffset &&
                    stored->metadata()->name == i.metadata()->name &&
                    stored->metadata()->artist == i.metadata()->artist) {
                    found = true;
                }
            }

            if (found) {
                continue;
            }

            LocalSong cp = i;
            auto res = nongs->add(std::move(cp));
            if (res.isErr()) {
                log::error("Failed to add migrated song to manifest: {}",
                           res.unwrapErr());
            }
        }

        (void)nongs->setActive(kv.second.active.metadata()->uniqueID);
        (void)nongs->commit();
        i++;
    }

    log::info("Migrated {} ids from v2", i);
    (void)compat::v2::backupManifest(true);

    return Ok();
}

Result<> NongManager::saveNongs(std::optional<int> saveID) {
    auto path = this->baseManifestPath();

    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }

    for (const auto& entry : m_manifest.m_nongs) {
        auto id = entry.first;

        if (saveID.has_value() && saveID.value() != id) {
            continue;
        }

        GEODE_UNWRAP(entry.second->commit());
    }

    return Ok();
}

Result<std::unique_ptr<Nongs>> NongManager::loadNongsFromPath(
    const std::filesystem::path& path) {
    auto stem = path.stem().string();
    // Watch someone edit a json name and have the game crash
    int id = std::stoi(stem);
    if (id == 0) {
        return Err(
            fmt::format("Invalid filename {}", path.filename().string()));
    }

    std::ifstream input(path);
    if (!input.is_open()) {
        return Err(
            fmt::format("Couldn't open file: {}", path.filename().string()));
    }

    // Did some brief research, this seems to be the most efficient method
    // https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html

    std::string contents;
    input.seekg(0, std::ios::end);
    contents.resize(input.tellg());
    input.seekg(0, std::ios::beg);
    input.read(contents.data(), contents.size());
    input.close();

    GEODE_UNWRAP_INTO(matjson::Value json,
                      matjson::parse(contents).mapErr([id](std::string err) {
                          return fmt::format(
                              "Couldn't parse JSON from file: {}", err);
                      }));

    GEODE_UNWRAP_INTO(Nongs nongs,
                      matjson::Serialize<Nongs>::fromJson(json, id).mapErr(
                          [id](std::string err) {
                              return fmt::format("Failed to parse JSON: {}",
                                                 err);
                          }));

    return Ok(std::make_unique<Nongs>(std::move(nongs)));
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
    if (auto err = m_manifest.m_nongs.at(gdSongID)->deleteAllSongs();
        err.isErr()) {
        return err;
    }
    return saveNongs(gdSongID);
}

Result<> NongManager::deleteSongAudio(int gdSongID, std::string uniqueID) {
    auto nongs = getNongs(gdSongID);
    if (!nongs.has_value()) {
        return Err("Song not initialized in manifest");
    }

    GEODE_UNWRAP(
        nongs.value()->deleteSongAudio(uniqueID).mapErr([](std::string err) {
            return fmt::format("Couldn't delete Nong: {}", err);
        }));

    return saveNongs(gdSongID);
}

Result<> NongManager::deleteSong(int gdSongID, std::string uniqueID) {
    std::optional<Nongs*> nongs = this->getNongs(gdSongID);
    if (!nongs.has_value()) {
        return Err("Song not initialized in manifest");
    }

    GEODE_UNWRAP(
        nongs.value()->deleteSong(uniqueID).mapErr([](std::string err) {
            return fmt::format("Couldn't delete Nong: {}", err);
        }));

    return nongs.value()->commit();
}

std::filesystem::path NongManager::generateSongFilePath(
    const std::string& extension, std::optional<std::string> filename) {
    auto unique = filename.value_or(jukebox::random_string(16));
    auto destination = Mod::get()->getSaveDir() / "nongs";
    if (!std::filesystem::exists(destination)) {
        std::filesystem::create_directory(destination);
    }
    unique += extension;
    destination = destination / unique;
    return destination;
}

};  // namespace jukebox
