#pragma once

#include <fmt/core.h>
#include <filesystem>
#include <matjson.hpp>
#include <memory>
#include <optional>

#include "Geode/Result.hpp"
#include "Geode/loader/Log.hpp"
#include "Geode/utils/string.hpp"

#include "nong.hpp"

template <>
struct matjson::Serialize<jukebox::SongMetadata> {
    static geode::Result<jukebox::SongMetadata> fromJson(
        const matjson::Value& value, int songID) {
        if (!value["name"].isString()) {
            return geode::Err("Invalid JSON key name");
        }
        if (!value["artist"].isString()) {
            return geode::Err("Invalid JSON key artist");
        }
        if (!value["unique_id"].isString()) {
            return geode::Err("Invalid JSON key artist");
        }

        return geode::Ok(jukebox::SongMetadata{
            songID, value["unique_id"].asString().unwrap(),
            value["name"].asString().unwrap(),
            value["artist"].asString().unwrap(),
            value["level"]
                .asString()
                .map([](auto i) { return std::optional(i); })
                .unwrapOr(std::nullopt),
            static_cast<int>(value["offset"].asInt().unwrapOr(0))});
    }
};

template <>
struct matjson::Serialize<jukebox::LocalSong> {
    static geode::Result<jukebox::LocalSong> fromJson(
        const matjson::Value& value, int songID) {
        GEODE_UNWRAP_INTO(
            jukebox::SongMetadata metadata,
            matjson::Serialize<jukebox::SongMetadata>::fromJson(value, songID)
                .mapErr([value](std::string err) {
                    return fmt::format("Local Song {} is invalid. Reason: {}",
                                       value.dump(matjson::NO_INDENTATION),
                                       err);
                }));

        if (!value["path"].isString()) {
            return geode::Err("Local Song {} is invalid. Reason: invalid path",
                              value.dump(matjson::NO_INDENTATION));
        }

        std::string path = value["path"].asString().unwrap();

#ifdef GEODE_IS_WINDOWS
        std::filesystem::path p = geode::utils::string::utf8ToWide(path);
#else
        std::filesystem::path p = path;
#endif

        return geode::Ok(jukebox::LocalSong{std::move(metadata), std::move(p)});
    }

    static matjson::Value toJson(const jukebox::LocalSong& value) {
#ifdef GEODE_IS_WINDOWS
        std::string path =
            geode::utils::string::wideToUtf8(value.path().value().wstring());
#else
        std::string path = value.path().value().string();
#endif
        matjson::Value ret = matjson::makeObject({
            {"name", value.metadata()->name},
            {"unique_id", value.metadata()->uniqueID},
            {"artist", value.metadata()->artist},
            {"path", path},
            {"offset", value.metadata()->startOffset},
        });
        if (value.metadata()->level.has_value()) {
            ret["level"] = value.metadata()->level.value();
        }
        return ret;
    }
};

template <>
struct matjson::Serialize<jukebox::YTSong> {
    static geode::Result<jukebox::YTSong> fromJson(const matjson::Value& value,
                                                   int songID) {
        GEODE_UNWRAP_INTO(
            jukebox::SongMetadata metadata,
            matjson::Serialize<jukebox::SongMetadata>::fromJson(value, songID)
                .mapErr([value](std::string err) {
                    return fmt::format("YouTube song {} is invalid. Reason: {}",
                                       value.dump(matjson::NO_INDENTATION),
                                       err);
                }));

        if (!value["path"].isString()) {
            return geode::Err(
                "YouTube song {} is invalid. Reason: invalid path",
                value.dump(matjson::NO_INDENTATION));
        }

        if (!value["youtube_id"].isString()) {
            return geode::Err(
                "YouTube song {} is invalid. Reason: invalid youtube ID",
                value.dump(matjson::NO_INDENTATION));
        }

        return geode::Ok(jukebox::YTSong{
            std::move(metadata), value["youtube_id"].asString().unwrap(),
            value["index_id"]
                .asString()
                .map([](auto i) { return std::optional(i); })
                .unwrapOr(std::nullopt),
            value["path"].asString().unwrap()});
    }

    static matjson::Value toJson(const jukebox::YTSong& value) {
#ifdef GEODE_IS_WINDOWS
        std::string path =
            geode::utils::string::wideToUtf8(value.path().value().wstring());
#else
        std::string path = value.path().value().string();
#endif
        matjson::Value ret =
            matjson::makeObject({{"name", value.metadata()->name},
                                 {"unique_id", value.metadata()->uniqueID},
                                 {"artist", value.metadata()->artist},
                                 {"path", path},
                                 {"offset", value.metadata()->startOffset},
                                 {"youtube_id", value.youtubeID()}});
        if (value.indexID().has_value()) {
            ret["index_id"] = value.indexID().value();
        }
        if (value.metadata()->level.has_value()) {
            ret["level"] = value.metadata()->level.value();
        }

        return ret;
    }
};

template <>
struct matjson::Serialize<jukebox::HostedSong> {
    static geode::Result<jukebox::HostedSong> fromJson(
        const matjson::Value& value, int songID) {
        GEODE_UNWRAP_INTO(
            jukebox::SongMetadata metadata,
            matjson::Serialize<jukebox::SongMetadata>::fromJson(value, songID)
                .mapErr([value](std::string err) {
                    return fmt::format("Hosted song {} is invalid. Reason: {}",
                                       value.dump(matjson::NO_INDENTATION),
                                       err);
                }));

        if (!value["path"].isString()) {
            return geode::Err("Hosted song {} is invalid. Reason: invalid path",
                              value.dump(matjson::NO_INDENTATION));
        }

        if (!value["url"].isString()) {
            return geode::Err("Hosted song {} is invalid. Reason: invalid url",
                              value.dump(matjson::NO_INDENTATION));
        }

        return geode::Ok(jukebox::HostedSong{
            std::move(metadata), value["url"].asString().unwrap(),
            value["index_id"]
                .asString()
                .map([](auto i) { return std::optional(i); })
                .unwrapOr(std::nullopt),
            value["path"].asString().unwrap()});
    }

    static matjson::Value toJson(const jukebox::HostedSong& value) {
#ifdef GEODE_IS_WINDOWS
        std::string path =
            geode::utils::string::wideToUtf8(value.path().value().wstring());
#else
        std::string path = value.path().value().string();
#endif
        matjson::Value ret =
            matjson::makeObject({{"name", value.metadata()->name},
                                 {"unique_id", value.metadata()->uniqueID},
                                 {"artist", value.metadata()->artist},
                                 {"path", path},
                                 {"offset", value.metadata()->startOffset},
                                 {"url", value.url()}});
        if (value.indexID().has_value()) {
            ret["index_id"] = value.indexID();
        }

        if (value.metadata()->level.has_value()) {
            ret["level"] = value.metadata()->level.value();
        }
        return ret;
    }
};

template <>
struct matjson::Serialize<jukebox::Nongs> {
    static matjson::Value toJson(jukebox::Nongs& value) {
        matjson::Value ret = matjson::makeObject({});

        ret["default"] = matjson::Serialize<jukebox::LocalSong>::toJson(
            *value.defaultSong());

        ret["active"] = value.active()->metadata()->uniqueID;

        matjson::Value locals = matjson::Value::array();

        for (std::unique_ptr<jukebox::LocalSong>& local : value.locals()) {
            locals.push(matjson::Serialize<jukebox::LocalSong>::toJson(*local));
        }

        ret["locals"] = locals;

        matjson::Value youtubes = matjson::Value::array();
        for (std::unique_ptr<jukebox::YTSong>& youtube : value.youtube()) {
            if (!youtube->path().has_value()) {
                continue;
            }
            youtubes.push(
                matjson::Serialize<jukebox::YTSong>::toJson(*youtube));
        }
        ret["youtube"] = youtubes;

        matjson::Value hosteds = matjson::Value::array();
        for (std::unique_ptr<jukebox::HostedSong>& hosted : value.hosted()) {
            if (!hosted->path().has_value()) {
                continue;
            }
            hosteds.push(
                matjson::Serialize<jukebox::HostedSong>::toJson(*hosted));
        }
        ret["hosted"] = hosteds;

        return ret;
    }

    static geode::Result<jukebox::Nongs> fromJson(const matjson::Value& value,
                                                  int songID) {
        if (!value["default"].isObject()) {
            return geode::Err("Invalid nongs object for id {}", songID);
        }

        GEODE_UNWRAP_INTO(jukebox::LocalSong defaultSong,
                          matjson::Serialize<jukebox::LocalSong>::fromJson(
                              value["default"], songID)
                              .mapErr([songID](std::string err) {
                                  return fmt::format(
                                      "Failed to parse default song for ID {}",
                                      songID);
                              }));

        jukebox::Nongs nongs = {songID, std::move(defaultSong)};

        if (value["locals"].isArray()) {
            for (const matjson::Value& local :
                 value["locals"].asArray().unwrap()) {
                geode::Result<jukebox::LocalSong> res =
                    matjson::Serialize<jukebox::LocalSong>::fromJson(local,
                                                                     songID);

                if (res.isErr()) {
                    geode::log::error("{}", res.unwrapErr());
                    continue;
                }

                nongs.locals().push_back(
                    std::make_unique<jukebox::LocalSong>(res.unwrap()));
            }
        }

        if (value["youtube"].isArray()) {
            for (const matjson::Value& yt :
                 value["youtube"].asArray().unwrap()) {
                geode::Result<jukebox::YTSong> res =
                    matjson::Serialize<jukebox::YTSong>::fromJson(yt, songID);

                if (res.isErr()) {
                    geode::log::error("{}", res.unwrapErr());
                    continue;
                }

                nongs.youtube().push_back(
                    std::make_unique<jukebox::YTSong>(res.unwrap()));
            }
        }

        if (value["hosted"].isArray()) {
            for (auto& hosted : value["hosted"].asArray().unwrap()) {
                geode::Result<jukebox::HostedSong> res =
                    matjson::Serialize<jukebox::HostedSong>::fromJson(hosted,
                                                                      songID);

                if (res.isErr()) {
                    geode::log::error("{}", res.unwrapErr());
                    continue;
                }

                nongs.hosted().push_back(
                    std::make_unique<jukebox::HostedSong>(res.unwrap()));
            }
        }

        if (!value["active"].isString()) {
            // This can't fail
            (void)nongs.setActive(nongs.defaultSong()->metadata()->uniqueID);
        } else {
            geode::Result<> res =
                nongs.setActive(value["active"].asString().unwrap());
            if (res.isErr()) {
                // Can't fail...
                (void)nongs.setActive(
                    nongs.defaultSong()->metadata()->uniqueID);
            }
        }

        return geode::Ok(std::move(nongs));
    }
};
