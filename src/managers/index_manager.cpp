#include "managers/index_manager.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>
#include <Geode/Result.hpp>
#include <matjson.hpp>
#include "Geode/binding/MusicDownloadManager.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Event.hpp"
#include "Geode/loader/Log.hpp"
#include "Geode/loader/Mod.hpp"
#include "Geode/utils/general.hpp"
#include "Geode/utils/web.hpp"

#include "download/hosted.hpp"
#include "download/youtube.hpp"
#include "events/song_download_failed.hpp"
#include "events/song_download_finished.hpp"
#include "events/song_download_progress.hpp"
#include "events/song_error.hpp"
#include "events/start_download.hpp"
#include "index.hpp"
#include "index_serialize.hpp"
#include "managers/nong_manager.hpp"
#include "nong.hpp"
#include "ui/indexes_setting.hpp"

namespace jukebox {

using namespace jukebox::index;

bool IndexManager::init() {
    if (m_initialized) {
        return true;
    }

    std::filesystem::path path = this->baseIndexesPath();
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        return true;
    }

    if (Result<> res = this->fetchIndexes(); res.isErr()) {
        event::SongError(
            false, fmt::format("Failed to fetch indexes: {}", res.unwrapErr()))
            .post();
        return false;
    }

    m_initialized = true;
    return true;
}

Result<std::vector<IndexSource>> IndexManager::getIndexes() {
    auto setting = Mod::get()->getSettingValue<Indexes>("indexes");
    return Ok(setting.indexes);
}

std::filesystem::path IndexManager::baseIndexesPath() {
    static std::filesystem::path path =
        Mod::get()->getSaveDir() / "indexes-cache";
    return path;
}

Result<> IndexManager::loadIndex(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        return Err("Index file does not exist");
    }

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

    GEODE_UNWRAP_INTO(matjson::Value jsonObj, matjson::parse(contents));
    GEODE_UNWRAP_INTO(IndexMetadata indexMeta,
                      matjson::Serialize<IndexMetadata>::fromJson(jsonObj));

    std::unique_ptr<IndexMetadata> index =
        std::make_unique<IndexMetadata>(std::move(indexMeta));

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
        Result<IndexSongMetadata> r =
            matjson::Serialize<IndexSongMetadata>::fromJson(hostedNong);
        if (r.isErr()) {
            event::SongError(
                false,
                fmt::format("Failed to parse index song: {}", r.unwrapErr()))
                .post();
            continue;
        }

        std::unique_ptr<IndexSongMetadata> song =
            std::make_unique<IndexSongMetadata>(r.unwrap());

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

            if (geode::Result<> r = nongs->registerIndexSong(song.get());
                r.isErr()) {
                event::SongError(
                    false, fmt::format("Failed to register index song: {}",
                                       r.unwrapErr()))
                    .post();
            }
        }

        index->m_songs.m_hosted.push_back(std::move(song));
    }

    IndexMetadata* ref = index.get();
    m_loadedIndexes.emplace(ref->m_id, std::move(index));

    return Ok();
}

Result<> IndexManager::fetchIndexes() {
    m_indexListeners.clear();
    m_downloadSongListeners.clear();

    GEODE_UNWRAP_INTO(const std::vector<IndexSource> indexes,
                      this->getIndexes());

    for (const IndexSource& index : indexes) {
        if (!index.m_enabled || index.m_url.size() < 3) {
            continue;
        }

        // Hash url to use as a filename per index
        std::hash<std::string> hasher;
        std::size_t hashValue = hasher(index.m_url);
        std::stringstream hashStream;
        hashStream << std::hex << hashValue;

        std::filesystem::path filepath =
            this->baseIndexesPath() / fmt::format("{}.json", hashStream.str());

        FetchIndexTask task =
            web::WebRequest()
                .timeout(std::chrono::seconds(30))
                .get(index.m_url)
                .map(
                    [this, filepath, index](
                        web::WebResponse* response) -> FetchIndexTask::Value {
                        if (response->ok() && response->string().isOk()) {
                            GEODE_UNWRAP_INTO(
                                matjson::Value jsonObj,
                                matjson::parse(response->string().unwrap()));

                            jsonObj.set("url", index.m_url);

                            GEODE_UNWRAP(
                                matjson::Serialize<IndexMetadata>::fromJson(
                                    jsonObj));

                            std::ofstream output(filepath);
                            if (!output.is_open()) {
                                return Err(fmt::format("Couldn't open file: {}",
                                                       filepath));
                            }
                            output << jsonObj.dump(matjson::NO_INDENTATION);
                            output.close();

                            return Ok();
                        }
                        return Err("Web request failed");
                    },
                    [](web::WebProgress* progress) -> FetchIndexTask::Progress {
                        return progress->downloadProgress().value_or(0) / 100.f;
                    });

        auto listener = EventListener<FetchIndexTask>();
        listener.bind([this, index, filepath](FetchIndexTask::Event* event) {
            if (float* progress = event->getProgress()) {
                return;
            }

            m_indexListeners.erase(index.m_url);

            if (FetchIndexTask::Value* result = event->getValue()) {
                if (result->isErr()) {
                    event::SongError(false,
                                     fmt::format("Failed to fetch index: {}",
                                                 result->unwrapErr()))
                        .post();
                } else {
                    log::info("Index fetched and cached: {}", index.m_url);
                }
            } else if (event->isCancelled()) {
            }

            if (Result<> res = this->loadIndex(filepath); res.isErr()) {
                event::SongError(false, fmt::format("Failed to load index: {}",
                                                    res.unwrapErr()))
                    .post();
            }
        });
        listener.setFilter(task);
        m_indexListeners.emplace(index.m_url, std::move(listener));
    }

    return Ok();
}

std::optional<float> IndexManager::getSongDownloadProgress(
    const std::string& uniqueID) {
    if (m_downloadSongListeners.contains(uniqueID)) {
        return m_downloadProgress.at(uniqueID);
    }
    return std::nullopt;
}

std::optional<std::string> IndexManager::getIndexName(
    const std::string& indexID) {
    auto jsonObj =
        Mod::get()->getSavedValue<matjson::Value>("cached-index-names");
    if (!jsonObj.contains(indexID)) {
        return std::nullopt;
    }
    return jsonObj[indexID].asString().mapOr<std::optional<std::string>>(
        std::nullopt, [](auto i) { return std::optional(i); });
}

void IndexManager::cacheIndexName(const std::string& indexId,
                                  const std::string& indexName) {
    auto jsonObj =
        Mod::get()->getSavedValue<matjson::Value>("cached-index-names", {});
    jsonObj.set(indexId, indexName);
    Mod::get()->setSavedValue("cached-index-names", jsonObj);
}

Result<> IndexManager::downloadSong(int gdSongID, const std::string& uniqueID) {
    Nongs* nongs = nullptr;

    if (!NongManager::get().hasSongID(gdSongID)) {
        SongInfoObject* obj =
            MusicDownloadManager::sharedState()->getSongInfoObject(gdSongID);

        GEODE_UNWRAP_INTO(nongs,
                          NongManager::get()
                              .initSongID(obj, gdSongID, false)
                              .mapErr([gdSongID](const std::string& err) {
                                  return fmt::format(
                                      "Failed to initialize song ID {}: {}",
                                      gdSongID, err);
                              }));
    } else {
        nongs = NongManager::get().getNongs(gdSongID).value();
    }

    DownloadSongTask task;
    std::optional<IndexSongMetadata*> indexMeta = std::nullopt;
    Song* local = nullptr;
    bool found = false;

    // Try starting download from local reference first
    for (const std::unique_ptr<YTSong>& song : nongs->youtube()) {
        if (song->metadata()->uniqueID != uniqueID) {
            continue;
        }

        return Err(
            "YouTube song downloads will be enabled in a future release!");
        break;

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

    if (!found) {
        for (const std::unique_ptr<HostedSong>& song : nongs->hosted()) {
            if (song->metadata()->uniqueID != uniqueID) {
                continue;
            }

            GEODE_UNWRAP_INTO(
                task, song->startDownload().mapErr([](std::string err) {
                    return fmt::format("Failed to start download: {}", err);
                }));

            found = true;
            local = song.get();
            break;
        }
    }

    // Look in indexes otherwise
    if (!m_nongsForId.contains(gdSongID)) {
        return Err("Can't download nong for id {}. No index songs found.",
                   gdSongID);
    }

    if (!found) {
        std::vector<IndexSongMetadata*> songs = m_nongsForId[gdSongID];
        for (IndexSongMetadata* s : songs) {
            if (s->uniqueID != uniqueID) {
                continue;
            }

            if (s->url.has_value()) {
                task = jukebox::download::startHostedDownload(s->url.value());
                indexMeta = s;
                found = true;
                break;
            } else if (s->ytId.has_value()) {
                return Err(
                    "YouTube song downloads will be enabled in a future "
                    "release!");
                break;

                task = jukebox::download::startYoutubeDownload(s->ytId.value());
                indexMeta = s;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        return Err("Couldn't download song. Reference not found");
    }

    task.listen(
        [this, indexMeta, local, nongs, gdSongID,
         uniqueID](Result<ByteVector>* vector) {
            if (vector->isErr()) {
                event::SongDownloadFailed(gdSongID, uniqueID,
                                          vector->unwrapErr())
                    .post();
                return;
            }

            std::variant<index::IndexSongMetadata*, Song*> source;

            if (indexMeta.has_value()) {
                source = indexMeta.value();
            } else {
                source = local;
            }

            this->onDownloadFinish(std::move(source), nongs,
                                   std::move(vector->unwrap()));
        },
        [this, uniqueID, gdSongID](float* progress) {
            this->onDownloadProgress(gdSongID, uniqueID, *progress);
        });
    return Ok();
}

void IndexManager::onDownloadProgress(int gdSongID, const std::string& uniqueId,
                                      float progress) {
    event::SongDownloadProgress(gdSongID, uniqueId, progress).post();
}

void IndexManager::onDownloadFinish(
    std::variant<index::IndexSongMetadata*, Song*>&& source, Nongs* destination,
    ByteVector&& data) {
    std::string uniqueId;
    if (std::holds_alternative<index::IndexSongMetadata*>(source)) {
        uniqueId = std::get<index::IndexSongMetadata*>(source)->uniqueID;
    } else {
        uniqueId = std::get<Song*>(source)->metadata()->uniqueID;
    }

    if (data.size() == 0) {
        const std::string err =
            "Failed to store downloaded file. ByteVector empty.";
        log::error("{}", err);
        event::SongDownloadFailed(destination->songID(), uniqueId, err).post();
        return;
    }

    std::filesystem::path path;

    if (std::holds_alternative<index::IndexSongMetadata*>(source)) {
        index::IndexSongMetadata* s =
            std::get<index::IndexSongMetadata*>(source);
        path = NongManager::get().baseNongsPath() /
               fmt::format("{}-{}.mp3", s->parentID->m_id, s->uniqueID);
    } else {
        std::string name;
        Song* song = std::get<Song*>(source);

        if (song->indexID().has_value()) {
            name = fmt::format("{}-{}.mp3", song->indexID().value(),
                               song->metadata()->uniqueID);
        } else {
            name = fmt::format("{}.mp3", song->metadata()->uniqueID);
        }

        path = NongManager::get().baseNongsPath() / name;
    }

    std::ofstream out(path, std::ios_base::out | std::ios_base::binary);

    if (!out.is_open()) {
        const std::string err =
            "Failed to store downloaded file. Couldn't open file for write";
        log::error("{}", err);
        event::SongDownloadFailed(destination->songID(), uniqueId, err).post();
        return;
    }

    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();

    Song* insertedSong = nullptr;

    if (auto s = std::holds_alternative<Song*>(source)) {
        Song* localSong = std::get<Song*>(source);
        localSong->setPath(path);
        event::SongDownloadFinished(std::nullopt, std::get<Song*>(source))
            .post();
        return;
    }

    index::IndexSongMetadata* metadata =
        std::get<index::IndexSongMetadata*>(source);

    const std::function<void(std::string)> orElse = [destination, uniqueId,
                                                     path](std::string err) {
        const std::string print =
            fmt::format("Couldn't store index song. {}", err);
        log::error("{}", print);
        event::SongDownloadFailed(destination->songID(), uniqueId, print)
            .post();
        std::error_code ec;
        std::filesystem::remove(path, ec);
    };

    if (metadata->url.has_value()) {
        Result<HostedSong*> r = destination->add(
            HostedSong(SongMetadata(destination->songID(), metadata->uniqueID,
                                    metadata->name, metadata->artist,
                                    std::nullopt, metadata->startOffset),
                       metadata->url.value(), metadata->parentID->m_id, path));

        if (r.isErr()) {
            orElse(r.unwrapErr());
            return;
        }

        insertedSong = r.unwrap();
    } else if (metadata->ytId.has_value()) {
        Result<YTSong*> r = destination->add(
            YTSong(SongMetadata(destination->songID(), metadata->uniqueID,
                                metadata->name, metadata->artist, std::nullopt,
                                metadata->startOffset),
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

    event::SongDownloadFinished(metadata, insertedSong).post();
}

ListenerResult IndexManager::onDownloadStart(event::StartDownload* e) {
    Result<> res = this->downloadSong(e->gdId(), e->song()->uniqueID);
    if (res.isErr()) {
        event::SongDownloadFailed(e->gdId(), e->song()->uniqueID,
                                  res.unwrapErr())
            .post();
    }
    return ListenerResult::Propagate;
}

void IndexManager::registerIndexNongs(Nongs* destination) {
    if (m_nongsForId.contains(!destination->songID())) {
        return;
    }

    destination->indexSongs().clear();
    for (index::IndexSongMetadata* s : m_nongsForId[destination->songID()]) {
        destination->indexSongs().push_back(s);
    }
}

};  // namespace jukebox
