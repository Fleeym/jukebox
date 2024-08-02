#include "../include/nong.hpp"

#include "Geode/binding/MusicDownloadManager.hpp"

#include <filesystem>
#include <fmt/core.h>
#include <memory>
#include <system_error>

using namespace geode::prelude;

namespace jukebox {

class LocalSong::Impl {
private:
    friend class LocalSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::filesystem::path m_path;
public:
    Impl(
        SongMetadata&& metadata,
        const std::filesystem::path& path
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_path(path)
    {}

    Impl(const Impl& other)
        : m_path(other.m_path),
        m_metadata(std::make_unique<SongMetadata>(*other.m_metadata))
    {}
    Impl& operator=(const Impl&) = delete;

    ~Impl() = default;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    SongMetadata* metadata() const {
        return m_metadata.get();
    }
    std::filesystem::path path() const {
        return m_path;
    }
};

LocalSong::LocalSong(
    SongMetadata&& metadata,
    const std::filesystem::path& path
) : m_impl(std::make_unique<Impl>(std::move(metadata), path))
{}

LocalSong::LocalSong(const LocalSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl))
{}

LocalSong& LocalSong::operator=(const LocalSong& other) {
    m_impl->m_path = other.path();
    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);

    return *this;
}

LocalSong::LocalSong(LocalSong&&) = default;
LocalSong& LocalSong::operator=(LocalSong&&) = default;
LocalSong::~LocalSong() = default;

SongMetadata* LocalSong::metadata() const {
    return m_impl->metadata();
}
std::filesystem::path LocalSong::path() const {
    return m_impl->path();
}

LocalSong LocalSong::createUnknown(int songID) {
    return LocalSong {
        SongMetadata {
            songID,
            "Unknown",
            ""
        },
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(songID)
        )
    };
}

LocalSong LocalSong::fromSongObject(SongInfoObject* obj) {
    return LocalSong {
        SongMetadata {
            obj->m_songID,
            obj->m_songName,
            obj->m_artistName
        },
        std::filesystem::path(
            MusicDownloadManager::sharedState()->pathForSong(obj->m_songID)
        )
    };
}

class YTSong::Impl {
private:
    friend class YTSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_youtubeID;
    std::optional<std::unique_ptr<SongIndexMetadata>> m_indexMetadata;
    std::optional<std::filesystem::path> m_path;
public:
    Impl(
        SongMetadata&& metadata,
        std::string youtubeID,
        std::optional<SongIndexMetadata> indexMetadata,
        std::optional<std::filesystem::path> path = std::nullopt
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_youtubeID(youtubeID),
        m_indexMetadata(indexMetadata.has_value() ? std::optional(std::make_unique<SongIndexMetadata>(*indexMetadata)) : std::nullopt),
        m_path(path)
    {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
        m_path(other.m_path),
        m_indexMetadata(other.m_indexMetadata.has_value() ? std::optional(std::make_unique<SongIndexMetadata>(*other.m_indexMetadata.value())) : std::nullopt),
        m_youtubeID(other.m_youtubeID)
    {}
    Impl& operator=(const Impl&) = delete;

    ~Impl() = default;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    SongMetadata* metadata() const {
        return m_metadata.get();
    }
    std::optional<std::filesystem::path> path() const {
        return m_path;
    }
    std::string youtubeID() const {
        return m_youtubeID;
    }
    std::optional<SongIndexMetadata*> indexMetadata() const {
        return m_indexMetadata.has_value() ? std::optional(m_indexMetadata->get()) : std::nullopt;
    }
};

YTSong::YTSong(
    SongMetadata&& metadata,
    std::string youtubeID,
    std::optional<SongIndexMetadata> indexMetadata,
    std::optional<std::filesystem::path> path
) : m_impl(std::make_unique<Impl>(
    std::move(metadata),
    youtubeID,
    indexMetadata,
    path
)) {}

YTSong::YTSong(const YTSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl))
{}

YTSong& YTSong::operator=(const YTSong& other) {
    m_impl->m_youtubeID = other.m_impl->m_youtubeID;
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_indexMetadata = other.m_impl->m_indexMetadata.has_value() ? std::optional(std::make_unique<SongIndexMetadata>(*other.m_impl->m_indexMetadata.value())) : std::nullopt;
    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);

    return *this;
}

YTSong::YTSong(YTSong&& other) = default;
YTSong& YTSong::operator=(YTSong&& other) = default;
YTSong::~YTSong() = default;

SongMetadata* YTSong::metadata() const {
    return m_impl->metadata();
}

std::string YTSong::youtubeID() const {
    return m_impl->youtubeID();
}

std::optional<SongIndexMetadata*> YTSong::indexMetadata() const {
    return m_impl->indexMetadata();
}

std::optional<std::filesystem::path> YTSong::path() const {
    return m_impl->path();
}

class HostedSong::Impl {
private:
    friend class HostedSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_url;
    std::optional<std::unique_ptr<SongIndexMetadata>> m_indexMetadata;
    std::optional<std::filesystem::path> m_path;
public:
    Impl(
        SongMetadata&& metadata,
        std::string url,
        std::optional<SongIndexMetadata> indexMetadata,
        std::optional<std::filesystem::path> path = std::nullopt
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_url(url),
        m_indexMetadata(indexMetadata.has_value() ? std::optional(std::make_unique<SongIndexMetadata>(*indexMetadata)) : std::nullopt),
        m_path(path)
    {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
        m_path(other.m_path),
        m_indexMetadata(other.m_indexMetadata.has_value() ? std::optional(std::make_unique<SongIndexMetadata>(*other.m_indexMetadata.value())) : std::nullopt),
        m_url(other.m_url)
    {}
    Impl& operator=(const Impl& other) = delete;

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    ~Impl() = default;

    SongMetadata* metadata() const {
        return m_metadata.get();
    }
    std::string url() const {
        return m_url;
    }
    std::optional<SongIndexMetadata*> indexMetadata() const {
        return m_indexMetadata.has_value() ? std::optional(m_indexMetadata->get()) : std::nullopt;
    }
    std::optional<std::filesystem::path> path() const {
        return m_path;
    }
};

HostedSong::HostedSong(
    SongMetadata&& metadata,
    std::string url,
    std::optional<SongIndexMetadata> indexMetadata,
    std::optional<std::filesystem::path> path
) : m_impl(std::make_unique<Impl>(
    std::move(metadata),
    url,
    indexMetadata,
    path
)) {}

HostedSong::HostedSong(const HostedSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl))
{}

HostedSong& HostedSong::operator=(const HostedSong& other) {
    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_indexMetadata = other.m_impl->m_indexMetadata.has_value() ? std::optional(std::make_unique<SongIndexMetadata>(*other.m_impl->m_indexMetadata.value())) : std::nullopt;
    m_impl->m_url = other.m_impl->m_url;

    return *this;
}
SongMetadata* HostedSong::metadata() const {
    return m_impl->metadata();
}
std::string HostedSong::url() const {
    return m_impl->url();
}
std::optional<SongIndexMetadata*> HostedSong::indexMetadata() const {
    return m_impl->indexMetadata();
}
std::optional<std::filesystem::path> HostedSong::path() const {
    return m_impl->path();
}

HostedSong::HostedSong(HostedSong&& other) = default;
HostedSong& HostedSong::operator=(HostedSong&& other) = default;
HostedSong::~HostedSong() = default;

class Nongs::Impl {
private:
    friend class Nongs;

    int m_songID;
    std::unique_ptr<SongMetadataPathed> m_active;
    std::unique_ptr<LocalSong> m_default;
    std::vector<std::unique_ptr<LocalSong>> m_locals {};
    std::vector<std::unique_ptr<YTSong>> m_youtube {};
    std::vector<std::unique_ptr<HostedSong>> m_hosted {};

    void deletePath(const std::filesystem::path& path) {
        std::error_code ec;
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path, ec);
            if (ec) {
                log::error(
                    "Couldn't delete nong. Category: {}, message: {}",
                    ec.category().name(),
                    ec.category().message(ec.value())
                );
                ec = {};
            }
        }
    }
public:
    Impl(int songID, LocalSong&& defaultSong)
        : m_songID(songID),
        m_default(std::make_unique<LocalSong>(defaultSong)),
        // Initialize default song as active
        m_active(std::make_unique<SongMetadataPathed>(*defaultSong.metadata(), defaultSong.path()))
    {}

    Impl(int songID)
        : Impl(songID, LocalSong::createUnknown(songID))
    {}

    geode::Result<> setActive(const std::filesystem::path& path) {
        if (m_default->path() == path) {
            m_active = std::make_unique<SongMetadataPathed>(*m_default);
            return Ok();
        }

        for (const auto& i: m_locals) {
            if (i->path() == path) {
                m_active = std::make_unique<SongMetadataPathed>(*i);
                return Ok();
            }
        }

        for (const auto& i: m_youtube) {
            if (i->path().has_value() && i->path() == path) {
                m_active = std::make_unique<SongMetadataPathed>(*i);
                return Ok();
            }
        }

        for (const auto& i: m_hosted) {
            if (i->path().has_value() && i->path() == path) {
                m_active = std::make_unique<SongMetadataPathed>(*i);
                return Ok();
            }
        }

        return Err("No song found with given path for song ID");
    }

    geode::Result<> merge(Nongs&& other) {
        if (other.songID() != m_songID) return Err("Merging with NONGs of a different song ID");

        for (const auto& i : other.locals()) {
            if (i->path() == other.defaultSong()->path()) continue;
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
        for (auto& local : m_locals) {
            this->deletePath(local->path());
        }
        m_locals.clear();

        for (auto& youtube : m_youtube) {
            this->deletePath(youtube->path().value());
        }
        m_youtube.clear();

        for (auto& hosted : m_hosted) {
            this->deletePath(hosted->path().value());
        }
        m_hosted.clear();

        return Ok();
    }

    geode::Result<> deleteSong(const std::filesystem::path& path) {
        if (m_default->path() == path) {
            return Err("Cannot delete default song");
        }

        log::info("locals: {}", m_locals.size());
        for (auto i = m_locals.begin(); i != m_locals.end(); ++i) {
            log::info("Comparing {} and {}", (*i)->path(), path);
            if ((*i)->path() == path) {
                this->deletePath(path);
                m_locals.erase(i);
                return Ok();
            }
        }

        for (auto i = m_youtube.begin(); i != m_youtube.end(); ++i) {
            if ((*i)->path().has_value() && (*i)->path().value() == path) {
                this->deletePath(path);
                m_youtube.erase(i);
                return Ok();
            }
        }

        for (auto i = m_hosted.begin(); i != m_hosted.end(); ++i) {
            if ((*i)->path().has_value() && (*i)->path().value() == path) {
                this->deletePath(path);
                m_hosted.erase(i);
                return Ok();
            }
        }

        return Err("No song found with given path for song ID");
    }


    void resetToDefault() {
        auto _ = this->setActive(this->defaultSong()->path());
        for (const auto& i : m_locals) {
            this->deletePath(i->path());
        }
        m_locals.clear();

        for (const auto& i : m_youtube) {
            if (i->path().has_value()) {
                this->deletePath(i->path().value());
            }
        }
        m_youtube.clear();

        for (const auto& i : m_hosted) {
            if (i->path().has_value()) {
                this->deletePath(i->path().value());
            }
        }
        m_hosted.clear();
    }

    Result<LocalSong*> add(LocalSong song) {
        for (const auto& i : m_locals) {
            if (i->path() == song.path()) {
                std::string err = fmt::format("Attempted to add a duplicate song for id {}", song.metadata()->m_gdID);
                // log::error(err);
                return Err(err);
            }
        }

        auto s = std::make_unique<LocalSong>(song);
        auto ret = s.get();
        m_locals.push_back(std::move(s));
        return Ok(ret);
    }

    Result<YTSong*> add(YTSong song) {
        auto s = std::make_unique<YTSong>(song);
        auto ret = s.get();
        m_youtube.push_back(std::move(s));
        return Ok(ret);
    }

    Result<HostedSong*> add(HostedSong song) {
        auto s = std::make_unique<HostedSong>(song);
        auto ret = s.get();
        m_hosted.push_back(std::move(s));
        return Ok(ret);
    }

    bool isDefaultActive() const {
        return m_active->m_path == m_default->path();
    }
    int songID() const {
        return m_songID;
    }
    LocalSong* defaultSong() const {
        return m_default.get();
    }
    SongMetadataPathed* active() const {
        return m_active.get();
    }
    std::vector<std::unique_ptr<LocalSong>>& locals() {
        return m_locals;
    }
    std::vector<std::unique_ptr<YTSong>>& youtube() {
        return m_youtube;
    }
    std::vector<std::unique_ptr<HostedSong>>& hosted() {
        return m_hosted;
    }
};

Nongs::Nongs(int songID, LocalSong&& defaultSong)
    : m_impl(std::make_unique<Impl>(
        songID,
        std::move(defaultSong)
    ))
{}

Nongs::Nongs(int songID)
    : m_impl(std::make_unique<Impl>(songID))
{}

bool Nongs::isDefaultActive() const {
    return m_impl->isDefaultActive();
}

int Nongs::songID() const {
    return m_impl->songID();
}

LocalSong* Nongs::defaultSong() const {
    return m_impl->defaultSong();
}

SongMetadataPathed* Nongs::active() const {
    return m_impl->active();
}

Result<> Nongs::setActive(const std::filesystem::path& path) {
    return m_impl->setActive(path);
}

Result<> Nongs::merge(Nongs&& other) {
  return m_impl->merge(std::move(other));
}

Result<> Nongs::deleteAllSongs() {
    return m_impl->deleteAllSongs();
}

Result<> Nongs::deleteSong(const std::filesystem::path& path) {
    return m_impl->deleteSong(path);
}

void Nongs::resetToDefault() {
    m_impl->resetToDefault();
}

std::vector<std::unique_ptr<LocalSong>>& Nongs::locals() {
    return m_impl->locals();
}

std::vector<std::unique_ptr<YTSong>>& Nongs::youtube() {
    return m_impl->youtube();
}

std::vector<std::unique_ptr<HostedSong>>& Nongs::hosted() {
    return m_impl->hosted();
}

Result<LocalSong*> Nongs::add(LocalSong song) {
    return m_impl->add(song);
}

Result<YTSong*> Nongs::add(YTSong song) {
    return m_impl->add(song);
}

Result<HostedSong*> Nongs::add(HostedSong song) {
    return m_impl->add(song);
}

// Nongs::Nongs(const Nongs&) = delete;
// Nongs& Nongs::operator=(const Nongs&) = delete;
Nongs::Nongs(Nongs&&) = default;
Nongs& Nongs::operator=(Nongs&&) = default;
Nongs::~Nongs() = default;

class Nong::Impl {
private:
    friend class Nong;

    enum class Type {
        Local,
        YT,
        Hosted,
    };

    Type m_type;
    std::unique_ptr<LocalSong> m_localSong;
    std::unique_ptr<YTSong> m_ytSong;
    std::unique_ptr<HostedSong> m_hostedSong;

public:
    Impl(const LocalSong local) : m_type(Type::Local), m_localSong(std::make_unique<LocalSong>(local)) {}
    Impl(const YTSong yt) : m_type(Type::YT), m_ytSong(std::make_unique<YTSong>(yt)) {}
    Impl(const HostedSong hosted) : m_type(Type::Hosted), m_hostedSong(std::make_unique<HostedSong>(hosted)) {}

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    void visit(
        std::function<void(LocalSong*)> local,
        std::function<void(YTSong*)> yt,
        std::function<void(HostedSong*)> hosted
    ) const {
        switch (m_type) {
            case Type::Local:
                local(m_localSong.get());
                break;
            case Type::YT:
                yt(m_ytSong.get());
                break;
            case Type::Hosted:
                hosted(m_hostedSong.get());
                break;
        }
    }

    SongMetadata* metadata() const {
        switch (m_type) {
            case Type::Local:
                return m_localSong->metadata();
            case Type::YT:
                return m_ytSong->metadata();
            case Type::Hosted:
                return m_hostedSong->metadata();
        }
    }

    std::optional<std::filesystem::path> path() const {
      switch (m_type) {
          case Type::Local:
              return m_localSong->path();
          case Type::YT:
              return m_ytSong->path();
          case Type::Hosted:
              return m_hostedSong->path();
      }
    }

    Result<Nongs> toNongs() const {
        int songId = metadata()->m_gdID;
        switch (m_type) {
            case Type::Local: {
                auto nongs = Nongs(songId);
                if (auto err = nongs.add(std::move(*m_localSong)); err.isErr()) {
                    return Err(err.error());
                }
                return Ok(std::move(nongs));
            }
            case Type::YT: {
                auto nongs = Nongs(songId);
                if (auto err = nongs.add(std::move(*m_ytSong)); err.isErr()) {
                    return Err(err.error());
                }
                return Ok(std::move(nongs));
            }
            case Type::Hosted: {
                auto nongs = Nongs(songId);
                if (auto err = nongs.add(std::move(*m_hostedSong)); err.isErr()) {
                    return Err(err.error());
                }
                return Ok(std::move(nongs));
            }
        }
    }
};


Nong::Nong(const LocalSong& local)
    : m_impl(std::make_unique<Impl>(local))
{};

Nong::Nong(const YTSong& yt)
    : m_impl(std::make_unique<Impl>(yt))
{};

Nong::Nong(const HostedSong& hosted)
    : m_impl(std::make_unique<Impl>(hosted))
{};

SongMetadata* Nong::metadata() const {
    return m_impl->metadata();
};

Result<Nongs> Nong::toNongs() const {
    return m_impl->toNongs();
};

std::optional<std::filesystem::path> Nong::path() const {
    return m_impl->path();
};

void Nong::visit(
    std::function<void(LocalSong*)> local,
    std::function<void(YTSong*)> yt,
    std::function<void(HostedSong*)> hosted
) const {
    m_impl->visit(local, yt, hosted);
}

Nong::Nong(Nong&&) = default;
Nong& Nong::operator=(Nong&&) = default;
Nong::~Nong() = default;

}
