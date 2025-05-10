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
    fromJson(matjson::Value const& value, int manifest) {
        if (manifest != 1) {
            return geode::Err("Using unsupported manifest version: " +
                              std::to_string(manifest));
        }

        if (!value.isObject()) {
            return geode::Err("Expected object in requestParams");
        }
        if (!value.contains("url") || !value["url"].isString()) {
            return geode::Err("Expected url in requestParams");
        }
        if (!value.contains("params") || !value["params"].isBool()) {
            return geode::Err("Expected params in requestParams");
        }

        return geode::Ok(jukebox::index::IndexMetadata::Features::RequestParams{
            .m_url = value["url"].asString().unwrap(),
            .m_params = value["params"].asBool().unwrap()});
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
        matjson::Value const& value, int manifest) {
        if (manifest != 1) {
            return geode::Err("Using unsupported manifest version: " +
                              std::to_string(manifest));
        }

        if (!value.isObject()) {
            return geode::Err("Expected object in features");
        }

        jukebox::index::IndexMetadata::Features features;

        const auto& featuresObj = value;

        if (featuresObj.contains("submit")) {
            if (!featuresObj["submit"].isObject()) {
                return geode::Err(
                    "Expected submit to be an object in features");
            }
            const auto& submitObj = featuresObj["submit"];

            features.m_submit =
                jukebox::index::IndexMetadata::Features::Submit{};

            if (submitObj.contains("preSubmitMessage")) {
                if (!submitObj["preSubmitMessage"].isString()) {
                    return geode::Err(
                        "Expected preSubmitMessage to be a string in submit");
                }
                features.m_submit.value().m_preSubmitMessage =
                    submitObj["preSubmitMessage"].asString().unwrap();
            }

            if (submitObj.contains("supportedSongTypes")) {
                if (!submitObj["supportedSongTypes"].isArray()) {
                    return geode::Err(
                        "Expected supportedSongTypes to be an array in submit");
                }
                for (const auto& songType :
                     submitObj["supportedSongTypes"].asArray().unwrap()) {
                    if (!songType.isString()) {
                        return geode::Err(
                            "Expected supportedSongTypes to be an array of "
                            "strings in submit");
                    }
                    geode::Result<jukebox::index::IndexMetadata::Features::
                                      SupportedSongType>
                        supportedSongType = getSupportedSongTypeFromString(
                            songType.asString().unwrap());
                    if (supportedSongType.isErr()) {
                        return geode::Err(supportedSongType.unwrapErr());
                    }
                    features.m_submit.value()
                        .m_supportedSongTypes[supportedSongType.unwrap()] =
                        true;
                }
            }

            if (submitObj.contains("requestParams")) {
                auto requestParams = matjson::Serialize<
                    jukebox::index::IndexMetadata::Features::RequestParams>::
                    fromJson(submitObj["requestParams"], manifest);
                if (requestParams.isErr()) {
                    return geode::Err(requestParams.unwrapErr());
                }
                features.m_submit.value().m_requestParams =
                    requestParams.unwrap();
            }
        }

        if (featuresObj.contains("report")) {
            if (!featuresObj["report"].isObject()) {
                return geode::Err(
                    "Expected search to be an object in features");
            }
            const matjson::Value& searchObj = featuresObj["report"];

            features.m_report =
                jukebox::index::IndexMetadata::Features::Report{};

            if (searchObj.contains("requestParams")) {
                auto requestParams = matjson::Serialize<
                    jukebox::index::IndexMetadata::Features::RequestParams>::
                    fromJson(searchObj["requestParams"], manifest);
                if (requestParams.isErr()) {
                    return geode::Err(requestParams.unwrapErr());
                }
                features.m_report.value().m_requestParams =
                    requestParams.unwrap();
            }
        }

        return geode::Ok(features);
    };
};

template <>
struct matjson::Serialize<jukebox::index::IndexMetadata> {
    static geode::Result<jukebox::index::IndexMetadata> fromJson(
        matjson::Value const& value) {
        if (!value.isObject()) {
            return geode::Err("Expected object");
        }

        if (!value.contains("manifest") || !value["manifest"].isNumber()) {
            return geode::Err("Expected manifest version");
        }
        const int manifestVersion = value["manifest"].asInt().unwrap();

        if (manifestVersion == 1) {
            // Links
            jukebox::index::IndexMetadata::Links links;
            if (value.contains("links") && value["links"].isObject()) {
                const auto& linksObj = value["links"];
                if (linksObj.contains("discord")) {
                    if (!linksObj["discord"].isString()) {
                        return geode::Err(
                            "Expected discord to be a string in links");
                    }
                }
                links.m_discord = linksObj["discord"].asString().unwrap();
            }

            // Features
            geode::Result<jukebox::index::IndexMetadata::Features>
                featuresResult =
                    value.contains("features")
                        ? matjson::Serialize<
                              jukebox::index::IndexMetadata::Features>::
                              fromJson(value["features"], manifestVersion)
                        : geode::Ok(jukebox::index::IndexMetadata::Features());
            if (featuresResult.isErr()) {
                return geode::Err(featuresResult.unwrapErr());
            }

            // Validate fields
            if (!value.contains("name") || !value["name"].isString()) {
                return geode::Err("Expected name");
            }
            if (!value.contains("id") || !value["id"].isString()) {
                return geode::Err("Expected id");
            }
            if (value.contains("description") &&
                !value["description"].isString()) {
                return geode::Err("Description must be a string");
            }

            return geode::Ok(jukebox::index::IndexMetadata{
                .m_manifest = manifestVersion,
                .m_url = value["url"].asString().unwrap(),
                .m_id = value["id"].asString().unwrap(),
                .m_name = value["name"].asString().unwrap(),
                .m_description =
                    value["description"]
                        .asString()
                        .map([](auto i) { return std::optional(i); })
                        .unwrapOr(std::nullopt),
                .m_lastUpdate =
                    value["lastUpdate"]
                        .asInt()
                        .map([](auto i) { return std::optional(i); })
                        .unwrapOr(std::nullopt),
                .m_links = links,
                .m_features = featuresResult.unwrap()});
        }

        return geode::Err("Using unsupported manifest version: " +
                          std::to_string(manifestVersion));
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
            geode::Result<int64_t> id = i.asInt();
            if (!id) {
                continue;
            }
            songs.push_back(id.unwrap());
        }

        std::vector<int> verifiedLevelIDs;
        verifiedLevelIDs.reserve(value["verifiedLevelIDs"].size());
        for (const matjson::Value& i : value["verifiedLevelIDs"]) {
            geode::Result<int64_t> res = i.asInt();
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
        if (!value["url"].isString() || !value["userAdded"].isBool() ||
            !value["enabled"].isBool()) {
            return geode::Err("Invalid JSON");
        }

        return geode::Ok(jukebox::index::IndexSource{
            .m_url = value["url"].asString().unwrap(),
            .m_userAdded = value["userAdded"].asBool().unwrap(),
            .m_enabled = value["enabled"].asBool().unwrap()});
    }

    static matjson::Value toJson(jukebox::index::IndexSource const& value) {
        return matjson::makeObject({{"url", value.m_url},
                                    {"userAdded", value.m_userAdded},
                                    {"enabled", value.m_enabled}});
    }
};
