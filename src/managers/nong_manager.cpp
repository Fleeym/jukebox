#include "managers/nong_manager.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <matjson.hpp>
#include "Geode/binding/LevelTools.hpp"
#include "Geode/binding/MusicDownloadManager.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Log.hpp"

#include "events/song_state_changed.hpp"
#include "managers/index_manager.hpp"
#include "nong.hpp"
#include "nong_serialize.hpp"
#include "utils/random_string.hpp"

namespace jukebox {

std::optional<Nongs*> NongManager::getNongs(int songID) {
    if (!m_manifest.m_nongs.contains(songID)) {
        return std::nullopt;
    }

    // Try to update saved songs from the index in case it changed
    return m_manifest.m_nongs[songID].get();

    /*if (!IndexManager::get().m_indexNongs.contains(songID)) {*/
    /*    return localNongs;*/
    /*}*/
    /**/
    /*Nongs* indexNongs = &IndexManager::get().m_indexNongs.at(songID);*/
    /**/
    /*bool changed = false;*/
    /**/
    /*for (std::unique_ptr<YTSong>& song : localNongs->youtube()) {*/
    /*    // Check if song is from an index*/
    /*    if (!song->indexID().has_value()) {*/
    /*        continue;*/
    /*    }*/
    /*    for (std::unique_ptr<YTSong>& indexSong : indexNongs->youtube()) {*/
    /*        if (song->indexID() != indexSong->indexID()) {*/
    /*            continue;*/
    /*        }*/
    /*        auto metadata = song->metadata();*/
    /*        auto indexMetadata = indexSong->metadata();*/
    /*        if (metadata->uniqueID != indexMetadata->uniqueID) {*/
    /*            continue;*/
    /*        }*/
    /*        if (metadata->gdID == indexMetadata->gdID &&*/
    /*            metadata->name == indexMetadata->name &&*/
    /*            metadata->artist == indexMetadata->artist &&*/
    /*            metadata->level == indexMetadata->level &&*/
    /*            metadata->startOffset == indexMetadata->startOffset &&*/
    /*            song->youtubeID() == indexSong->youtubeID()) {*/
    /*            continue;*/
    /*        }*/
    /**/
    /*        bool deleteAudio = song->youtubeID() != indexSong->youtubeID();*/
    /**/
    /*        if (auto res = localNongs->replaceSong(*/
    /*                song->metadata()->uniqueID,*/
    /*                YTSong{*/
    /*                    SongMetadata(*indexSong->metadata()),*/
    /*                    indexSong->youtubeID(),*/
    /*                    indexSong->indexID(),*/
    /*                    deleteAudio ? std::nullopt : song->path(),*/
    /*                });*/
    /*            res.isErr()) {*/
    /*            SongErrorEvent(false,*/
    /*                           "Failed to replace song while updating saved
     * "*/
    /*                           "songs from index: {}",*/
    /*                           res.error())*/
    /*                .post();*/
    /*            continue;*/
    /*        }*/
    /*        changed = true;*/
    /*    }*/
    /*}*/
    /**/
    /*for (std::unique_ptr<HostedSong>& song : localNongs->hosted()) {*/
    /*    // Check if song is from an index*/
    /*    if (!song->indexID().has_value()) {*/
    /*        continue;*/
    /*    }*/
    /*    for (std::unique_ptr<HostedSong>& indexSong : indexNongs->hosted())
     * {*/
    /*        if (song->indexID() != indexSong->indexID()) {*/
    /*            continue;*/
    /*        }*/
    /*        auto metadata = song->metadata();*/
    /*        auto indexMetadata = indexSong->metadata();*/
    /*        if (metadata->uniqueID != indexMetadata->uniqueID) {*/
    /*            continue;*/
    /*        }*/
    /*        if (metadata->gdID == indexMetadata->gdID &&*/
    /*            metadata->name == indexMetadata->name &&*/
    /*            metadata->artist == indexMetadata->artist &&*/
    /*            metadata->level == indexMetadata->level &&*/
    /*            metadata->startOffset == indexMetadata->startOffset &&*/
    /*            song->url() == indexSong->url()) {*/
    /*            continue;*/
    /*        }*/
    /**/
    /*        bool deleteAudio = song->url() != indexSong->url();*/
    /**/
    /*        if (auto res = localNongs->replaceSong(*/
    /*                song->metadata()->uniqueID,*/
    /*                HostedSong{*/
    /*                    SongMetadata(*indexSong->metadata()),*/
    /*                    indexSong->url(),*/
    /*                    indexSong->indexID(),*/
    /*                    deleteAudio ? std::nullopt : song->path(),*/
    /*                });*/
    /*            res.isErr()) {*/
    /*            SongErrorEvent(false,*/
    /*                           "Failed to replace song while updating saved
     * "*/
    /*                           "songs from index: {}",*/
    /*                           res.error())*/
    /*                .post();*/
    /*            continue;*/
    /*        }*/
    /*        changed = true;*/
    /*    }*/
    /*}*/
    /**/
    /*if (changed) {*/
    /*    (void)this->saveNongs(songID);*/
    /*}*/
    /**/
    /*return m_manifest.m_nongs[songID].get();*/
}

int NongManager::getCurrentManifestVersion() { return m_manifest.m_version; }

int NongManager::getStoredIDCount() { return m_manifest.m_nongs.size(); }

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
        std::filesystem::path gdDir = std::filesystem::path(
            CCFileUtils::sharedFileUtils()->getWritablePath2().c_str());
        m_manifest.m_nongs.insert(
            {adjusted,
             std::make_unique<Nongs>(Nongs{
                 adjusted,
                 LocalSong{SongMetadata{adjusted, jukebox::random_string(16),
                                        obj->m_songName, obj->m_artistName},
                           gdDir / "Resources" / filename}})});
        (void)this->saveNongs(adjusted);

        return;
    }

    if (!obj) {
        // Try and maybe fetch it
        obj = MusicDownloadManager::sharedState()->getSongInfoObject(id);
    }

    if (!obj) {
        // Try fetch song info from servers
        MusicDownloadManager::sharedState()->getSongInfo(id, true);
        m_manifest.m_nongs.insert({id, std::make_unique<Nongs>(Nongs{
                                           id, LocalSong::createUnknown(id)})});
        (void)this->saveNongs(id);
        return;
    }

    m_manifest.m_nongs.insert(
        {id, std::make_unique<Nongs>(Nongs{
                 id, LocalSong{SongMetadata{id, jukebox::random_string(16),
                                            obj->m_songName, obj->m_artistName},
                               std::filesystem::path(
                                   MusicDownloadManager::sharedState()
                                       ->pathForSong(id)
                                       .c_str())}})});
    (void)this->saveNongs(id);
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
        log::info("Song info event for {}", event->gdSongID());
        log::info("info: {} - {}", event->songName(), event->artistName());

        auto nongs = getNongs(event->gdSongID());
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

        log::info("got here?");

        (void)this->saveNongs(event->gdSongID());

        return ListenerResult::Propagate;
    });

    auto path = this->baseManifestPath();
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        return true;
    }

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() != ".json") {
            continue;
        }
        log::info("Loading nongs from {}", entry.path().string());

        auto res = this->loadNongsFromPath(entry.path());
        if (res.isErr()) {
            log::error("{}", res.unwrapErr());
            std::filesystem::rename(
                entry.path(),
                path / fmt::format("{}.bak", entry.path().filename().string()));
            continue;
        }

        std::unique_ptr<Nongs> ptr = std::move(res.unwrap());
        int id = ptr->songID();

        m_manifest.m_nongs.insert({id, std::move(ptr)});
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

        if (saveID.has_value() && saveID.value() != id) {
            continue;
        }

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
        event::SongStateChanged(saveID.value()).post();
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
    input.read(&contents[0], contents.size());
    input.close();

    std::string err;
    auto res = matjson::parse(std::string_view(contents), err);
    if (!res.has_value()) {
        return Err(
            fmt::format("{}: Couldn't parse JSON from file: {}", id, err));
    }

    auto json = res.value();
    auto nongs = matjson::Serialize<Nongs>::from_json(json, id);
    if (nongs.isErr()) {
        return Err(
            fmt::format("{}: Failed to parse JSON: {}", id, nongs.unwrapErr()));
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

    if (auto err = nongs.value()->deleteSongAudio(uniqueID); err.isErr()) {
        return Err("Couldn't delete Nong: {}", err.error());
    }

    return saveNongs(gdSongID);
}

Result<> NongManager::deleteSong(int gdSongID, std::string uniqueID) {
    std::optional<Nongs*> nongs = this->getNongs(gdSongID);
    if (!nongs.has_value()) {
        return Err("Song not initialized in manifest");
    }

    if (auto err = nongs.value()->deleteSong(uniqueID); err.isErr()) {
        return Err("Couldn't delete Nong: {}", err.error());
    }

    return saveNongs(gdSongID);
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
