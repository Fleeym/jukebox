#include "nong_manager.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

#include <matjson.hpp>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>

namespace nongd {

std::optional<SongData*> NongManager::getNongs(int songID) {
    if (!m_state->m_nongs.contains(songID)) {
        return std::nullopt;
    }

    auto actions = this->getSongIDActions(songID);
    if (actions.has_value()) {
        return std::nullopt;
    }

    return m_state->m_nongs[songID].get();
}

void NongManager::resolveSongInfoCallback(int id) {
    if (m_getSongInfoCallbacks.contains(id)) {
        m_getSongInfoCallbacks[id](id);
        m_getSongInfoCallbacks.erase(id);
    }
}

int NongManager::getCurrentManifestVersion() {
    return m_state->m_manifestVersion;
}

int NongManager::getStoredIDCount() {
    return m_state->m_nongs.size();
}

void NongManager::save(int songID) {
    auto json = matjson::Serialize<ModState>::to_json(m_state.get());
    auto path = this->getJsonPath(songID);
    std::ofstream output(path.c_str());
    output << json.dump(matjson::NO_INDENTATION);
    output.close();
}

void NongManager::saveAll() {
    for (auto const& kv : m_state->m_nongs) {
        this->save(kv.first);
    }
}

void NongManager::addNong(std::unique_ptr<Nong> song, int songID) {
    std::optional<SongData*> result = this->getNongs(songID);
    if (!result.has_value()) {
        return;
    }
    auto existingData = result.value();

    for (auto const& savedSong : existingData->getSongs()) {
        if (song->path == savedSong->path) {
            return;
        }
    }
    existingData->addSong(std::move(song));
    this->save(songID);
}

void NongManager::deleteAll(int songID) {
    std::vector<Nong> newSongs;
    std::optional<SongData*> result = this->getNongs(songID);
    if (!result.has_value()) {
        return;
    }
    auto existingData = result.value();
    existingData->deleteAll();

    this->save(songID);
}

void NongManager::deleteNong(Nong* song, int songID) {
    std::vector<Nong> newSongs;
    auto result = this->getNongs(songID);
    if(!result.has_value()) {
        return;
    }
    auto existingData = result.value();
    existingData->deleteOne(song);
    this->save(songID);
}

void NongManager::createDefault(int songID) {
    if (m_state->m_nongs.contains(songID)) {
        return;
    }
    SongInfoObject* songInfo = MusicDownloadManager::sharedState()->getSongInfoObject(songID);
    if (songInfo == nullptr && !m_getSongInfoCallbacks.contains(songID)) {
        MusicDownloadManager::sharedState()->getSongInfo(songID, true);
        this->addSongIDAction(songID, SongInfoGetAction::CreateDefault);
        return;
    }
    if (songInfo == nullptr) {
        return;
    }

    this->createDefaultCallback(songInfo);
}

void NongManager::createUnknownDefault(int songID) {
    if (this->getNongs(songID).has_value()) {
        return;
    }
    auto songPath = std::filesystem::path(
        std::string(MusicDownloadManager::sharedState()->pathForSong(songID))
    );

    std::unique_ptr<Nong> defaultSong = std::make_unique<Nong>(Nong {
        .path = std::move(songPath),
        .songName = "Unknown",
        .authorName = "",
        .songUrl = ""
    });

    Nong* ptr = defaultSong.get();
    std::vector<std::unique_ptr<Nong>> vec;
    vec.push_back(std::move(defaultSong));

    m_state->m_nongs.insert(std::make_pair(songID, std::make_unique<SongData>(SongData {
        std::move(vec),
        ptr,
        ptr,
        false
    })));
}

void NongManager::getMultiAssetSizes(std::string songs, std::string sfx, std::function<void(std::string)> callback) {
    auto resources = std::filesystem::path(CCFileUtils::get()->getWritablePath2().c_str()) / "Resources";
    auto songDir = std::filesystem::path(CCFileUtils::get()->getWritablePath().c_str());
    std::thread([this, songs, sfx, callback, resources, songDir]() {
        float sum = 0.f;
        std::istringstream stream(songs);
        std::string s;
        const std::lock_guard<std::mutex> lock(m_state_mutex);
        while (std::getline(stream, s, ',')) {
            int id = std::stoi(s);
            // SLAP that mutex
            std::optional<SongData*> result = this->getNongs(id);
            if (!result.has_value()) {
                continue;
            }
            const Nong* active = result.value()->getActive();
            std::filesystem::path path = active->path;
            if (path.string().starts_with("songs/")) {
                path = resources / path;
            }
            if (std::filesystem::exists(path)) {
                sum += std::filesystem::file_size(path);
            }
        }
        stream = std::istringstream(sfx);
        while (std::getline(stream, s, ',')) {
            std::stringstream ss;
            ss << "s" << s << ".ogg";
            std::string filename = ss.str();
            auto localPath = resources / "sfx" / filename;
            if (std::filesystem::exists(localPath)) {
                sum += std::filesystem::file_size(localPath);
                continue;
            }
            auto path = songDir / filename;
            if (std::filesystem::exists(path)) {
                sum += std::filesystem::file_size(path);
            }
        }

        double toMegabytes = sum / 1024.f / 1024.f;
        std::stringstream ss;
        ss << std::setprecision(3) << toMegabytes << "MB";
        callback(ss.str());
    }).detach();
}

std::filesystem::path NongManager::getJsonPath(int songID) {
    auto savedir = std::filesystem::path(Mod::get()->getSaveDir().string()) / "nong_data";
    if (!std::filesystem::exists(savedir)) {
        std::filesystem::create_directory(savedir);
    }

    return savedir / fmt::format("{}.json", songID);
}

void NongManager::loadSongs() {
    std::filesystem::path nongDataDir = std::filesystem::path(Mod::get()->getSaveDir() / "nong_data");
    if (!std::filesystem::exists(nongDataDir)) {
        this->setDefaultState();
        return;
    }

    std::unordered_map<int, std::unique_ptr<SongData>> songs;

    for (std::filesystem::directory_entry const& entry 
        : std::filesystem::directory_iterator(nongDataDir)) {
        if (entry.is_directory()) {
            continue;
        }
        if (std::optional<std::unique_ptr<SongData>> res = this->loadFromPath(entry.path())) {
            songs.insert(
                std::make_pair(
                    std::stoi(entry.path().filename()),
                    std::move(res.value())
                )
            );
        } else {
            // TODO add error messages for failed loads
            this->backupBadJson(entry.path());
        }
    }

    m_state = std::make_unique<ModState>(ModState {
        nongd::getManifestVersion(),
        std::move(songs)
    });
}

std::optional<std::unique_ptr<SongData>> NongManager::loadFromPath(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }
    
    std::ifstream input(path.string());
    std::stringstream buffer;
    buffer << input.rdbuf();
    input.close();

    std::string string = buffer.str();
    std::string fixed;
    fixed.reserve(string.size());
    for (size_t i = 0; i < string.size(); i++) {
        if (string[i] != '\0') {
            fixed += string[i];
        }
    }

    std::string error;
    std::optional<matjson::Value> json = matjson::parse(
        std::string_view(std::move(string)),
        error
    );
    if (!json) {
        // TODO backup as failed
        return std::nullopt;
    }

    return matjson::Serialize<SongData>::from_json(std::move(json.value()));
}

void NongManager::backupBadJson(std::filesystem::path const& jsonPath) {
    auto savedir = std::filesystem::path(Mod::get()->getSaveDir());
    auto backups = savedir / "backups" / jsonPath.filename();

    if (!std::filesystem::exists(backups)) {
        std::error_code ec;
        bool result = std::filesystem::create_directories(backups, ec);
        if (ec) {
            log::error("Couldn't create backups directory, error category: {}, message: {}", ec.category().name(), ec.category().message(ec.value()));
            return;
        }
        if (!result) {
            log::error("Couldn't create backups directory");
            return;
        }
    }

    auto now = std::chrono::system_clock::now();
    std::string formatted = fmt::format("backup-{:%d-%m-%Y %H-%M-%OS}.json", now);
    std::filesystem::path backupPath = backups / formatted;
    std::error_code ec;
    bool result = std::filesystem::copy_file(jsonPath, backupPath, ec);
    if (ec) {
        log::error(
            "Couldn't create backup for {}, error category: {}, message: {}", 
            jsonPath.c_str(),
            ec.category().name(), 
            ec.category().message(ec.value())
        );
    }
}

void NongManager::prepareCorrectDefault(int songID) {
    auto res = this->getNongs(songID);
    if (!res.has_value()) {
        return;
    }

    auto nongs = res.value();
    if (nongs->isDefaultValid()) {
        return;
    }
    nongs->setDefaultValid(false);
    MusicDownloadManager::sharedState()->clearSong(songID);
    MusicDownloadManager::sharedState()->getSongInfo(songID, true);
    this->addSongIDAction(songID, SongInfoGetAction::FixDefault);
}

void NongManager::markAsInvalidDefault(int songID) {
    auto res = this->getNongs(songID);
    if (!res.has_value()) {
        return;
    }

    auto nongs = res.value();
    nongs->setDefaultValid(false);
}

void NongManager::fixDefault(SongInfoObject* obj) {
    int songID = obj->m_songID;
    std::optional<SongData*> res = this->getNongs(songID);
    if (!res) {
        return;
    }
    SongData* nongs = res.value();
    Nong* defaultNong = nongs->getDefault();
    if (obj->m_songUrl.empty() && obj->m_artistName.empty()) {
        nongs->setDefaultValid(false);
        return;
    }
    defaultNong->songName = obj->m_songName;
    defaultNong->authorName = obj->m_artistName;
    defaultNong->songUrl = obj->m_songUrl;
    nongs->setDefaultValid(true);
    this->save(songID);
}

void NongManager::createDefaultCallback(SongInfoObject* obj) {
    if (auto nongs = NongManager::get()->getNongs(obj->m_songID)) {
        return;
    }
    std::filesystem::path songPath = std::filesystem::path(MusicDownloadManager::sharedState()->pathForSong(obj->m_songID).c_str());
    std::unique_ptr<Nong> defaultSong = std::make_unique<Nong>(Nong {
        .path = songPath,
        .songName = obj->m_songName,
        .authorName = obj->m_artistName,
        .levelName = ""
    });

    Nong* ptr = defaultSong.get();
    std::vector<std::unique_ptr<Nong>> vec;
    vec.push_back(std::move(defaultSong));

    std::unique_ptr<SongData> data = std::make_unique<SongData>(SongData {
        std::move(vec),
        ptr,
        ptr,
        true
    });
    m_state->m_nongs.insert(std::make_pair(obj->m_songID, std::move(data)));
}

ListenerResult NongManager::onSongInfoFetched(GetSongInfoEvent* event) {
    SongInfoObject* obj = event->getObject();
    if (obj == nullptr) {
        return ListenerResult::Stop;
    }

    auto res = this->getSongIDActions(obj->m_songID);
    if (!res.has_value()) {
        return ListenerResult::Propagate;
    }

    auto actions = res.value();
    for (auto const& action : actions) {
        switch (action) {
            case SongInfoGetAction::CreateDefault: {
                this->createDefaultCallback(obj);
                break;
            }
            case SongInfoGetAction::FixDefault: {
                this->fixDefault(obj);
                break;
            }
        }
    }
    m_getSongInfoActions.erase(obj->m_songID);
    return ListenerResult::Propagate;
}

std::optional<std::vector<SongInfoGetAction>> NongManager::getSongIDActions(int songID) {
    if (m_getSongInfoActions.contains(songID)) {
        return std::nullopt;
    }

    if (m_getSongInfoActions[songID].empty()) {
        m_getSongInfoActions.erase(songID);
        return std::nullopt;
    }

    return m_getSongInfoActions[songID];
}

void NongManager::addSongIDAction(int songID, SongInfoGetAction action) {
    if (!m_getSongInfoActions.contains(songID)) {
        m_getSongInfoActions[songID] = { action };
        return;
    }

    for (auto const& existingAction : m_getSongInfoActions[songID]) {
        if (action == existingAction) {
            return;
        }
    }

    m_getSongInfoActions[songID].push_back(action);
}

}
