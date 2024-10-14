#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>

#include "Geode/loader/Event.hpp"
#include "Geode/utils/Task.hpp"

#include "index.hpp"
#include "managers/nong_manager.hpp"
#include "nong.hpp"

using namespace geode::prelude;

namespace jukebox {

using namespace jukebox::index;

class IndexManager {
    friend class NongManager;

protected:
    bool m_initialized = false;

    using FetchIndexTask = Task<Result<>, float>;
    using DownloadSongTask = Task<Result<std::filesystem::path>, float>;

    IndexManager() { this->init(); }

    bool init();
    // index url -> task listener
    std::unordered_map<std::string, EventListener<FetchIndexTask>>
        m_indexListeners;

    std::unordered_map<int, Nongs> m_indexNongs;
    std::unordered_map<int, std::vector<std::unique_ptr<IndexSongMetadata>>>
        m_nongsForId;
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

    Result<std::vector<Song*>> getNongs(int gdSongID);

    std::function<void(IndexManager::DownloadSongTask::Event*)>
    createDownloadSongBind(int gdSongID, Song* nong);
    Result<> downloadSong(int gdSongID, const std::string& uniqueID);
    Result<> downloadSong(Song* hosted);

    static IndexManager& get() {
        static IndexManager instance;
        return instance;
    }
};

}  // namespace jukebox
