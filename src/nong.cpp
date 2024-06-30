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
    std::optional<std::filesystem::path> m_path;
public:
    Impl(
        SongMetadata&& metadata,
        std::string youtubeID,
        std::optional<std::filesystem::path> path = std::nullopt
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_youtubeID(youtubeID),
        m_path(path)
    {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
        m_path(other.m_path),
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
};

YTSong::YTSong(
    SongMetadata&& metadata,
    std::string youtubeID,
    std::optional<std::filesystem::path> path
) : m_impl(std::make_unique<Impl>(
    std::move(metadata),
    youtubeID,
    path
)) {}

YTSong::YTSong(const YTSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl))
{}

YTSong& YTSong::operator=(const YTSong& other) {
    m_impl->m_youtubeID = other.m_impl->m_youtubeID;
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);

    return *this;
}

SongMetadata* YTSong::metadata() const {
    return m_impl->metadata();
}

std::string YTSong::youtubeID() const {
    return m_impl->youtubeID();
}
std::optional<std::filesystem::path> YTSong::path() const {
    return m_impl->path();
}

class HostedSong::Impl {
private:
    friend class HostedSong;

    std::unique_ptr<SongMetadata> m_metadata;
    std::string m_url;
    std::optional<std::filesystem::path> m_path;
public:
    Impl(
        SongMetadata&& metadata,
        std::string url,
        std::optional<std::filesystem::path> path = std::nullopt
    ) : m_metadata(std::make_unique<SongMetadata>(metadata)),
        m_url(url),
        m_path(path)
    {}

    Impl(const Impl& other)
        : m_metadata(std::make_unique<SongMetadata>(*other.m_metadata)),
        m_path(other.m_path),
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
    std::optional<std::filesystem::path> path() const {
        return m_path;
    }
};

HostedSong::HostedSong(
    SongMetadata&& metadata,
    std::string url,
    std::optional<std::filesystem::path> path
) : m_impl(std::make_unique<Impl>(
    std::move(metadata),
    url,
    path
)) {}

HostedSong::HostedSong(const HostedSong& other)
    : m_impl(std::make_unique<Impl>(*other.m_impl))
{}

HostedSong& HostedSong::operator=(const HostedSong& other) {
    m_impl->m_metadata = std::make_unique<SongMetadata>(*other.m_impl->m_metadata);
    m_impl->m_path = other.m_impl->m_path;
    m_impl->m_url = other.m_impl->m_url;

    return *this;
}
SongMetadata* HostedSong::metadata() const {
    return m_impl->metadata();
}
std::string HostedSong::url() const {
    return m_impl->url();
}
std::optional<std::filesystem::path> HostedSong::path() const {
    return m_impl->path();
}

class Nongs::Impl {
private:
    friend class Nongs;

    int m_songID;
    std::unique_ptr<ActiveSong> m_active = std::make_unique<ActiveSong>();
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
        m_default(std::make_unique<LocalSong>(defaultSong))
    {
        // Initialize default song as active
        m_active->metadata = m_default->metadata();
        m_active->path = m_default->path();
    }

    geode::Result<> setActive(const std::filesystem::path& path) {
        if (m_default->path() == path) {
            m_active->path = m_default->path();
            m_active->metadata = m_default->metadata();
            return Ok();
        }

        for (const auto& i: m_locals) {
            if (i->path() == path) {
                m_active->path = i->path();
                m_active->metadata = i->metadata();
                return Ok();
            }
        }

        for (const auto& i: m_youtube) {
            if (i->path().has_value() && i->path() == path) {
                m_active->path = i->path().value();
                m_active->metadata = i->metadata();
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
                log::error(err);
                return Err(err);
            }
        }

        auto s = std::make_unique<LocalSong>(song);
        auto ret = s.get();
        m_locals.push_back(std::move(s));
        return Ok(ret);
    }

    Result<YTSong*> add(YTSong song) {
        for (const auto& i : m_youtube) {
            if (i->youtubeID() == song.youtubeID()) {
                std::string err = fmt::format("Attempted to add a duplicate song for id {}", song.metadata()->m_gdID);
                log::error(err);
                return Err(err);
            }
        }

        auto s = std::make_unique<YTSong>(song);
        auto ret = s.get();
        m_youtube.push_back(std::move(s));
        return Ok(ret);
    }

    Result<HostedSong*> add(HostedSong song) {
        for (const auto& i : m_hosted) {
            if (i->url() == song.url()) {
                std::string err = fmt::format("Attempted to add a duplicate song for id {}", song.metadata()->m_gdID);
                log::error(err);
                return Err(err);
            }
        }

        auto s = std::make_unique<HostedSong>(song);
        auto ret = s.get();
        m_hosted.push_back(std::move(s));
        return Ok(ret);
    }

    bool isDefaultActive() const {
        return m_active->path == m_default->path();
    }
    int songID() const {
        return m_songID;
    }
    LocalSong* defaultSong() const {
        return m_default.get();
    }
    ActiveSong* active() const {
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

bool Nongs::isDefaultActive() const {
    return m_impl->isDefaultActive();
}

int Nongs::songID() const {
    return m_impl->songID();
}

LocalSong* Nongs::defaultSong() const {
    return m_impl->defaultSong();
}

Nongs::ActiveSong* Nongs::active() const {
    return m_impl->active();
}

Result<> Nongs::setActive(const std::filesystem::path& path) {
    return m_impl->setActive(path);
}

void Nongs::resetToDefault() {
    m_impl->resetToDefault();    
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

}