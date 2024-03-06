#include "nong_manager.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <system_error>
#include <utility>
#include <vector>
#include <string>

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

// std::vector<Nong> NongManager::validateNongs(int songID) {
//     auto result = this->getNongs(songID);
//     // Validate nong paths and delete those that don't exist anymore
//     std::vector<Nong> invalidSongs;
//     std::vector<Nong> validSongs;
//     if (!result.has_value()) {
//         return invalidSongs;
//     }
//     auto currentData = result.value();

//     for (auto &song : currentData.songs) {
//         if (!fs::exists(song.path) && currentData.defaultPath != song.path && song.songUrl == "local") {
//             invalidSongs.push_back(song);
//             if (song.path == currentData.active) {
//                 currentData.active = currentData.defaultPath;
//             }
//         } else {
//             validSongs.push_back(song);
//         }
//     }

//     if (invalidSongs.size() > 0) {
//         NongData newData = {
//             .active = currentData.active,
//             .defaultPath = currentData.defaultPath,
//             .songs = validSongs,
//         };

//         this->saveNongs(newData, songID);
//     }

//     return invalidSongs;
// }

int NongManager::getCurrentManifestVersion() {
    return m_state->m_manifestVersion;
}

int NongManager::getStoredIDCount() {
    return m_state->m_nongs.size();
}

void NongManager::save() {
    auto json = matjson::Serialize<ModState>::to_json(m_state.get());
    auto path = this->getJsonPath();
    std::ofstream output(path.c_str());
    output << json.dump(matjson::NO_INDENTATION);
    output.close();
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
    this->save();
}

void NongManager::deleteAll(int songID) {
    std::vector<Nong> newSongs;
    std::optional<SongData*> result = this->getNongs(songID);
    if (!result.has_value()) {
        return;
    }
    auto existingData = result.value();
    existingData->deleteAll();

    this->save();
}

void NongManager::deleteNong(Nong* song, int songID) {
    std::vector<Nong> newSongs;
    auto result = this->getNongs(songID);
    if(!result.has_value()) {
        return;
    }
    auto existingData = result.value();
    existingData->deleteOne(song);
    this->save();
}

void NongManager::createDefault(int songID) {
    if (m_state.m_nongs.contains(songID)) {
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
    this->save();
}

void NongManager::getMultiAssetSizes(std::string songs, std::string sfx, std::function<void(std::string)> callback) {
    fs::path resources = fs::path(CCFileUtils::get()->getWritablePath2().c_str()) / "Resources";
    fs::path songDir = fs::path(CCFileUtils::get()->getWritablePath().c_str());
    std::thread([this, songs, sfx, callback, resources, songDir]() {
        float sum = 0.f;
        std::istringstream stream(songs);
        std::string s;
        while (std::getline(stream, s, ',')) {
            int id = std::stoi(s);
            auto result = this->getActiveNong(id);
            if (!result.has_value()) {
                continue;
            }
            auto path = result.value().path;
            if (path.string().starts_with("songs/")) {
                path = resources / path;
            }
            if (fs::exists(path)) {
                sum += fs::file_size(path);
            }
        }
        stream = std::istringstream(sfx);
        while (std::getline(stream, s, ',')) {
            std::stringstream ss;
            ss << "s" << s << ".ogg";
            std::string filename = ss.str();
            auto localPath = resources / "sfx" / filename;
            if (fs::exists(localPath)) {
                sum += fs::file_size(localPath);
                continue;
            }
            auto path = songDir / filename;
            if (fs::exists(path)) {
                sum += fs::file_size(path);
            }
        }

        double toMegabytes = sum / 1024.f / 1024.f;
        std::stringstream ss;
        ss << std::setprecision(3) << toMegabytes << "MB";
        callback(ss.str());
    }).detach();
}

std::filesystem::path NongManager::getJsonPath() {
    auto savedir = std::filesystem::path(Mod::get()->getSaveDir().string());
    return savedir / "nong_data.json";
}

void NongManager::loadSongs() {
    auto path = this->getJsonPath();
    if (!std::filesystem::exists(path)) {
        this->setDefaultState();
        return;
    }
    std::ifstream input(path.string());
    std::stringstream buffer;
    buffer << input.rdbuf();
    input.close();

    std::string string = buffer.str();
    if (string.empty()) {
        this->setDefaultState();
        return;
    }
    std::string fixed;
    fixed.reserve(string.size());
    for (size_t i = 0; i < string.size(); i++) {
        if (string[i] != '\0') {
            fixed += string[i];
        }
    }

    std::string error;
    auto json = matjson::parse(std::string_view(fixed), error);
    if (!json.has_value()) {
        this->backupCurrentJSON();
        this->setDefaultState();
        Mod::get()->setSavedValue("failed-load", true);
        return;
    }
    m_state = matjson::Serialize<ModState>::from_json(json.value());
}

void NongManager::backupCurrentJSON() {
    auto savedir = fs::path(Mod::get()->getSaveDir());
    auto backups = savedir / "backups";
    if (!fs::exists(backups)) {
        std::error_code ec;
        bool result = fs::create_directory(backups, ec);
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
    fs::path backupPath = backups / formatted;
    std::error_code ec;
    fs::path currentJson = this->getJsonPath();
    bool result = fs::copy_file(currentJson, backupPath, ec);
    if (ec) {
        log::error("Couldn't create backup for nong_data.json, error category: {}, message: {}", ec.category().name(), ec.category().message(ec.value()));
    }
}

void NongManager::setDefaultState() {
    m_state.m_manifestVersion = nongd::getManifestVersion();
}

void NongManager::prepareCorrectDefault(int songID) {
    auto res = this->getNongs(songID);
    if (!res.has_value()) {
        return;
    }

    auto nongs = res.value();
    if (nongs.defaultValid) {
        return;
    }
    nongs.defaultValid = false;
    this->saveNongs(nongs, songID);
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
    nongs.defaultValid = false;

    this->saveNongs(nongs, songID);
}

void NongManager::fixDefault(SongInfoObject* obj) {
    int songID = obj->m_songID;
    auto nongs = this->getNongs(songID).value();
    auto defaultNong = this->getDefaultNong(songID).value();
    if (obj->m_songUrl.empty() && obj->m_artistName.empty()) {
        nongs.defaultValid = false;
        this->saveNongs(nongs, obj->m_songID);
        return;
    }
    for (auto& song : nongs.songs) {
        if (song.path != defaultNong.path) {
            continue;
        }

        song.songName = obj->m_songName;
        song.authorName = obj->m_artistName;
        song.songUrl = obj->m_songUrl;
    }
    nongs.defaultValid = true;
    this->saveNongs(nongs, obj->m_songID);
}

void NongManager::createDefaultCallback(SongInfoObject* obj) {
    if (auto nongs = NongManager::get()->getNongs(obj->m_songID)) {
        return;
    }
    fs::path songPath = fs::path(std::string(MusicDownloadManager::sharedState()->pathForSong(obj->m_songID)));
    NongData data;
    Nong defaultSong;
    defaultSong.authorName = obj->m_artistName;
    defaultSong.songName = obj->m_songName;
    defaultSong.path = songPath;
    defaultSong.songUrl = obj->m_songUrl;
    data.active = songPath;
    data.defaultPath = songPath;
    data.songs.push_back(defaultSong);
    data.defaultValid = true;
    m_state.m_nongs[obj->m_songID] = data;
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
