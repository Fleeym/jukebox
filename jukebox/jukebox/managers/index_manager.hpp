#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <variant>

#include <Geode/Result.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/utils/general.hpp>
#include <matjson.hpp>

#include <jukebox/events/start_download.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox {

class IndexManager {
protected:
    bool m_initialized = false;

    using FetchIndexTask = geode::Task<geode::Result<>, float>;
    using DownloadSongTask =
        geode::Task<geode::Result<geode::ByteVector>, float>;

    IndexManager() = default;

    IndexManager(const IndexManager&) = delete;
    IndexManager(IndexManager&&) = delete;

    IndexManager& operator=(const IndexManager&) = delete;
    IndexManager& operator=(IndexManager&&) = delete;

    // index url -> task listener
    std::unordered_map<std::string, geode::EventListener<FetchIndexTask>>
        m_indexListeners;

    std::unordered_map<int, std::vector<index::IndexSongMetadata*>>
        m_nongsForId;
    // song id -> download song task
    std::unordered_map<std::string, geode::EventListener<DownloadSongTask>>
        m_downloadSongListeners;
    // song id -> current download progress (used when opening NongDropdownLayer
    // while a song is being downloaded)
    std::unordered_map<std::string, float> m_downloadProgress;

    geode::EventListener<geode::EventFilter<jukebox::event::StartDownload>>
        m_downloadSignalListener{this, &IndexManager::onDownloadStart};

    geode::ListenerResult onDownloadStart(jukebox::event::StartDownload* e);
    void onDownloadProgress(int gdSongID, const std::string& uniqueId,
                            float progress);
    void onDownloadFinish(
        std::variant<index::IndexSongMetadata*, Song*>&& source,
        Nongs* destination, geode::ByteVector&& data);
    geode::Task<geode::Result<matjson::Value>, float> fetchIndex(
        const index::IndexSource& index);
    void onIndexFetched(const std::string& url,
                        geode::Result<matjson::Value>* r);

public:
    bool init();
    // index id -> index metadata
    std::unordered_map<std::string, std::unique_ptr<index::IndexMetadata>>
        m_loadedIndexes;

    bool initialized() const { return m_initialized; }

    geode::Result<> fetchIndexes();

    geode::Result<> loadIndex(std::filesystem::path path);
    geode::Result<> loadIndex(matjson::Value&& jsonObj);

    geode::Result<std::vector<index::IndexSource>> getIndexes();

    std::optional<float> getSongDownloadProgress(const std::string& uniqueID);
    std::optional<std::string> getIndexName(const std::string& indexID);
    void cacheIndexName(const std::string& indexId,
                        const std::string& indexName);

    std::filesystem::path baseIndexesPath();

    geode::Result<> downloadSong(int gdSongID, const std::string& uniqueID);

    void registerIndexNongs(Nongs* destination);

    static IndexManager& get() {
        static IndexManager instance;
        return instance;
    }
};

}  // namespace jukebox
