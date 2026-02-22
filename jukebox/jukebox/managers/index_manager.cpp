#include <jukebox/managers/index_manager.hpp>

#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <Geode/Result.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/utils/web.hpp>
#include <arc/future/Future.hpp>
#include <matjson.hpp>

#include <jukebox/download/hosted.hpp>
#include <jukebox/events/file_download_progress.hpp>
#include <jukebox/events/song_download_failed.hpp>
#include <jukebox/events/song_download_finished.hpp>
#include <jukebox/events/song_download_progress.hpp>
#include <jukebox/events/song_error.hpp>
#include <jukebox/events/start_download.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/nong/index_serialize.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/ui/indexes_setting.hpp>

using namespace geode::prelude;
using namespace jukebox::index;
using namespace arc;

namespace jukebox {

bool IndexManager::init() {
    if (m_initialized) {
        return true;
    }

    if (const std::filesystem::path path = this->baseIndexesPath(); !std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        return true;
    }

    async::spawn(this->fetchIndexes(), [](Result<> result) {
        if (GEODE_UNWRAP_IF_ERR(err, result)) {
            log::error("Failed to start fetching indexes: {}", err);
        }
    });

    event::StartDownload()
        .listen([this](const event::StartDownloadData& event) {
            Result<> res = this->downloadSong(event.gdId(), event.uniqueID());
            if (res.isErr()) {
                event::SongDownloadFailed().send(
                    event::SongDownloadFailedData{event.gdId(), std::string(event.uniqueID()), res.unwrapErr()});
            }
        })
        .leak();

    events::FileDownloadProgress()
        .listen([this](const events::FileDownloadProgressData& event) {
            const auto url = std::string(event.url());
            if (const auto& it = m_urlToIDs.find(url); it != m_urlToIDs.end()) {
                const int gdId = std::get<int>(it->second);
                auto uniqueID = std::get<std::string>(it->second);
                event::SongDownloadProgress(gdId).send(
                    event::SongDownloadProgressData{gdId, std::move(uniqueID), event.progress()});
            }
        })
        .leak();

    m_initialized = true;
    return true;
}

Result<std::vector<IndexSource>> IndexManager::getIndexes() {
    auto setting = Mod::get()->getSettingValue<Indexes>("indexes");
    return Ok(setting.indexes);
}

std::filesystem::path IndexManager::baseIndexesPath() {
    static std::filesystem::path path = Mod::get()->getSaveDir() / "indexes-cache";
    return path;
}

Result<> IndexManager::loadIndex(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        return Err("Index file does not exist");
    }

    std::ifstream input(path);
    if (!input.is_open()) {
        return Err(fmt::format("Couldn't open file: {}", path.filename().string()));
    }

    GEODE_UNWRAP_INTO(matjson::Value jsonObj, matjson::parse(input));

    return this->loadIndex(std::move(jsonObj));
}

Result<> IndexManager::loadIndex(matjson::Value&& jsonObj) {
    GEODE_UNWRAP_INTO(auto indexMeta, jsonObj.as<IndexMetadata>());
    auto index = std::make_unique<IndexMetadata>(std::move(indexMeta));

    this->cacheIndexName(index->m_id, index->m_name);

    // TODO: re-enable youtube downloads at a later date
    /*for (const auto& [key, ytNong] : jsonObj["nongs"]["youtube"].as_object())
     * {*/
    /*    Result<IndexSongMetadata> r =*/
    /*        matjson::Serialize<IndexSongMetadata>::from_json(ytNong);*/
    /*    if (r.isErr()) {*/
    /*        event::SongError(*/
    /*            false, fmt::format("Failed to parse index song: {}",
     * r.error()))*/
    /*            .post();*/
    /*        continue;*/
    /*    }*/
    /*    std::unique_ptr<IndexSongMetadata> song =*/
    /*        std::make_unique<IndexSongMetadata>(r.unwrap());*/
    /*    song->uniqueID = key;*/
    /*    song->parentID = index.get();*/
    /**/
    /*    for (int id : song->songIDs) {*/
    /*        if (!m_nongsForId.contains(id)) {*/
    /*            m_nongsForId[id] = {song.get()};*/
    /*        } else {*/
    /*            m_nongsForId[id].push_back(song.get());*/
    /*        }*/
    /**/
    /*        std::optional<Nongs*> opt = NongManager::get().getNongs(id);*/
    /*        if (!opt.has_value()) {*/
    /*            continue;*/
    /*        }*/
    /**/
    /*        Nongs* nongs = opt.value();*/
    /**/
    /*        if (geode::Result<> r = nongs->registerIndexSong(song.get());*/
    /*            r.isErr()) {*/
    /*            event::SongError(*/
    /*                false,*/
    /*                fmt::format("Failed to register index song: {}",
     * r.error()))*/
    /*                .post();*/
    /*        }*/
    /*    }*/
    /*    index->m_songs.m_youtube.push_back(std::move(song));*/
    /*}*/

    for (const auto& [key, hostedNong] : jsonObj["nongs"]["hosted"]) {
        GEODE_UNWRAP_OR_ELSE(r, err, hostedNong.as<IndexSongMetadata>()) {
            event::SongError().send(event::SongErrorData{false, fmt::format("Failed to parse index song: {}", err)});
            continue;
        }

        auto song = std::make_unique<IndexSongMetadata>(std::move(r));

        song->uniqueID = key;
        song->parentID = index.get();

        for (int id : song->songIDs) {
            if (!m_nongsForId.contains(id)) {
                m_nongsForId[id] = {song.get()};
            } else {
                m_nongsForId[id].push_back(song.get());
            }
            std::optional<Nongs*> opt = NongManager::get().getNongs(id);
            if (!opt.has_value()) {
                continue;
            }

            Nongs* nongs = opt.value();

            if (GEODE_UNWRAP_IF_ERR(err, nongs->registerIndexSong(song.get()))) {
                event::SongError().send(
                    event::SongErrorData{false, fmt::format("Failed to register index song: {}", err)});
            }
        }

        index->m_songs.m_hosted.push_back(std::move(song));
    }

    IndexMetadata* ref = index.get();
    m_loadedIndexes.emplace(ref->m_id, std::move(index));

    return Ok();
}

Future<Result<>> IndexManager::fetchIndexes() {
    ARC_CO_UNWRAP_INTO(const std::vector<IndexSource> indexes, this->getIndexes());

    for (const IndexSource& index : indexes) {
        if (!index.m_enabled || index.m_url.size() < 3) {
            if (!index.m_userAdded) {
                log::warn(
                    "Skipping default index {} provided by Jukebox, as it is "
                    "disabled",
                    index.m_url);
            } else {
                log::info("Skipping index {}, as it is disabled", index.m_url);
            }
            continue;
        }

        const std::string url = index.m_url;

        log::info("Starting fetch for index {}", index.m_url);

        Result<matjson::Value> fetchedIndex = co_await this->fetchIndex(index);

        if (GEODE_UNWRAP_EITHER(value, err, fetchedIndex)) {
            co_await this->onIndexFetched(url, std::move(value));
        } else {
            log::error("Failed to fetch index {}: {}", index.m_url, err);
        }
    }

    co_return Ok();
}

Future<Result<matjson::Value>> IndexManager::fetchIndex(const IndexSource& index) {
    const web::WebResponse response = co_await web::WebRequest().timeout(std::chrono::seconds(30)).get(index.m_url);

    if (!response.ok()) {
        co_return Err(fmt::format("Web request failed. Status code: {}", response.code()));
    }

    ARC_CO_UNWRAP_INTO(matjson::Value jsonObj, response.json());

    jsonObj.set("url", index.m_url);

    ARC_CO_UNWRAP(matjson::Serialize<IndexMetadata>::fromJson(jsonObj));

    co_return Ok(std::move(jsonObj));
}

Future<> IndexManager::onIndexFetched(const std::string& url, matjson::Value&& json) {
    static constexpr std::hash<std::string> hasher;
    std::size_t hashValue = hasher(url);

    const std::filesystem::path filepath = this->baseIndexesPath() / fmt::format("{0:x}.json", hashValue);

    log::info("Fetched index: {}", url);

    if (std::ofstream out(filepath); out.is_open()) {
        const std::string str = json.dump(matjson::NO_INDENTATION);
        out.write(str.data(), str.length());
        log::info("Cached index: {}", url);
    } else {
        log::error("Failed to cache index: {}", url);
    }

    this->loadIndex(std::move(json)).inspectErr([url](const std::string& err) {
        log::error("Failed to load index {}: {}", url, err);
    });

    co_return;
}

std::optional<std::string> IndexManager::getIndexName(const std::string& indexID) {
    auto jsonObj = Mod::get()->getSavedValue<matjson::Value>("cached-index-names");
    if (!jsonObj.contains(indexID)) {
        return std::nullopt;
    }
    return jsonObj[indexID].asString().mapOr<std::optional<std::string>>(std::nullopt,
                                                                         [](auto i) { return std::optional(i); });
}

void IndexManager::cacheIndexName(const std::string& indexId, const std::string& indexName) {
    auto jsonObj = Mod::get()->getSavedValue<matjson::Value>("cached-index-names", {});
    jsonObj.set(indexId, indexName);
    Mod::get()->setSavedValue("cached-index-names", jsonObj);
}

Result<> IndexManager::downloadSong(int gdSongID, const std::string_view uniqueID) {
    Nongs* nongs = nullptr;

    if (!NongManager::get().hasSongID(gdSongID)) {
        SongInfoObject* obj = MusicDownloadManager::sharedState()->getSongInfoObject(gdSongID);

        GEODE_UNWRAP_INTO(
            nongs, NongManager::get().initSongID(obj, gdSongID, false).mapErr([gdSongID](const std::string& err) {
                return fmt::format("Failed to initialize song ID {}: {}", gdSongID, err);
            }));
    } else {
        nongs = NongManager::get().getNongs(gdSongID).value();
    }

    bool found = false;

    // Try starting download from local reference first
    for (const std::unique_ptr<YTSong>& song : nongs->youtube()) {
        if (song->metadata()->uniqueID != uniqueID) {
            continue;
        }

        return Err("YouTube song downloads will be enabled in a future release!");

        /*Result<DownloadSongTask> t = song->startDownload();*/
        /*if (t.isErr()) {*/
        /*    return Err("Failed to start download: {}", t.error());*/
        /*}*/
        /**/
        /*task = t.unwrap();*/
        /*found = true;*/
        /*local = song.get();*/
        /*break;*/
    }

    for (const std::unique_ptr<HostedSong>& song : nongs->hosted()) {
        if (song->metadata()->uniqueID != uniqueID) {
            continue;
        }

        m_urlToIDs[song->url()] = {song->metadata()->gdID, std::string(uniqueID)};
        async::spawn(song->startDownload(),
                     [this, song = song.get(), nongs, uniqueID = std::string(uniqueID)](Result<ByteVector> data) {
                         m_urlToIDs.erase(song->url());
                         if (data.isErr()) {
                             event::SongDownloadFailed(nongs->songID())
                                 .send(event::SongDownloadFailedData{nongs->songID(), uniqueID, data.unwrapErr()});
                             return;
                         }
                         this->onDownloadFinish(song, nongs, std::move(data.unwrap()));
                     });

        found = true;
        break;
    }

    // If not uniqueID not found in local songs, search in indexes
    if (!found) {
        if (!m_nongsForId.contains(gdSongID)) {
            return Err("Can't download nong for id {}. No local or index songs found.", gdSongID);
        }

        for (IndexSongMetadata* s : m_nongsForId[gdSongID]) {
            if (s->uniqueID != uniqueID) {
                continue;
            }

            if (s->ytId.has_value()) {
                return Err("YouTube song downloads will be enabled in a future release!");
            }

            if (s->url.has_value()) {
                m_urlToIDs[s->url.value()] = {nongs->songID(), std::string(uniqueID)};
                async::spawn(
                    download::startHostedDownload(s->url.value()),
                    [this, s, nongs, uniqueID = std::string(uniqueID)](Result<ByteVector> data) {
                        m_urlToIDs.erase(s->url.value());
                        if (data.isErr()) {
                            event::SongDownloadFailed(nongs->songID())
                                .send(event::SongDownloadFailedData{nongs->songID(), uniqueID, data.unwrapErr()});
                            return;
                        }
                        this->onDownloadFinish(s, nongs, std::move(data.unwrap()));
                    });

                found = true;
                break;
            }
        }
    }

    if (!found) {
        return Err("Couldn't download song. Reference not found");
    }

    return Ok();
}

void IndexManager::onDownloadFinish(std::variant<IndexSongMetadata*, Song*>&& source, Nongs* destination,
                                    ByteVector&& data) {
    std::string uniqueId;
    if (std::holds_alternative<index::IndexSongMetadata*>(source)) {
        uniqueId = std::get<index::IndexSongMetadata*>(source)->uniqueID;
    } else {
        uniqueId = std::get<Song*>(source)->metadata()->uniqueID;
    }

    if (data.empty()) {
        const std::string err = "Failed to store downloaded file. ByteVector empty.";
        log::error("{}", err);
        event::SongDownloadFailed(destination->songID())
            .send(event::SongDownloadFailedData{destination->songID(), uniqueId, err});
        return;
    }

    std::filesystem::path path;

    if (std::holds_alternative<IndexSongMetadata*>(source)) {
        auto s = std::get<IndexSongMetadata*>(source);
        path = NongManager::get().baseNongsPath() / fmt::format("{}-{}.mp3", s->parentID->m_id, s->uniqueID);
    } else {
        std::string name;

        if (auto song = std::get<Song*>(source); song->indexID().has_value()) {
            name = fmt::format("{}-{}.mp3", song->indexID().value(), song->metadata()->uniqueID);
        } else {
            name = fmt::format("{}.mp3", song->metadata()->uniqueID);
        }

        path = NongManager::get().baseNongsPath() / name;
    }

    std::ofstream out(path, std::ios_base::out | std::ios_base::binary);

    if (!out.is_open()) {
        const std::string err = "Failed to store downloaded file. Couldn't open file for write";
        log::error("{}", err);
        event::SongDownloadFailed(destination->songID())
            .send(event::SongDownloadFailedData{destination->songID(), uniqueId, err});
        return;
    }

    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();

    Song* insertedSong = nullptr;

    if (std::holds_alternative<Song*>(source)) {
        auto localSong = std::get<Song*>(source);
        localSong->setPath(path);
        event::SongDownloadFinished().send(event::SongDownloadFinishedData(std::nullopt, std::get<Song*>(source)));
        return;
    }

    auto metadata = std::get<IndexSongMetadata*>(source);

    const std::function<void(std::string)> orElse = [destination, uniqueId, path](std::string err) {
        const std::string print = fmt::format("Couldn't store index song. {}", err);
        log::error("{}", print);
        event::SongDownloadFailed(destination->songID())
            .send(event::SongDownloadFailedData{destination->songID(), uniqueId, print});
        std::error_code ec;
        std::filesystem::remove(path, ec);
    };

    if (metadata->url.has_value()) {
        Result<HostedSong*> r =
            destination->add(HostedSong(SongMetadata(destination->songID(), metadata->uniqueID, metadata->name,
                                                     metadata->artist, std::nullopt, metadata->startOffset),
                                        metadata->url.value(), metadata->parentID->m_id, path));

        if (r.isErr()) {
            orElse(r.unwrapErr());
            return;
        }

        insertedSong = r.unwrap();
    } else if (metadata->ytId.has_value()) {
        Result<YTSong*> r =
            destination->add(YTSong(SongMetadata(destination->songID(), metadata->uniqueID, metadata->name,
                                                 metadata->artist, std::nullopt, metadata->startOffset),
                                    metadata->ytId.value(), metadata->parentID->m_id, path));
        if (r.isErr()) {
            orElse(r.unwrapErr());
            return;
        }

        insertedSong = r.unwrap();
    } else {
        const std::string err = "No url or YouTube ID on song";
        orElse(err);
        return;
    }

    (void)destination->commit();

    event::SongDownloadFinished().send(event::SongDownloadFinishedData{std::optional(metadata), insertedSong});
}

void IndexManager::registerIndexNongs(Nongs* destination) {
    if (m_nongsForId.contains(!destination->songID())) {
        return;
    }

    destination->indexSongs().clear();
    for (IndexSongMetadata* s : m_nongsForId[destination->songID()]) {
        destination->indexSongs().push_back(s);
    }
}

};  // namespace jukebox
