#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <variant>

#include <Geode/Result.hpp>
#include <Geode/utils/general.hpp>
#include <arc/future/Future.hpp>
#include <matjson.hpp>

#include <jukebox/events/start_download.hpp>
#include <jukebox/nong/index.hpp>
#include <jukebox/nong/nong.hpp>

namespace jukebox {

class IndexManager {
protected:
    bool m_initialized = false;

    IndexManager() = default;

    std::unordered_map<int, std::vector<index::IndexSongMetadata*>> m_nongsForId {};
    std::unordered_map<std::string, std::tuple<int, std::string>> m_urlToIDs {};

    void onDownloadProgress(int gdSongID, const std::string& uniqueId, float progress);
    void onDownloadFinish(std::variant<index::IndexSongMetadata*, Song*>&& source, Nongs* destination,
                          geode::ByteVector&& data);
    arc::Future<geode::Result<matjson::Value>> fetchIndex(const index::IndexSource& index);
    arc::Future<> onIndexFetched(const std::string& url, matjson::Value&& json);

public:
    IndexManager(const IndexManager&) = delete;
    IndexManager(IndexManager&&) = delete;

    IndexManager& operator=(const IndexManager&) = delete;
    IndexManager& operator=(IndexManager&&) = delete;

    bool init();
    // index id -> index metadata
    std::unordered_map<std::string, std::unique_ptr<index::IndexMetadata>> m_loadedIndexes {};

    [[nodiscard]] bool initialized() const { return m_initialized; }

    arc::Future<geode::Result<>> fetchIndexes();

    geode::Result<> loadIndex(std::filesystem::path path);
    geode::Result<> loadIndex(matjson::Value&& jsonObj);

    geode::Result<std::vector<index::IndexSource>> getIndexes();

    std::optional<std::string> getIndexName(const std::string& indexID);
    void cacheIndexName(const std::string& indexId, const std::string& indexName);

    std::filesystem::path baseIndexesPath();

    geode::Result<> downloadSong(int gdSongID, std::string_view uniqueID);

    void registerIndexNongs(Nongs* destination);

    static IndexManager& get() {
        static IndexManager instance;
        return instance;
    }
};

}  // namespace jukebox
