#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "Geode/loader/Event.hpp"
#include "Geode/utils/Task.hpp"

#include "../../include/index.hpp"
#include "../../include/nong.hpp"
#include "nong_manager.hpp"

using namespace geode::prelude;

namespace jukebox {

class IndexManager : public CCObject {
    friend class NongManager;

protected:
    inline static IndexManager* m_instance = nullptr;
    bool m_initialized = false;

    using FetchIndexTask = Task<Result<>, float>;
    using DownloadSongTask = Task<Result<std::filesystem::path>, float>;

    bool init();
    // index url -> task listener
    std::unordered_map<std::string, EventListener<FetchIndexTask>>
        m_indexListeners;

    std::unordered_map<int, Nongs> m_indexNongs;
    // song id -> download song task
    std::unordered_map<std::string, EventListener<DownloadSongTask>>
        m_downloadSongListeners;
    // song id -> current download progress (used when opening NongDropdownLayer
    // while a song is being downloaded)
    std::unordered_map<std::string, float> m_downloadProgress;

public:
    // index id -> index metadata
    std::unordered_map<std::string, std::unique_ptr<IndexMetadata>>
        m_loadedIndexes;

    bool initialized() const { return m_initialized; }

    Result<> fetchIndexes();

    Result<> loadIndex(std::filesystem::path path);

    Result<std::vector<IndexSource>> getIndexes();

    std::optional<float> getSongDownloadProgress(const std::string& uniqueID);
    std::optional<std::string> getIndexName(const std::string& indexID);
    void cacheIndexName(const std::string& indexId,
                        const std::string& indexName);

    std::filesystem::path baseIndexesPath();

    Result<std::vector<Nong>> getNongs(int gdSongID);

    std::function<void(IndexManager::DownloadSongTask::Event*)>
    createDownloadSongBind(int gdSongID, Nong nong);
    Result<> downloadSong(int gdSongID, const std::string& uniqueID);
    Result<> downloadSong(Nong hosted);

    static IndexManager* get() {
        if (m_instance == nullptr) {
            m_instance = new IndexManager();
            m_instance->retain();
            m_instance->init();
        }

        return m_instance;
    }
};

}  // namespace jukebox
