#include <jukebox/nong/nong.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>
#include <Geode/Result.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/utils/general.hpp>
#include <matjson.hpp>

#include <jukebox/download/hosted.hpp>
#include <jukebox/download/youtube.hpp>
#include <jukebox/events/nong_deleted.hpp>
#include <jukebox/events/song_state_changed.hpp>
#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/nong/nong_serialize.hpp>
#include <jukebox/utils/random_string.hpp>

using namespace geode::prelude;
using namespace jukebox::index;

namespace jukebox {

class LocalSong::Impl {
private:
    friend class LocalSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::filesystem::path m_path;

public:
    Impl(SongMetadata&& metadata, const std::filesystem::path& path)
        : m_metadata(std::make_unique<SongMetadata>(metadata)), m_path(path) {}

    Impl(const Impl& other)
        : m_path(other.m_path),
          m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)) {}
    Impl& operator=(const Impl&) = delete;

    ~Impl() = default;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    SongMetadata* metadata() const { return m_metadata.get(); }
    std::filesystem::path path() const { return m_path; }
    void setPath(std::filesystem::path&& p) { m_path = p; }
};

LocalSong::LocalSong(SongMetadata&& metadata, const std::filesystem::path& path)
    : m_impl(std::make_unique<Impl>(std::move(metadata), path)) {}

LocalSong::LocalSong(const LocalSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl)) {}

LocalSong& LocalSong::operator=(const LocalSong& other) {
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_metadata =
        std::make_unique<SongMetadata>(*other.m_impl->m_metadata);

    return *this;
}

LocalSong::LocalSong(LocalSong&&) = default;
LocalSong& LocalSong::operator=(LocalSong&&) = default;
LocalSong::~LocalSong() = default;

SongMetadata* LocalSong::metadata() const { return m_impl->metadata(); }
std::optional<std::filesystem::path> LocalSong::path() const {
    return m_impl->path();
}
void LocalSong::setPath(std::filesystem::path p) {
    m_impl->setPath(std::move(p));
}

LocalSong LocalSong::createUnknown(int songID) {
    return LocalSong{
        SongMetadata{songID, jukebox::random_string(16), "Unknown", ""},
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(songID))};
}

LocalSong LocalSong::fromSongObject(SongInfoObject* obj) {
    return LocalSong{
        SongMetadata{obj->m_songID, jukebox::random_string(16), obj->m_songName,
                     obj->m_artistName},
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(obj->m_songID))};
}

class YTSong::Impl {
private:
    friend class YTSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_youtubeID;
    std::optional<std::string> m_indexID;
    std::optional<std::filesystem::path> m_path;

public:
    Impl(SongMetadata&& metadata, std::string youtubeID,
         std::optional<std::string> indexID,
         std::optional<std::filesystem::path> path = std::nullopt)
        : m_metadata(std::make_unique<SongMetadata>(metadata)),
          m_youtubeID(youtubeID),
          m_indexID(indexID),
          m_path(path) {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
          m_path(other.m_path),
          m_indexID(other.m_indexID),
          m_youtubeID(other.m_youtubeID) {}
    Impl& operator=(const Impl&) = delete;

    ~Impl() = default;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    SongMetadata* metadata() const { return m_metadata.get(); }
    std::optional<std::filesystem::path> path() const { return m_path; }
    std::string youtubeID() const { return m_youtubeID; }
    std::optional<std::string> indexID() const { return m_indexID; }
    Result<Task<Result<ByteVector>, float>> startDownload() {
        std::error_code ec;
        if (m_path.has_value() && std::filesystem::exists(m_path.value(), ec)) {
            return Err("Song already is downloaded");
        }

        return Ok(download::startYoutubeDownload(m_youtubeID));
    }
    void setPath(std::filesystem::path&& p) { m_path = p; }
};

YTSong::YTSong(SongMetadata&& metadata, std::string youtubeID,
               std::optional<std::string> indexID,
               std::optional<std::filesystem::path> path)
    : m_impl(std::make_unique<Impl>(std::move(metadata), youtubeID, indexID,
                                    path)) {}

YTSong::YTSong(const YTSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl)) {}

YTSong& YTSong::operator=(const YTSong& other) {
    m_impl->m_youtubeID = other.m_impl->m_youtubeID;
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_indexID = other.m_impl->m_indexID;
    m_impl->m_metadata =
        std::make_unique<SongMetadata>(*other.m_impl->m_metadata);

    return *this;
}

YTSong::YTSong(YTSong&& other) = default;
YTSong& YTSong::operator=(YTSong&& other) = default;
YTSong::~YTSong() = default;

SongMetadata* YTSong::metadata() const { return m_impl->metadata(); }

std::string YTSong::youtubeID() const { return m_impl->youtubeID(); }

std::optional<std::string> YTSong::indexID() const { return m_impl->indexID(); }
void YTSong::setIndexID(const std::string& id) { m_impl->m_indexID = id; }

std::optional<std::filesystem::path> YTSong::path() const {
    return m_impl->path();
}
void YTSong::setPath(std::filesystem::path p) { m_impl->setPath(std::move(p)); }

Result<Task<Result<ByteVector>, float>> YTSong::startDownload() {
    return m_impl->startDownload();
}

class HostedSong::Impl {
private:
    friend class HostedSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_url;
    std::optional<std::string> m_indexID;
    std::optional<std::filesystem::path> m_path;

public:
    Impl(SongMetadata&& metadata, std::string url,
         std::optional<std::string> indexID,
         std::optional<std::filesystem::path> path = std::nullopt)
        : m_metadata(std::make_unique<SongMetadata>(metadata)),
          m_url(url),
          m_indexID(indexID),
          m_path(path) {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
          m_path(other.m_path),
          m_indexID(other.m_indexID),
          m_url(other.m_url) {}
    Impl& operator=(const Impl& other) = delete;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    ~Impl() = default;

    SongMetadata* metadata() const { return m_metadata.get(); }
    std::string url() const { return m_url; }
    std::optional<std::string> indexID() const { return m_indexID; }
    std::optional<std::filesystem::path> path() const { return m_path; }
    Result<Task<Result<ByteVector>, float>> startDownload() {
        std::error_code ec;
        if (m_path.has_value() && std::filesystem::exists(m_path.value(), ec)) {
            return Err("Song already is downloaded");
        }

        return Ok(download::startHostedDownload(m_url));
    }
    void setPath(std::filesystem::path&& p) { m_path = p; }
};

HostedSong::HostedSong(SongMetadata&& metadata, std::string url,
                       std::optional<std::string> indexID,
                       std::optional<std::filesystem::path> path)
    : m_impl(std::make_unique<Impl>(std::move(metadata), url, indexID, path)) {}

HostedSong::HostedSong(const HostedSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl)) {}

HostedSong& HostedSong::operator=(const HostedSong& other) {
    m_impl->m_metadata =
        std::make_unique<SongMetadata>(*other.m_impl->m_metadata);
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_indexID = other.m_impl->m_indexID;
    m_impl->m_url = other.m_impl->m_url;

    return *this;
}
SongMetadata* HostedSong::metadata() const { return m_impl->metadata(); }
std::string HostedSong::url() const { return m_impl->url(); }
std::optional<std::string> HostedSong::indexID() const {
    return m_impl->indexID();
}
void HostedSong::setIndexID(const std::string& id) { m_impl->m_indexID = id; }
std::optional<std::filesystem::path> HostedSong::path() const {
    return m_impl->path();
}
void HostedSong::setPath(std::filesystem::path p) {
    m_impl->setPath(std::move(p));
}

Result<Task<Result<ByteVector>, float>> HostedSong::startDownload() {
    return m_impl->startDownload();
}

HostedSong::HostedSong(HostedSong&& other) = default;
HostedSong& HostedSong::operator=(HostedSong&& other) = default;
HostedSong::~HostedSong() = default;

class Nongs::Impl {
private:
    friend class Nongs;

    bool m_sfx;
    int m_songID;
    Song* m_active;
    std::unique_ptr<LocalSong> m_default;
    std::vector<std::unique_ptr<LocalSong>> m_locals;
    std::vector<std::unique_ptr<YTSong>> m_youtube;
    std::vector<std::unique_ptr<HostedSong>> m_hosted;

    std::vector<IndexSongMetadata*> m_indexSongs;

    void deletePath(std::optional<std::filesystem::path> path) {
        std::error_code ec;
        if (path.has_value() && std::filesystem::exists(path.value())) {
            std::filesystem::remove(path.value(), ec);
            if (ec) {
                log::error("Couldn't delete nong. Category: {}, message: {}",
                           ec.category().name(),
                           ec.category().message(ec.value()));
                ec = {};
            }
        }
    }

public:
    Impl(int songID, std::unique_ptr<LocalSong> defaultSong, bool sfx)
        : m_songID(songID),
          m_active(defaultSong.get()),
          m_default(std::move(defaultSong)),
          m_sfx(sfx) {}

    Impl(int songID, bool sfx)
        : Impl(songID,
               std::make_unique<LocalSong>(LocalSong::createUnknown(songID)),
               sfx) {}

    geode::Result<> commit(Nongs* self) {
        const std::filesystem::path path =
            NongManager::get().baseManifestPath() /
            fmt::format("{}.json", m_songID);

        // Don't save manifest for songs with no nongs
        if (m_locals.empty() && m_youtube.empty() && m_hosted.empty()) {
            std::error_code ec;
            if (std::filesystem::exists(path, ec)) {
                std::filesystem::remove(path, ec);
            }

            return Ok();
        }

        matjson::Value json = matjson::Serialize<Nongs>::toJson(*self);

        std::ofstream output(path);
        if (!output.is_open()) {
            return Err(fmt::format("Couldn't open file: {}", path));
        }

        output << json.dump(matjson::NO_INDENTATION);
        output.close();

        return Ok();
    }

    geode::Result<> canSetActive(const std::string& uniqueID,
                                 std::filesystem::path path) {
        const bool IS_DEFAULT = uniqueID == m_default->metadata()->uniqueID;

        if (IS_DEFAULT) {
            return Ok();
        }

        std::error_code ec;

        if (!IS_DEFAULT && !std::filesystem::exists(path, ec)) {
            return Err("Song doesn't exist on disk");
        }

        return Ok();
    }

    geode::Result<> setActive(const std::string& uniqueID, Nongs* self) {
        if (auto song = this->getLocalFromID(uniqueID)) {
            geode::Result<> res =
                this->canSetActive(uniqueID, song.value()->path().value());
            if (res.isErr()) {
                return res;
            }

            m_active = song.value();

            if (NongManager::get().initialized()) {
                event::SongStateChanged(self).post();
            }

            return Ok();
        }

        if (auto song = this->getYTFromID(uniqueID)) {
            if (!song.value()->path()) {
                return Err("Song is not downloaded");
            }
            geode::Result<> res =
                this->canSetActive(uniqueID, song.value()->path().value());
            if (res.isErr()) {
                return res;
            }

            m_active = song.value();
            if (NongManager::get().initialized()) {
                event::SongStateChanged(self).post();
            }
            return Ok();
        }

        if (auto song = this->getHostedFromID(uniqueID)) {
            if (!song.value()->path()) {
                return Err("Song is not downloaded");
            }
            geode::Result<> res =
                this->canSetActive(uniqueID, song.value()->path().value());
            if (res.isErr()) {
                return res;
            }

            m_active = song.value();
            if (NongManager::get().initialized()) {
                event::SongStateChanged(self).post();
            }
            return Ok();
        }

        return Err("No song found with given path for song ID");
    }

    geode::Result<> merge(Nongs&& other) {
        if (other.songID() != m_songID) {
            return Err("Merging with NONGs of a different song ID");
        }

        for (const std::unique_ptr<LocalSong>& i : other.locals()) {
            if (i->path() == other.defaultSong()->path()) {
                continue;
            }
            m_locals.emplace_back(std::make_unique<LocalSong>(*i));
        }

        for (const auto& i : other.youtube()) {
            m_youtube.emplace_back(std::make_unique<YTSong>(*i));
        }

        for (const auto& i : other.hosted()) {
            m_hosted.emplace_back(std::make_unique<HostedSong>(*i));
        }

        return Ok();
    }

    geode::Result<> deleteAllSongs() {
        for (std::unique_ptr<LocalSong>& local : m_locals) {
            this->deletePath(local->path());
        }
        m_locals.clear();

        for (std::unique_ptr<YTSong>& youtube : m_youtube) {
            this->deletePath(youtube->path());
        }
        m_youtube.clear();

        for (std::unique_ptr<HostedSong>& hosted : m_hosted) {
            this->deletePath(hosted->path());
        }
        m_hosted.clear();

        m_active = m_default.get();

        return Ok();
    }

    geode::Result<> deleteSong(const std::string& uniqueID, bool audio,
                               Nongs* self) {
        if (m_default->metadata()->uniqueID == uniqueID) {
            return Err("Cannot delete default song");
        }

        if (m_active->metadata()->uniqueID == uniqueID) {
            (void)this->setActive(m_default.get()->metadata()->uniqueID, self);
        }

        for (auto i = m_locals.begin(); i != m_locals.end(); ++i) {
            if ((*i)->metadata()->uniqueID == uniqueID) {
                if (audio) {
                    this->deletePath((*i)->path());
                }
                m_locals.erase(i);
                if (NongManager::get().initialized()) {
                    event::NongDeleted(uniqueID, m_songID).post();
                }
                return Ok();
            }
        }

        for (auto i = m_youtube.begin(); i != m_youtube.end(); ++i) {
            if ((*i)->metadata()->uniqueID == uniqueID) {
                if (audio) {
                    this->deletePath((*i)->path());
                }
                m_youtube.erase(i);
                if (NongManager::get().initialized()) {
                    event::NongDeleted(uniqueID, m_songID).post();
                }
                return Ok();
            }
        }

        for (auto i = m_hosted.begin(); i != m_hosted.end(); ++i) {
            if ((*i)->metadata()->uniqueID == uniqueID) {
                if (audio) {
                    this->deletePath((*i)->path());
                }
                m_hosted.erase(i);
                if (NongManager::get().initialized()) {
                    event::NongDeleted(uniqueID, m_songID).post();
                }
                return Ok();
            }
        }

        return Err("No song found with given path for song ID");
    }

    geode::Result<> deleteSongAudio(const std::string& uniqueID) {
        if (m_default->metadata()->uniqueID == uniqueID) {
            return Err("Cannot delete audio of the default song");
        }

        if (m_active->metadata()->uniqueID == uniqueID) {
            m_active = m_default.get();
        }

        for (auto i = m_locals.begin(); i != m_locals.end(); ++i) {
            if ((*i)->metadata()->uniqueID == uniqueID) {
                return Err("Cannot delete audio of local songs");
                return Ok();
            }
        }

        for (auto i = m_youtube.begin(); i != m_youtube.end(); ++i) {
            if ((*i)->metadata()->uniqueID == uniqueID) {
                this->deletePath((*i)->path());
                return Ok();
            }
        }

        for (auto i = m_hosted.begin(); i != m_hosted.end(); ++i) {
            if ((*i)->metadata()->uniqueID == uniqueID) {
                this->deletePath((*i)->path());
                return Ok();
            }
        }

        return Err("No song found with given path for song ID");
    }

    std::optional<LocalSong*> getLocalFromID(const std::string_view uniqueID) {
        if (m_default->metadata()->uniqueID == uniqueID) {
            return m_default.get();
        }

        for (const std::unique_ptr<LocalSong>& i : m_locals) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        return std::nullopt;
    }

    std::optional<YTSong*> getYTFromID(const std::string_view uniqueID) {
        for (const std::unique_ptr<YTSong>& i : m_youtube) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        return std::nullopt;
    }

    std::optional<HostedSong*> getHostedFromID(
        const std::string_view uniqueID) {
        for (const std::unique_ptr<HostedSong>& i : m_hosted) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        return std::nullopt;
    }

    std::optional<Song*> findSong(const std::string& uniqueID) const {
        if (m_default->metadata()->uniqueID == uniqueID) {
            return m_default.get();
        }

        for (const auto& i : m_locals) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        for (const auto& i : m_youtube) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        for (const auto& i : m_hosted) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        return std::nullopt;
    }

    geode::Result<> replaceSong(const std::string& id, LocalSong&& song,
                                Nongs* self) {
        bool isActive = m_active->metadata()->uniqueID == id;
        std::optional<Song*> opt = this->findSong(id);
        if (!opt) {
            return Err(fmt::format("NONG ID {} not found", id));
        }
        Song* prevNong = opt.value();
        std::optional<std::filesystem::path> prevPath = prevNong->path();

        bool deleteAudio = prevPath != song.path();
        auto _ = this->deleteSong(id, deleteAudio, self);

        GEODE_UNWRAP(this->add(std::move(song)));

        if (isActive) {
            (void)this->setActive(id, self);
        }
        return Ok();
    }

    geode::Result<> replaceSong(const std::string& id, YTSong&& song,
                                Nongs* self) {
        bool isActive = m_active->metadata()->uniqueID == id;
        std::optional<Song*> opt = this->findSong(id);
        if (!opt) {
            return Err(fmt::format("NONG ID {} not found", id));
        }
        Song* prevNong = opt.value();
        std::optional<std::filesystem::path> prevPath = prevNong->path();

        bool deleteAudio = prevPath != song.path();
        auto _ = this->deleteSong(id, deleteAudio, self);

        YTSong* added = nullptr;

        GEODE_UNWRAP_INTO(added, this->add(std::move(song)));

        if (isActive && added->path().has_value()) {
            auto _ = this->setActive(id, self);
        }
        return Ok();
    }

    geode::Result<> replaceSong(const std::string& id, HostedSong&& song,
                                Nongs* self) {
        bool isActive = m_active->metadata()->uniqueID == id;
        std::optional<Song*> opt = this->findSong(id);
        if (!opt) {
            return Err(fmt::format("NONG ID {} not found", id));
        }
        Song* prevNong = opt.value();
        std::optional<std::filesystem::path> prevPath = prevNong->path();

        bool deleteAudio = prevPath != song.path();
        auto _ = this->deleteSong(id, deleteAudio, self);

        HostedSong* added = nullptr;

        GEODE_UNWRAP_INTO(added, this->add(std::move(song)));

        if (isActive && added->path().has_value()) {
            auto _ = this->setActive(id, self);
        }
        return Ok();
    }

    Result<LocalSong*> add(LocalSong&& song) {
        for (const std::unique_ptr<LocalSong>& i : m_locals) {
            if (i->metadata() == song.metadata() && i->path() == song.path()) {
                std::string err =
                    fmt::format("Attempted to add a duplicate song for id {}",
                                song.metadata()->gdID);
                return Err(err);
            }
        }

        std::unique_ptr<LocalSong> ptr =
            std::make_unique<LocalSong>(std::move(song));
        LocalSong* ret = ptr.get();
        m_locals.push_back(std::move(ptr));

        return Ok(ret);
    }

    Result<YTSong*> add(YTSong&& song) {
        auto s = std::make_unique<YTSong>(std::move(song));
        auto ret = s.get();
        m_youtube.push_back(std::move(s));

        return Ok(ret);
    }

    Result<HostedSong*> add(HostedSong&& song) {
        for (const std::unique_ptr<HostedSong>& i : m_hosted) {
            if (i->url() == song.url() && i->metadata() == song.metadata()) {
                std::string err =
                    fmt::format("Attempted to add a duplicate song for id {}",
                                song.metadata()->gdID);
                return Err(err);
            }
        }

        auto s = std::make_unique<HostedSong>(std::move(song));
        HostedSong* ret = s.get();

        m_hosted.push_back(std::move(s));

        return Ok(ret);
    }

    bool isDefaultActive() const {
        return m_active->metadata()->uniqueID ==
               m_default->metadata()->uniqueID;
    }

    geode::Result<> registerIndexSong(index::IndexSongMetadata* song) {
        bool hasSongID = false;
        for (int id : song->songIDs) {
            if (id == m_songID) {
                hasSongID = true;
            }
        }

        if (!hasSongID) {
            return Err("Index song {} doesn't apply for ID {}", song->uniqueID,
                       m_songID);
        }

        for (index::IndexSongMetadata* i : m_indexSongs) {
            if (i == song) {
                return Err("Song {} already registered for ID {}",
                           song->uniqueID, m_songID);
            }
        }

        m_indexSongs.push_back(song);
        return Ok();
    }

    int songID() const { return m_songID; }
    LocalSong* defaultSong() const { return m_default.get(); }
    Song* active() const { return m_active; }
    std::vector<std::unique_ptr<LocalSong>>& locals() { return m_locals; }
    std::vector<std::unique_ptr<YTSong>>& youtube() { return m_youtube; }
    std::vector<std::unique_ptr<HostedSong>>& hosted() { return m_hosted; }
};

Nongs::Nongs(int songID, LocalSong&& defaultSong, bool sfx)
    : m_impl(std::make_unique<Impl>(
          songID, std::make_unique<LocalSong>(defaultSong), sfx)) {}
Nongs::Nongs(int songID, bool sfx)
    : m_impl(std::make_unique<Impl>(songID, sfx)) {}

std::optional<Song*> Nongs::findSong(const std::string& uniqueID) {
    return m_impl->findSong(uniqueID);
}
Result<> Nongs::commit() { return m_impl->commit(this); }
geode::Result<> Nongs::replaceSong(const std::string& id, LocalSong&& song) {
    return m_impl->replaceSong(id, std::move(song), this);
}
geode::Result<> Nongs::replaceSong(const std::string& id, YTSong&& song) {
    return m_impl->replaceSong(id, std::move(song), this);
}
geode::Result<> Nongs::replaceSong(const std::string& id, HostedSong&& song) {
    return m_impl->replaceSong(id, std::move(song), this);
}
geode::Result<> Nongs::registerIndexSong(index::IndexSongMetadata* song) {
    return m_impl->registerIndexSong(song);
}
bool Nongs::isDefaultActive() const { return m_impl->isDefaultActive(); }
int Nongs::songID() const { return m_impl->songID(); }
LocalSong* Nongs::defaultSong() const { return m_impl->defaultSong(); }
Song* Nongs::active() const { return m_impl->active(); }
Result<> Nongs::setActive(const std::string& uniqueID) {
    return m_impl->setActive(uniqueID, this);
}
Result<> Nongs::merge(Nongs&& other) { return m_impl->merge(std::move(other)); }
Result<> Nongs::deleteAllSongs() { return m_impl->deleteAllSongs(); }
Result<> Nongs::deleteSong(const std::string& uniqueID, bool audio) {
    return m_impl->deleteSong(uniqueID, audio, this);
}
Result<> Nongs::deleteSongAudio(const std::string& uniqueID) {
    return m_impl->deleteSongAudio(uniqueID);
}
std::vector<std::unique_ptr<LocalSong>>& Nongs::locals() const {
    return m_impl->locals();
}
std::vector<std::unique_ptr<YTSong>>& Nongs::youtube() const {
    return m_impl->youtube();
}
std::vector<std::unique_ptr<HostedSong>>& Nongs::hosted() const {
    return m_impl->hosted();
}
std::vector<index::IndexSongMetadata*>& Nongs::indexSongs() const {
    return m_impl->m_indexSongs;
}
geode::Result<LocalSong*> Nongs::add(LocalSong&& song) {
    return m_impl->add(std::move(song));
}
geode::Result<YTSong*> Nongs::add(YTSong&& song) {
    return m_impl->add(std::move(song));
}
geode::Result<HostedSong*> Nongs::add(HostedSong&& song) {
    return m_impl->add(std::move(song));
}

Nongs::Nongs(Nongs&& other) : m_impl(std::move(other.m_impl)) {}
Nongs& Nongs::operator=(Nongs&& other) {
    m_impl = std::move(other.m_impl);
    return *this;
}

Nongs::~Nongs() = default;

}  // namespace jukebox
