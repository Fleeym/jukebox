#pragma once

#include <optional>

#include <matjson.hpp>
#include "Geode/utils/Result.hpp"

#include "index.hpp"

template <>
struct matjson::Serialize<
    jukebox::index::IndexMetadata::Features::RequestParams> {
    static geode::Result<jukebox::index::IndexMetadata::Features::RequestParams>
    from_json(matjson::Value const& value, int manifest) {
        if (manifest != 1) {
            return geode::Err("Using unsupported manifest version: " +
                              std::to_string(manifest));
        }

        if (!value.is_object()) {
            return geode::Err("Expected object in requestParams");
        }
        if (!value.contains("url") || !value["url"].is_string()) {
            return geode::Err("Expected url in requestParams");
        }
        if (!value.contains("params") || !value["params"].is_bool()) {
            return geode::Err("Expected params in requestParams");
        }

        return geode::Ok(jukebox::index::IndexMetadata::Features::RequestParams{
            .m_url = value["url"].as_string(),
            .m_params = value["params"].as_bool()});
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
                                 SupportedSongType::local);
        }
        if (str == "youtube") {
            return geode::Ok(jukebox::index::IndexMetadata::Features::
                                 SupportedSongType::youtube);
        }
        if (str == "hosted") {
            return geode::Ok(jukebox::index::IndexMetadata::Features::
                                 SupportedSongType::hosted);
        }
        return geode::Err("Invalid supported song type: " + str);
    }

public:
    static geode::Result<jukebox::index::IndexMetadata::Features> from_json(
        matjson::Value const& value, int manifest) {
        if (manifest != 1) {
            return geode::Err("Using unsupported manifest version: " +
                              std::to_string(manifest));
        }

        if (!value.is_object()) {
            return geode::Err("Expected object in features");
        }

        jukebox::index::IndexMetadata::Features features;

        const auto& featuresObj = value;

        if (featuresObj.contains("submit")) {
            if (!featuresObj["submit"].is_object()) {
                return geode::Err(
                    "Expected submit to be an object in features");
            }
            const auto& submitObj = featuresObj["submit"];

            features.m_submit =
                jukebox::index::IndexMetadata::Features::Submit{};

            if (submitObj.contains("preSubmitMessage")) {
                if (!submitObj["preSubmitMessage"].is_string()) {
                    return geode::Err(
                        "Expected preSubmitMessage to be a string in submit");
                }
                features.m_submit.value().m_preSubmitMessage =
                    submitObj["preSubmitMessage"].as_string();
            }

            if (submitObj.contains("supportedSongTypes")) {
                if (!submitObj["supportedSongTypes"].is_array()) {
                    return geode::Err(
                        "Expected supportedSongTypes to be an array in submit");
                }
                for (const auto& songType :
                     submitObj["supportedSongTypes"].as_array()) {
                    if (!songType.is_string()) {
                        return geode::Err(
                            "Expected supportedSongTypes to be an array of "
                            "strings in submit");
                    }
                    auto supportedSongType =
                        getSupportedSongTypeFromString(songType.as_string());
                    if (supportedSongType.isErr()) {
                        return geode::Err(supportedSongType.error());
                    }
                    features.m_submit.value()
                        .m_supportedSongTypes[supportedSongType.value()] = true;
                }
            }

            if (submitObj.contains("requestParams")) {
                auto requestParams = matjson::Serialize<
                    jukebox::index::IndexMetadata::Features::RequestParams>::
                    from_json(submitObj["requestParams"], manifest);
                if (requestParams.isErr()) {
                    return geode::Err(requestParams.error());
                }
                features.m_submit.value().m_requestParams =
                    requestParams.value();
            }
        }

        if (featuresObj.contains("report")) {
            if (!featuresObj["report"].is_object()) {
                return geode::Err(
                    "Expected search to be an object in features");
            }
            const auto& searchObj = featuresObj["report"];

            features.m_report =
                jukebox::index::IndexMetadata::Features::Report{};

            if (searchObj.contains("requestParams")) {
                auto requestParams = matjson::Serialize<
                    jukebox::index::IndexMetadata::Features::RequestParams>::
                    from_json(searchObj["requestParams"], manifest);
                if (requestParams.isErr()) {
                    return geode::Err(requestParams.error());
                }
                features.m_report.value().m_requestParams =
                    requestParams.value();
            }
        }

        return geode::Ok(features);
    };
};

template <>
struct matjson::Serialize<jukebox::index::IndexMetadata> {
    static geode::Result<jukebox::index::IndexMetadata> from_json(
        matjson::Value const& value) {
        if (!value.is_object()) {
            return geode::Err("Expected object");
        }

        if (!value.contains("manifest") || !value["manifest"].is_number()) {
            return geode::Err("Expected manifest version");
        }
        const int manifestVersion = value["manifest"].as_int();

        if (manifestVersion == 1) {
            // Links
            jukebox::index::IndexMetadata::Links links;
            if (value.contains("links") && value["links"].is_object()) {
                const auto& linksObj = value["links"];
                if (linksObj.contains("discord")) {
                    if (!linksObj["discord"].is_string()) {
                        return geode::Err(
                            "Expected discord to be a string in links");
                    }
                }
                links.m_discord = linksObj["discord"].as_string();
            }

            // Features
            const auto featuresResult =
                value.contains("features")
                    ? matjson::Serialize<
                          jukebox::index::IndexMetadata::Features>::
                          from_json(value["features"], manifestVersion)
                    : geode::Ok(jukebox::index::IndexMetadata::Features());
            if (featuresResult.has_error()) {
                return geode::Err(featuresResult.error());
            }

            // Validate fields
            if (!value.contains("name") || !value["name"].is_string()) {
                return geode::Err("Expected name");
            }
            if (!value.contains("id") || !value["id"].is_string()) {
                return geode::Err("Expected id");
            }
            if (value.contains("description") &&
                !value["description"].is_string()) {
                return geode::Err("Description must be a string");
            }

            return geode::Ok(jukebox::index::IndexMetadata{
                .m_manifest = manifestVersion,
                .m_url = value["url"].as_string(),
                .m_id = value["id"].as_string(),
                .m_name = value["name"].as_string(),
                .m_description =
                    value.contains("description") &&
                            value["description"].is_string()
                        ? std::optional(value["description"].as_string())
                        : std::nullopt,
                .m_lastUpdate =
                    value.contains("lastUpdate") &&
                            value["lastUpdate"].is_number()
                        ? std::optional(value["lastUpdate"].as_int())
                        : std::nullopt,
                .m_links = links,
                .m_features = featuresResult.value()});
        }

        return geode::Err("Using unsupported manifest version: " +
                          std::to_string(manifestVersion));
    }
};

template <>
struct matjson::Serialize<jukebox::index::IndexSongMetadata> {
    static geode::Result<jukebox::index::IndexSongMetadata> from_json(
        const matjson::Value& value) {
        if (!value.contains("name")) {
            return geode::Err("Song is missing \"name\" key");
        }

        if (!value.contains("artist")) {
            return geode::Err("Song is missing \"artist\" key");
        }

        if (!value.contains("startOffset") ||
            !value["startOffset"].is_number()) {
            return geode::Err("Song is missing \"startOffset\" key");
        }

        if (!value.contains("songs") || !value["songs"].is_array()) {
            return geode::Err("Song is missing \"songs\" key");
        }

        const std::vector<matjson::Value>& jsonSongs =
            value["songs"].as_array();
        std::vector<int> songs;
        songs.reserve(jsonSongs.size());
        for (const matjson::Value& i : jsonSongs) {
            songs.push_back(i.as_int());
        }

        return geode::Ok(jukebox::index::IndexSongMetadata{
            .uniqueID = "",
            .name = value["name"].as_string(),
            .artist = value["artist"].as_string(),
            .url = value.contains("url")
                       ? std::optional(value["url"].as_string())
                       : std::nullopt,
            .ytId = value.contains("ytId")
                        ? std::optional(value["ytId"].as_string())
                        : std::nullopt,
            .songIDs = std::move(songs),
            .startOffset = value["startOffset"].as_int(),
            .parentID = nullptr});
    }
};

template <>
struct matjson::Serialize<jukebox::index::IndexSource> {
    static jukebox::index::IndexSource from_json(matjson::Value const& value) {
        return jukebox::index::IndexSource{
            .m_url = value["url"].as_string(),
            .m_userAdded = value["userAdded"].as_bool(),
            .m_enabled = value["enabled"].as_bool()};
    }

    static matjson::Value to_json(jukebox::index::IndexSource const& value) {
        auto indexObject = matjson::Object();
        indexObject["url"] = value.m_url;
        indexObject["userAdded"] = value.m_userAdded;
        indexObject["enabled"] = value.m_enabled;
        return indexObject;
    }
};
