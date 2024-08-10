#pragma once

#include "index.hpp"

template<>
struct matjson::Serialize<jukebox::IndexMetadata::Features::RequestParams> {
    static geode::Result<jukebox::IndexMetadata::Features::RequestParams> from_json(matjson::Value const& value, int manifest) {
        if (manifest != 1) return geode::Err("Using unsupported manifest version: " + std::to_string(manifest));

        if (!value.is_object()) {
            return geode::Err("Expected object in requestParams");
        }
        if (!value.contains("url") || !value["url"].is_string()) {
            return geode::Err("Expected url in requestParams");
        }
        return geode::Ok(jukebox::IndexMetadata::Features::RequestParams {
            .m_url = value["url"].as_string()
        });
    }
};

template<>
struct matjson::Serialize<jukebox::IndexMetadata::Features> {
    static geode::Result<jukebox::IndexMetadata::Features> from_json(matjson::Value const& value, int manifest) {
        if (manifest != 1) return geode::Err("Using unsupported manifest version: " + std::to_string(manifest));

        if (!value.is_object()) {
            return geode::Err("Expected object in features");
        }

        jukebox::IndexMetadata::Features features;

        const auto& featuresObj = value;

        if (featuresObj.contains("submit")) {
            if (!featuresObj["submit"].is_object()) {
                return geode::Err("Expected submit to be an object in features");
            }
            const auto& submitObj = featuresObj["submit"];

            features.m_submit = { };

            if (submitObj.contains("preSubmitMessage")) {
                if (!submitObj["preSubmitMessage"].is_string()) {
                    return geode::Err("Expected preSubmitMessage to be a string in submit");
                }
                features.m_submit.value().m_preSubmitMessage = submitObj["preSubmitMessage"].as_string();
            }

            if (submitObj.contains("requestParams")) {
                auto requestParams = matjson::Serialize<jukebox::IndexMetadata::Features::RequestParams>::from_json(submitObj["requestParams"], manifest);
                if (requestParams.isErr()) {
                    return geode::Err(requestParams.error());
                }
                features.m_submit.value().m_requestParams = requestParams.value();
            }
        }

        if (featuresObj.contains("report")) {
            if (!featuresObj["report"].is_object()) {
                return geode::Err("Expected search to be an object in features");
            }
            const auto& searchObj = featuresObj["report"];

            features.m_report = { };

            if (searchObj.contains("requestParams")) {
                auto requestParams = matjson::Serialize<jukebox::IndexMetadata::Features::RequestParams>::from_json(searchObj["requestParams"], manifest);
                if (requestParams.isErr()) {
                    return geode::Err(requestParams.error());
                }
                features.m_report.value().m_requestParams = requestParams.value();
            }
        }

        return geode::Ok(features);
    };
};

template<>
struct matjson::Serialize<jukebox::IndexMetadata> {
    static geode::Result<jukebox::IndexMetadata> from_json(matjson::Value const& value) {
        if (!value.is_object()) {
            return geode::Err("Expected object");
        }

        if (!value.contains("manifest") || !value["manifest"].is_number()) {
            return geode::Err("Expected manifest version");
        }
        const int manifestVersion = value["manifest"].as_int();

        if (manifestVersion == 1) {
            // Links
            jukebox::IndexMetadata::Links links;
            if (value.contains("links") && value["links"].is_object()) {
                const auto& linksObj = value["links"];
                if (linksObj.contains("discord")) {
                  if (!linksObj["discord"].is_string()) {
                    return geode::Err("Expected discord to be a string in links");
                  }
                }
                links.m_discord = linksObj["discord"].as_string();
            }

            // Features
            const auto featuresResult = value.contains("features")
                ? matjson::Serialize<jukebox::IndexMetadata::Features>::from_json(value, manifestVersion)
                : geode::Ok(jukebox::IndexMetadata::Features());
            if (!featuresResult) {
                return geode::Err(featuresResult.error());
            }

            // Validate fields
            if (!value.contains("name") || !value["name"].is_string()) {
                return geode::Err("Expected name");
            }
            if (!value.contains("id") || !value["id"].is_string()) {
                return geode::Err("Expected id");
            }
            if (value.contains("description") && !value["description"].is_string()) {
                return geode::Err("Description must be a string");
            }

            return geode::Ok(jukebox::IndexMetadata {
                .m_manifest = manifestVersion,
                .m_url = value["url"].as_string(),
                .m_id = value["id"].as_string(),
                .m_name = value["name"].as_string(),
                .m_description =  value.contains("description")
                    && value["description"].is_string()
                        ? std::optional(value["description"].as_string())
                        : std::nullopt,
                .m_lastUpdate = value.contains("lastUpdate") && value["lastUpdate"].is_number()
                    ? std::optional(value["lastUpdate"].as_int())
                    : std::nullopt,
                .m_links = links,
                .m_features = featuresResult.value()
            });
        }

        return geode::Err("Using unsupported manifest version: " + std::to_string(manifestVersion));
    }
};

template<>
struct matjson::Serialize<jukebox::IndexSource> {
    static jukebox::IndexSource from_json(matjson::Value const& value) {
        return jukebox::IndexSource {
            .m_url = value["url"].as_string(),
            .m_userAdded = value["userAdded"].as_bool(),
            .m_enabled = value["enabled"].as_bool()
        };
    }

    static matjson::Value to_json(jukebox::IndexSource const& value) {
        auto indexObject = matjson::Object();
        indexObject["url"] = value.m_url;
        indexObject["userAdded"] = value.m_userAdded;
        indexObject["enabled"] = value.m_enabled;
        return indexObject;
    }
};
