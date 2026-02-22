#include <jukebox/nong/nong.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <Geode/Result.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Log.hpp>
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
using namespace arc;

namespace jukebox {

class LocalSong::Impl {
private:
    friend class LocalSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::filesystem::path m_path;

public:
    Impl(SongMetadata&& metadata, std::filesystem::path path)
        : m_metadata(std::make_unique<SongMetadata>(metadata)), m_path(std::move(path)) {}

    Impl(const Impl& other) : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)), m_path(other.m_path) {}
    Impl& operator=(const Impl&) = delete;

    ~Impl() = default;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    [[nodiscard]] SongMetadata* metadata() const { return m_metadata.get(); }
    [[nodiscard]] std::filesystem::path path() const { return m_path; }
    void setPath(std::filesystem::path&& p) { m_path = p; }
};

LocalSong::LocalSong(SongMetadata&& metadata, const std::filesystem::path& path)
    : m_impl(std::make_unique<Impl>(std::move(metadata), path)) {}

LocalSong::LocalSong(const LocalSong& other) : m_impl(std::make_unique<Impl>(*other.m_impl)) {}

LocalSong& LocalSong::operator=(const LocalSong& other) {
    if (this == &other) {
        return *this;
    }

    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);

    return *this;
}

LocalSong::LocalSong(LocalSong&&) noexcept = default;
LocalSong& LocalSong::operator=(LocalSong&&) noexcept = default;
LocalSong::~LocalSong() = default;

SongMetadata* LocalSong::metadata() const { return m_impl->metadata(); }
std::optional<std::filesystem::path> LocalSong::path() const { return m_impl->path(); }
void LocalSong::setPath(std::filesystem::path p) { m_impl->setPath(std::move(p)); }

LocalSong LocalSong::createUnknown(const int songID) {
    return LocalSong{SongMetadata{songID, random_string(16), "Unknown", ""},
                     std::filesystem::path(MusicDownloadManager::sharedState()->pathForSong(songID))};
}

LocalSong LocalSong::fromSongObject(const SongInfoObject* obj) {
    return LocalSong{SongMetadata{obj->m_songID, random_string(16), obj->m_songName, obj->m_artistName},
                     std::filesystem::path(MusicDownloadManager::sharedState()->pathForSong(obj->m_songID))};
}

class YTSong::Impl {
private:
    friend class YTSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_youtubeID;
    std::optional<std::string> m_indexID;
    std::optional<std::filesystem::path> m_path;

public:
    Impl(SongMetadata&& metadata, std::string youtubeID, std::optional<std::string> indexID,
         std::optional<std::filesystem::path> path = std::nullopt)
        : m_metadata(std::make_unique<SongMetadata>(metadata)),
          m_youtubeID(std::move(youtubeID)),
          m_indexID(std::move(indexID)),
          m_path(std::move(path)) {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
          m_youtubeID(other.m_youtubeID),
          m_indexID(other.m_indexID),
          m_path(other.m_path) {}
    Impl& operator=(const Impl&) = delete;

    ~Impl() = default;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    [[nodiscard]] SongMetadata* metadata() const { return m_metadata.get(); }
    [[nodiscard]] std::optional<std::filesystem::path> path() const { return m_path; }
    [[nodiscard]] std::string youtubeID() const { return m_youtubeID; }
    [[nodiscard]] std::optional<std::string> indexID() const { return m_indexID; }
    Future<Result<ByteVector>> startDownload() const {
        if (std::error_code ec; m_path.has_value() && std::filesystem::exists(m_path.value(), ec)) {
            co_return Err("Song already is downloaded");
        }

        co_return co_await download::startYoutubeDownload(m_youtubeID);
    }
    void setPath(std::filesystem::path&& p) { m_path = p; }
};

YTSong::YTSong(SongMetadata&& metadata, std::string youtubeID, std::optional<std::string> indexID,
               std::optional<std::filesystem::path> path)
    : m_impl(std::make_unique<Impl>(std::move(metadata), std::move(youtubeID), std::move(indexID), std::move(path))) {}

YTSong::YTSong(const YTSong& other) : m_impl(std::make_unique<Impl>(*other.m_impl)) {}

YTSong& YTSong::operator=(const YTSong& other) {
    if (this == &other) {
        return *this;
    }

    m_impl->m_youtubeID = other.m_impl->m_youtubeID;
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_indexID = other.m_impl->m_indexID;
    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);

    return *this;
}

YTSong::YTSong(YTSong&& other) noexcept = default;
YTSong& YTSong::operator=(YTSong&& other) noexcept = default;
YTSong::~YTSong() = default;

SongMetadata* YTSong::metadata() const { return m_impl->metadata(); }

std::string YTSong::youtubeID() const { return m_impl->youtubeID(); }

std::optional<std::string> YTSong::indexID() const { return m_impl->indexID(); }
void YTSong::setIndexID(const std::string& id) { m_impl->m_indexID = id; }

std::optional<std::filesystem::path> YTSong::path() const { return m_impl->path(); }
void YTSong::setPath(std::filesystem::path p) { m_impl->setPath(std::move(p)); }

Future<Result<ByteVector>> YTSong::startDownload() const { return m_impl->startDownload(); }

class HostedSong::Impl {
private:
    friend class HostedSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_url;
    std::optional<std::string> m_indexID;
    std::optional<std::filesystem::path> m_path;

public:
    Impl(SongMetadata&& metadata, std::string url, std::optional<std::string> indexID,
         std::optional<std::filesystem::path> path = std::nullopt)
        : m_metadata(std::make_unique<SongMetadata>(metadata)),
          m_url(std::move(url)),
          m_indexID(std::move(indexID)),
          m_path(std::move(path)) {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
          m_url(other.m_url),
          m_indexID(other.m_indexID),
          m_path(other.m_path) {}
    Impl& operator=(const Impl& other) = delete;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    ~Impl() = default;

    [[nodiscard]] SongMetadata* metadata() const noexcept { return m_metadata.get(); }
    [[nodiscard]] std::string url() const noexcept { return m_url; }
    [[nodiscard]] std::optional<std::string> indexID() const noexcept { return m_indexID; }
    [[nodiscard]] std::optional<std::filesystem::path> path() const noexcept { return m_path; }
    Future<Result<ByteVector>> startDownload() const {
        if (std::error_code ec; m_path.has_value() && std::filesystem::exists(m_path.value(), ec)) {
            co_return Err("Song already is downloaded");
        }

        co_return co_await download::startHostedDownload(m_url);
    }
    void setPath(std::filesystem::path&& p) { m_path = p; }
};

HostedSong::HostedSong(SongMetadata&& metadata, std::string url, std::optional<std::string> indexID,
                       std::optional<std::filesystem::path> path)
    : m_impl(std::make_unique<Impl>(std::move(metadata), std::move(url), std::move(indexID), std::move(path))) {}

HostedSong::HostedSong(const HostedSong& other) : m_impl(std::make_unique<Impl>(*other.m_impl)) {}

HostedSong& HostedSong::operator=(const HostedSong& other) {
    if (this == &other) {
        return *this;
    }

    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_indexID = other.m_impl->m_indexID;
    m_impl->m_url = other.m_impl->m_url;

    return *this;
}
SongMetadata* HostedSong::metadata() const { return m_impl->metadata(); }
std::string HostedSong::url() const { return m_impl->url(); }
std::optional<std::string> HostedSong::indexID() const { return m_impl->indexID(); }
void HostedSong::setIndexID(const std::string& id) { m_impl->m_indexID = id; }
std::optional<std::filesystem::path> HostedSong::path() const { return m_impl->path(); }
void HostedSong::setPath(std::filesystem::path p) { m_impl->setPath(std::move(p)); }

Future<Result<ByteVector>> HostedSong::startDownload() const { return m_impl->startDownload(); }

HostedSong::HostedSong(HostedSong&& other) noexcept = default;
HostedSong& HostedSong::operator=(HostedSong&& other) noexcept = default;
HostedSong::~HostedSong() = default;

class Nongs::Impl {
private:
    friend class Nongs;

    int m_songID;
    Song* m_active;
    std::unique_ptr<LocalSong> m_default;
    std::vector<std::unique_ptr<LocalSong>> m_locals;
    std::vector<std::unique_ptr<YTSong>> m_youtube;
    std::vector<std::unique_ptr<HostedSong>> m_hosted;

    std::vector<IndexSongMetadata*> m_indexSongs;

    void deletePath(const std::optional<std::filesystem::path>& path) {
        std::error_code ec;
        if (path.has_value() && std::filesystem::exists(path.value(), ec)) {
            std::filesystem::remove(path.value(), ec);
            if (ec) {
                log::error("Couldn't delete nong. Category: {}, message: {}", ec.category().name(),
                           ec.category().message(ec.value()));
            }
        }
    }

public:
    Impl(const int songID, std::unique_ptr<LocalSong> defaultSong)
        : m_songID(songID), m_active(defaultSong.get()), m_default(std::move(defaultSong)) {}

    explicit Impl(const int songID) : Impl(songID, std::make_unique<LocalSong>(LocalSong::createUnknown(songID))) {}

    Result<> commit(Nongs* self) {
        const std::filesystem::path path = NongManager::get().baseManifestPath() / fmt::format("{}.json", m_songID);

        // Don't save manifest for songs with no nongs
        if (m_locals.empty() && m_youtube.empty() && m_hosted.empty()) {
            if (std::error_code ec; std::filesystem::exists(path, ec)) {
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

    Result<> canSetActive(const std::string& uniqueID, const std::filesystem::path& path) const {
        const bool IS_DEFAULT = uniqueID == m_default->metadata()->uniqueID;

        if (IS_DEFAULT) {
            return Ok();
        }

        if (std::error_code ec; !std::filesystem::exists(path, ec)) {
            return Err("Song doesn't exist on disk");
        }

        return Ok();
    }

    Result<> setActive(const std::string& uniqueID, Nongs* self) {
        if (const std::optional<LocalSong*> song = this->getLocalFromID(uniqueID)) {
            Result<> res = this->canSetActive(uniqueID, song.value()->path().value());
            if (res.isErr()) {
                return res;
            }

            m_active = song.value();

            if (NongManager::get().initialized()) {
                event::SongStateChanged().send(event::SongStateChangedData{self});
            }

            return Ok();
        }

        if (const std::optional<YTSong*> song = this->getYTFromID(uniqueID)) {
            if (!song.value()->path()) {
                return Err("Song is not downloaded");
            }
            Result<> res = this->canSetActive(uniqueID, song.value()->path().value());
            if (res.isErr()) {
                return res;
            }

            m_active = song.value();
            if (NongManager::get().initialized()) {
                event::SongStateChanged().send(event::SongStateChangedData{self});
            }
            return Ok();
        }

        if (const std::optional<HostedSong*> song = this->getHostedFromID(uniqueID)) {
            if (!song.value()->path()) {
                return Err("Song is not downloaded");
            }
            Result<> res = this->canSetActive(uniqueID, song.value()->path().value());
            if (res.isErr()) {
                return res;
            }

            m_active = song.value();
            if (NongManager::get().initialized()) {
                event::SongStateChanged().send(event::SongStateChangedData{self});
            }
            return Ok();
        }

        return Err("No song found with given path for song ID");
    }

    Result<> merge(Nongs&& other) {
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

    Result<> deleteAllSongs(Nongs* self) {
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
        event::SongStateChanged().send(event::SongStateChangedData{self});

        return Ok();
    }

    Result<> deleteSong(const std::string& uniqueID, bool audio, Nongs* self) {
        if (m_default->metadata()->uniqueID == uniqueID) {
            return Err("Cannot delete default song");
        }

        if (m_active->metadata()->uniqueID == uniqueID) {
            (void)this->setActive(m_default->metadata()->uniqueID, self);
        }

        for (auto i = m_locals.begin(); i != m_locals.end(); ++i) {
            if ((*i)->metadata()->uniqueID == uniqueID) {
                if (audio) {
                    this->deletePath((*i)->path());
                }
                m_locals.erase(i);
                if (NongManager::get().initialized()) {
                    event::NongDeleted(m_songID).send(event::NongDeletedData{m_songID, uniqueID});
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
                    event::NongDeleted(m_songID).send(event::NongDeletedData{m_songID, uniqueID});
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
                    event::NongDeleted(m_songID).send(event::NongDeletedData{m_songID, uniqueID});
                }
                return Ok();
            }
        }

        return Err("No song found with given path for song ID");
    }

    Result<> deleteSongAudio(const std::string& uniqueID) {
        if (m_default->metadata()->uniqueID == uniqueID) {
            return Err("Cannot delete audio of the default song");
        }

        if (m_active->metadata()->uniqueID == uniqueID) {
            m_active = m_default.get();
        }

        for (const auto& m_local : m_locals) {
            if (m_local->metadata()->uniqueID == uniqueID) {
                return Err("Cannot delete audio of local songs");
            }
        }

        for (const auto& i : m_youtube) {
            if (i->metadata()->uniqueID == uniqueID) {
                this->deletePath(i->path());
                return Ok();
            }
        }

        for (const auto& i : m_hosted) {
            if (i->metadata()->uniqueID == uniqueID) {
                this->deletePath(i->path());
                return Ok();
            }
        }

        return Err("No song found with given path for song ID");
    }

    [[nodiscard]] std::optional<LocalSong*> getLocalFromID(const std::string_view uniqueID) const {
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

    [[nodiscard]] std::optional<YTSong*> getYTFromID(const std::string_view uniqueID) const {
        for (const std::unique_ptr<YTSong>& i : m_youtube) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] std::optional<HostedSong*> getHostedFromID(const std::string_view uniqueID) const {
        for (const std::unique_ptr<HostedSong>& i : m_hosted) {
            if (i->metadata()->uniqueID == uniqueID) {
                return i.get();
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] std::optional<Song*> findSong(const std::string& uniqueID) const {
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

    Result<> replaceSong(const std::string& id, LocalSong&& song, Nongs* self) {
        const bool isActive = m_active->metadata()->uniqueID == id;
        const std::optional<Song*> opt = this->findSong(id);
        if (!opt) {
            return Err(fmt::format("NONG ID {} not found", id));
        }
        const Song* prevNong = opt.value();
        const std::optional<std::filesystem::path> prevPath = prevNong->path();

        const bool deleteAudio = prevPath != song.path();
        if (GEODE_UNWRAP_IF_ERR(err, this->deleteSong(id, deleteAudio, self))) {
            log::warn("Failed to delete song: {}", err);
        }

        GEODE_UNWRAP(this->add(std::move(song)));

        if (isActive) {
            if (GEODE_UNWRAP_IF_ERR(err, this->setActive(id, self))) {
                log::warn("Failed to set song as active: {}", err);
            }
        }
        return Ok();
    }

    Result<> replaceSong(const std::string& id, YTSong&& song, Nongs* self) {
        const bool isActive = m_active->metadata()->uniqueID == id;
        const std::optional<Song*> opt = this->findSong(id);
        if (!opt) {
            return Err(fmt::format("NONG ID {} not found", id));
        }
        const Song* prevNong = opt.value();
        const std::optional<std::filesystem::path> prevPath = prevNong->path();

        const bool deleteAudio = prevPath != song.path();
        if (GEODE_UNWRAP_IF_ERR(err, this->deleteSong(id, deleteAudio, self))) {
            log::warn("Failed to delete song: {}", err);
        }

        YTSong* added = nullptr;

        GEODE_UNWRAP_INTO(added, this->add(std::move(song)));

        if (isActive && added->path().has_value()) {
            if (GEODE_UNWRAP_IF_ERR(err, this->setActive(id, self))) {
                log::warn("Failed to set song as active: {}", err);
            }
        }
        return Ok();
    }

    Result<> replaceSong(const std::string& id, HostedSong&& song, Nongs* self) {
        const bool isActive = m_active->metadata()->uniqueID == id;
        const std::optional<Song*> opt = this->findSong(id);
        if (!opt) {
            return Err(fmt::format("NONG ID {} not found", id));
        }
        const Song* prevNong = opt.value();
        const std::optional<std::filesystem::path> prevPath = prevNong->path();

        const bool deleteAudio = prevPath != song.path();
        if (GEODE_UNWRAP_IF_ERR(err, this->deleteSong(id, deleteAudio, self))) {
            log::warn("Failed to delete song: {}", err);
        }

        HostedSong* added = nullptr;

        GEODE_UNWRAP_INTO(added, this->add(std::move(song)));

        if (isActive && added->path().has_value()) {
            if (GEODE_UNWRAP_IF_ERR(err, this->setActive(id, self))) {
                log::warn("Failed to set song as active: {}", err);
            }
        }
        return Ok();
    }

    Result<LocalSong*> add(LocalSong&& song) {
        for (const std::unique_ptr<LocalSong>& i : m_locals) {
            if (i->metadata() == song.metadata() && i->path() == song.path()) {
                std::string err = fmt::format("Attempted to add a duplicate song for id {}", song.metadata()->gdID);
                return Err(err);
            }
        }

        auto ptr = std::make_unique<LocalSong>(std::move(song));
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
                std::string err = fmt::format("Attempted to add a duplicate song for id {}", song.metadata()->gdID);
                return Err(err);
            }
        }

        auto s = std::make_unique<HostedSong>(std::move(song));
        HostedSong* ret = s.get();

        m_hosted.push_back(std::move(s));

        return Ok(ret);
    }

    [[nodiscard]] bool isDefaultActive() const {
        return m_active->metadata()->uniqueID == m_default->metadata()->uniqueID;
    }

    Result<> registerIndexSong(IndexSongMetadata* song) {
        bool hasSongID = false;
        for (const int id : song->songIDs) {
            if (id == m_songID) {
                hasSongID = true;
            }
        }

        if (!hasSongID) {
            return Err("Index song {} doesn't apply for ID {}", song->uniqueID, m_songID);
        }

        for (const IndexSongMetadata* i : m_indexSongs) {
            if (i == song) {
                return Err("Song {} already registered for ID {}", song->uniqueID, m_songID);
            }
        }

        m_indexSongs.push_back(song);
        return Ok();
    }

    [[nodiscard]] int songID() const { return m_songID; }
    [[nodiscard]] LocalSong* defaultSong() const { return m_default.get(); }
    [[nodiscard]] Song* active() const { return m_active; }
    std::vector<std::unique_ptr<LocalSong>>& locals() { return m_locals; }
    std::vector<std::unique_ptr<YTSong>>& youtube() { return m_youtube; }
    std::vector<std::unique_ptr<HostedSong>>& hosted() { return m_hosted; }
};

Nongs::Nongs(int songID, LocalSong&& defaultSong)
    : m_impl(std::make_unique<Impl>(songID, std::make_unique<LocalSong>(defaultSong))) {}
Nongs::Nongs(int songID) : m_impl(std::make_unique<Impl>(songID)) {}

std::optional<Song*> Nongs::findSong(const std::string& uniqueID) const { return m_impl->findSong(uniqueID); }
Result<> Nongs::commit() { return m_impl->commit(this); }
Result<> Nongs::replaceSong(const std::string& id, LocalSong&& song) {
    return m_impl->replaceSong(id, std::move(song), this);
}
Result<> Nongs::replaceSong(const std::string& id, YTSong&& song) {
    return m_impl->replaceSong(id, std::move(song), this);
}
Result<> Nongs::replaceSong(const std::string& id, HostedSong&& song) {
    return m_impl->replaceSong(id, std::move(song), this);
}
Result<> Nongs::registerIndexSong(IndexSongMetadata* song) const { return m_impl->registerIndexSong(song); }
bool Nongs::isDefaultActive() const { return m_impl->isDefaultActive(); }
int Nongs::songID() const { return m_impl->songID(); }
LocalSong* Nongs::defaultSong() const { return m_impl->defaultSong(); }
Song* Nongs::active() const { return m_impl->active(); }
Result<> Nongs::setActive(const std::string& uniqueID) { return m_impl->setActive(uniqueID, this); }
Result<> Nongs::merge(Nongs&& other) { return m_impl->merge(std::move(other)); }
Result<> Nongs::deleteAllSongs() { return m_impl->deleteAllSongs(this); }
Result<> Nongs::deleteSong(const std::string& uniqueID, bool audio) {
    return m_impl->deleteSong(uniqueID, audio, this);
}
Result<> Nongs::deleteSongAudio(const std::string& uniqueID) const { return m_impl->deleteSongAudio(uniqueID); }
std::vector<std::unique_ptr<LocalSong>>& Nongs::locals() const { return m_impl->locals(); }
std::vector<std::unique_ptr<YTSong>>& Nongs::youtube() const { return m_impl->youtube(); }
std::vector<std::unique_ptr<HostedSong>>& Nongs::hosted() const { return m_impl->hosted(); }
std::vector<IndexSongMetadata*>& Nongs::indexSongs() const { return m_impl->m_indexSongs; }
Result<LocalSong*> Nongs::add(LocalSong&& song) const { return m_impl->add(std::move(song)); }
Result<YTSong*> Nongs::add(YTSong&& song) const { return m_impl->add(std::move(song)); }
Result<HostedSong*> Nongs::add(HostedSong&& song) const { return m_impl->add(std::move(song)); }

Nongs::Nongs(Nongs&& other) noexcept : m_impl(std::move(other.m_impl)) {}
Nongs& Nongs::operator=(Nongs&& other) noexcept {
    m_impl = std::move(other.m_impl);
    return *this;
}

Nongs::~Nongs() = default;

}  // namespace jukebox
