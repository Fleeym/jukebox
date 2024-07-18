#include "nong_manager.hpp"
#include "index_manager.hpp"

#include <Geode/utils/web.hpp>
#include <matjson.hpp>

#include "../../include/nong.hpp"
#include "../../include/nong_serialize.hpp"
#include "../../include/index_serialize.hpp"
#include "../ui/indexes_setting.hpp"

namespace jukebox {

bool IndexManager::init() {
    if (m_initialized) {
        return true;
    }

    Mod::get()->addCustomSetting<IndexesSettingValue>(
        "indexes", Mod::get()
                       ->getSettingDefinition("indexes")
                       ->get<CustomSetting>()
                       ->json->get<std::vector<IndexSource>>("default"));

    auto path = this->baseIndexesPath();
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        return true;
    }

    if (auto err = fetchIndexes().error(); !err.empty()) {
        log::error("Failed to fetch indexes: {}", err);
        return false;
    }

    m_initialized = true;
    return true;
}

Result<std::vector<IndexSource>> IndexManager::getIndexes() {
    auto setting = Mod::get()->getSettingValue<IndexesSettingStruct>("indexes");
    log::info("Indexes: {}", setting.m_indexes.size());
    for (const auto index : setting.m_indexes) {
      log::info("Index({}): {}", index.m_enabled, index.m_url);
    }
    return Ok(setting.m_indexes);
}

std::filesystem::path IndexManager::baseIndexesPath() {
    static std::filesystem::path path = Mod::get()->getSaveDir() / "indexes-cache";
    return path;
}

Result<> IndexManager::loadIndex(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        return Err("Index file does not exist");
    }

    std::ifstream input(path);
    if (!input.is_open()) {
        return Err(fmt::format("Couldn't open file: {}", path.filename().string()));
    }

    std::string contents;
    input.seekg(0, std::ios::end);
    contents.resize(input.tellg());
    input.seekg(0, std::ios::beg);
    input.read(&contents[0], contents.size());
    input.close();

    std::string error;
    std::optional<matjson::Value> jsonObj = matjson::parse(contents, error);

    if (!jsonObj.has_value()) {
        return Err(error);
    }

    const auto indexRes = matjson::Serialize<IndexMetadata>::from_json(jsonObj.value());

    if (indexRes.isErr()) {
        return Err(indexRes.error());
    }

    const auto&& index = std::move(indexRes.value());

    cacheIndexName(index.m_id, index.m_name);

    NongManager::get()->deleteSongsByIndex(index.m_id);

    m_loadedIndexes.emplace(
        index.m_id,
        std::make_unique<IndexMetadata>(index)
    );
    log::info("Jukebox index \"{}\" ({}) Loaded", index.m_name, index.m_id);

    return Ok();
}

Result<> IndexManager::fetchIndexes() {
    const auto indexesRes = getIndexes();
    if (indexesRes.isErr()) {
        return Err(indexesRes.error());
    }
    const auto indexes = indexesRes.value();

    for (const auto index : indexes) {
        log::info("Fetching index {}", index.m_url);
        if (!index.m_enabled || index.m_url.size() < 3) continue;

        auto filepath = baseIndexesPath() / fmt::format("{}.json", "replace-later");

        FetchIndexTask task = web::WebRequest().timeout(std::chrono::seconds(30)).get(index.m_url).map(
            [this, filepath, index](web::WebResponse *response) -> FetchIndexTask::Value {
                if (response->ok() && response->string().isOk()) {
                    std::string error;
                    std::optional<matjson::Value> jsonObj = matjson::parse(response->string().value(), error);

                    if (!jsonObj.has_value()) {
                        return Err(error);
                    }

                    if (!jsonObj.value().is_object()) {
                        return Err("Index supposed to be an object");
                    }
                    jsonObj.value().set("url", index.m_url);
                    const auto indexRes = matjson::Serialize<IndexMetadata>::from_json(jsonObj.value());

                    if (indexRes.isErr()) {
                        return Err(indexRes.error());
                    }

                    std::ofstream output(filepath);
                    if (!output.is_open()) {
                        return Err(fmt::format("Couldn't open file: {}", filepath));
                    }
                    output << jsonObj.value().dump(matjson::NO_INDENTATION);
                    output.close();

                    return Ok();
                }
                return Err("Web request failed");
            },
            [](web::WebProgress *progress) -> FetchIndexTask::Progress {
                return progress->downloadProgress().value_or(0) / 100.f;
            }
        );

        auto listener = EventListener<FetchIndexTask>();
        listener.bind([this, index, filepath](FetchIndexTask::Event* event) {
            if (float *progress = event->getProgress()) {
                return;
            }

            m_indexListeners.erase(index.m_url);

            if (FetchIndexTask::Value *result = event->getValue()) {
                if (result->isErr()) {
                    log::error("Failed to fetch index: {}", result->error());
                } else {
                    log::info("Index fetched and cached: {}", index.m_url);
                }
            } else if (event->isCancelled()) {}

            if (auto err = loadIndex(filepath).error(); !err.empty()) {
                log::error("Failed to load index: {}", err);
            }
        });
        listener.setFilter(task);
        m_indexListeners.emplace(index.m_url, std::move(listener));
    }

    return Ok();
}

Result<std::string> IndexManager::getIndexName(const std::string& indexId) {
    return Ok("Index Name");
    // auto jsonObj = Mod::get()->getSavedValue<matjson::Value>("cached-index-names");
    // if (!jsonObj.contains(indexId)) return Err("Index not found");
    // return Ok(jsonObj[indexId].as_string());
}

void IndexManager::cacheIndexName(const std::string& indexId, const std::string& indexName) {
    // auto jsonObj = Mod::get()->getSavedValue<matjson::Value>("cached-index-names", {});
    // jsonObj.set(indexId, indexName);
    // Mod::get()->setSavedValue(indexId, jsonObj);
}

};
