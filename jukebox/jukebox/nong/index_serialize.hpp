#pragma once

#include <cstdint>
#include <optional>

#include <Geode/Result.hpp>
#include <matjson.hpp>

#include <jukebox/nong/index.hpp>

template <>
struct matjson::Serialize<
    jukebox::index::IndexMetadata::Features::RequestParams> {
    static geode::Result<jukebox::index::IndexMetadata::Features::RequestParams>
    fromJson(matjson::Value const& value) {
        GEODE_UNWRAP_INTO(std::string url, value["url"].asString());
        GEODE_UNWRAP_INTO(bool params, value["params"].asBool());

        return geode::Ok(jukebox::index::IndexMetadata::Features::RequestParams{
            .m_url = url, .m_params = params});
    }
};

template <>
struct matjson::Serialize<jukebox::index::IndexMetadata::Features> {
protected:
    static geode::Result<
        jukebox::index::IndexMetadata::Features::SupportedSongType>
    getSupportedSongTypeFromString(const std::string& str) {
        if (str == "local") {
            return geode::Ok(jukebox::index::IndexMetadata::Features::
                                 SupportedSongType::LOCAL);
        }
        if (str == "youtube") {
            return geode::Ok(jukebox::index::IndexMetadata::Features::
                                 SupportedSongType::YOUTUBE);
        }
        if (str == "hosted") {
            return geode::Ok(jukebox::index::IndexMetadata::Features::
                                 SupportedSongType::HOSTED);
        }
        return geode::Err("Invalid supported song type: " + str);
    }

public:
    static geode::Result<jukebox::index::IndexMetadata::Features> fromJson(
        matjson::Value const& value) {
        jukebox::index::IndexMetadata::Features features;

        if (value.contains("submit")) {
            const auto& submitObj = value["submit"];

            jukebox::index::IndexMetadata::Features::Submit submit;

            if (submitObj.contains("preSubmitMessage")) {
                GEODE_UNWRAP_INTO(submit.m_preSubmitMessage,
                                  submitObj["preSubmitMessage"].asString());
            }

            if (submitObj.contains("supportedSongTypes")) {
                for (const auto& songType : submitObj["supportedSongTypes"]) {
                    GEODE_UNWRAP_INTO(std::string str, songType.asString());
                    GEODE_UNWRAP_INTO(auto supportedSongType,
                                      getSupportedSongTypeFromString(str));
                    submit.m_supportedSongTypes[supportedSongType] = true;
                }
            }

            if (submitObj.contains("requestParams")) {
                GEODE_UNWRAP_INTO(submit.m_requestParams,
                                  submitObj["requestParams"]
                                      .as<jukebox::index::IndexMetadata::
                                              Features::RequestParams>());
            }

            features.m_submit = std::move(submit);
        }

        if (value.contains("report")) {
            const matjson::Value& searchObj = value["report"];

            jukebox::index::IndexMetadata::Features::Report report;

            if (searchObj.contains("requestParams")) {
                GEODE_UNWRAP_INTO(report.m_requestParams,
                                  searchObj["requestParams"]
                                      .as<jukebox::index::IndexMetadata::
                                              Features::RequestParams>());
            }

            features.m_report = std::move(report);
        }

        return geode::Ok(std::move(features));
    };
};

template <>
struct matjson::Serialize<jukebox::index::IndexMetadata> {
    static geode::Result<jukebox::index::IndexMetadata> fromJson(
        matjson::Value const& value) {
        GEODE_UNWRAP_INTO(int manifestVersion, value["manifest"].asInt());
        if (manifestVersion != 1) {
            return geode::Err(
                fmt::format("Invalid manifest version: {}", manifestVersion));
        }

        // Links
        jukebox::index::IndexMetadata::Links links;
        if (value.contains("links")) {
            if (value["links"].contains("discord")) {
                GEODE_UNWRAP_INTO(links.m_discord,
                                  value["links"]["discord"].asString());
            }
        }

        // Features
        jukebox::index::IndexMetadata::Features features;
        if (value.contains("features")) {
            GEODE_UNWRAP_INTO(
                features, value["features"]
                              .as<jukebox::index::IndexMetadata::Features>());
        }

        // Validate fields
        GEODE_UNWRAP_INTO(std::string name, value["name"].asString());
        GEODE_UNWRAP_INTO(std::string id, value["id"].asString());
        GEODE_UNWRAP_INTO(std::string url, value["url"].asString());
        GEODE_UNWRAP_INTO(std::string description,
                          value["description"].asString());

        return geode::Ok(jukebox::index::IndexMetadata{
            .m_manifest = manifestVersion,
            .m_url = std::move(url),
            .m_id = std::move(id),
            .m_name = std::move(name),
            .m_description = value["description"]
                                 .asString()
                                 .map([](auto i) { return std::optional(i); })
                                 .unwrapOr(std::nullopt),
            .m_lastUpdate = value["lastUpdate"]
                                .asInt()
                                .map([](auto i) { return std::optional(i); })
                                .unwrapOr(std::nullopt),
            .m_links = std::move(links),
            .m_features = std::move(features)});
    }
};

template <>
struct matjson::Serialize<jukebox::index::IndexSongMetadata> {
    static geode::Result<jukebox::index::IndexSongMetadata> fromJson(
        const matjson::Value& value) {
        GEODE_UNWRAP_INTO(std::string name, value["name"].asString());
        GEODE_UNWRAP_INTO(std::string artist, value["artist"].asString());

        if (value.contains("verifiedLevelIDs") &&
            !value["verifiedLevelIDs"].isArray()) {
            return geode::Err("Invalid \"verifiedLevelIDs\" key");
        }

        std::vector<int> songs;
        songs.reserve(value["songs"].size());
        for (const matjson::Value& i : value["songs"]) {
            geode::Result<std::intmax_t> id = i.asInt();
            if (!id) {
                continue;
            }
            songs.push_back(id.unwrap());
        }

        std::vector<int> verifiedLevelIDs;
        verifiedLevelIDs.reserve(value["verifiedLevelIDs"].size());
        for (const matjson::Value& i : value["verifiedLevelIDs"]) {
            geode::Result<std::intmax_t> res = i.asInt();
            if (!res) {
                continue;
            }
            verifiedLevelIDs.push_back(res.unwrap());
        }

        return geode::Ok(jukebox::index::IndexSongMetadata{
            .uniqueID = "",
            .name = name,
            .artist = artist,
            .url = value["url"]
                       .asString()
                       .map([](auto i) { return std::optional(i); })
                       .unwrapOr(std::nullopt),
            .ytId = value["ytID"]
                        .asString()
                        .map([](auto i) { return std::optional(i); })
                        .unwrapOr(std::nullopt),
            .songIDs = std::move(songs),
            .verifiedLevelIDs = std::move(verifiedLevelIDs),
            .startOffset =
                static_cast<int>(value["startOffset"].asInt().unwrapOr(0)),
            .parentID = nullptr});
    }
};

template <>
struct matjson::Serialize<jukebox::index::IndexSource> {
    static geode::Result<jukebox::index::IndexSource> fromJson(
        matjson::Value const& value) {
        GEODE_UNWRAP_INTO(std::string url, value["url"].asString());
        GEODE_UNWRAP_INTO(bool userAdded, value["userAdded"].asBool());
        GEODE_UNWRAP_INTO(bool enabled, value["userAdded"].asBool());

        return geode::Ok(jukebox::index::IndexSource{.m_url = std::move(url),
                                                     .m_userAdded = userAdded,
                                                     .m_enabled = enabled});
    }

    static matjson::Value toJson(jukebox::index::IndexSource const& value) {
        return matjson::makeObject({{"url", value.m_url},
                                    {"userAdded", value.m_userAdded},
                                    {"enabled", value.m_enabled}});
    }
};
