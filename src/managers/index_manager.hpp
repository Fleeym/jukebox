#pragma once

#include <filesystem>
#include <optional>

#include "Geode/utils/Task.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Event.hpp"
#include "Geode/loader/Mod.hpp"

#include "../../include/nong.hpp"
#include "../../include/index.hpp"

using namespace geode::prelude;

namespace jukebox {

class IndexManager : public CCObject {
protected:
    inline static IndexManager* m_instance = nullptr;
    bool m_initialized = false;

    using FetchIndexTask = Task<Result<>, float>;

    bool init();
    // index id -> index metadata
    std::unordered_map<std::string, std::unique_ptr<IndexMetadata>> m_loadedIndexes;
    // index url -> task listener
    std::unordered_map<std::string, EventListener<FetchIndexTask>> m_indexListeners;
public:
    bool initialized() const {
        return m_initialized;
    }

    Result<> fetchIndexes();

    Result<> loadIndex(std::filesystem::path path);

    // Result<> addIndex(IndexSource source);
    // Result<> removeIndex(const std::string& url);
    Result<std::vector<IndexSource>> getIndexes();

    Result<std::string> getIndexName(const std::string& indexId);
    void cacheIndexName(const std::string& indexId, const std::string& indexName);

    std::filesystem::path baseIndexesPath();

    static IndexManager* get() {
        if (m_instance == nullptr) {
            m_instance = new IndexManager();
            m_instance->retain();
            m_instance->init();
        }

        return m_instance;
    }
};

}
