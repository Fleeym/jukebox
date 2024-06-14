#include <Geode/binding/LevelTools.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/cocos/platform/CCFileUtils.h>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/Task.hpp>
#include <chrono>
#include <fmt/chrono.h>
#include <filesystem>
#include <fmt/core.h>
#include <optional>
#include <system_error>
#include <vector>
#include <string>

#include "nong_manager.hpp"

namespace jukebox {

std::optional<NongData> NongManager::getNongs(int songID) {
    if (!m_state.m_nongs.contains(songID)) {
        return std::nullopt;
    }

    return m_state.m_nongs[songID];
}

std::optional<SongInfo> NongManager::getActiveNong(int songID) {
    auto nongs_res = this->getNongs(songID);
    if (!nongs_res.has_value()) {
        return std::nullopt;
    }
    auto nongs = nongs_res.value();

    for (auto &song : nongs.songs) {
        if (song.path == nongs.active) {
            return song;
        }
    }
    
    nongs.active = nongs.defaultPath;

    for (auto &song : nongs.songs) {
        if (song.path == nongs.active) {
            return song;
        }
    }

    return std::nullopt;
}

bool NongManager::hasActions(int songID) {
    return m_getSongInfoActions.contains(songID);
}

bool NongManager::isFixingDefault(int songID) {
    if (!m_getSongInfoActions.contains(songID)) {
        return false;
    }

    for (auto action : m_getSongInfoActions[songID]) {
        if (action == SongInfoGetAction::FixDefault) {
            return true;
        }
    }

    return false;
}

std::optional<SongInfo> NongManager::getDefaultNong(int songID) {
    auto nongs_res = this->getNongs(songID);
    if (!nongs_res.has_value()) {
        return std::nullopt;
    }
    auto nongs = nongs_res.value();

    for (auto &song : nongs.songs) {
        if (song.path == nongs.defaultPath) {
            return song;
        }
    }
    
    return std::nullopt;
}

void NongManager::resolveSongInfoCallback(int id) {
    if (m_getSongInfoCallbacks.contains(id)) {
        m_getSongInfoCallbacks[id](id);
        m_getSongInfoCallbacks.erase(id);
    }
}

std::vector<SongInfo> NongManager::validateNongs(int songID) {
    auto result = this->getNongs(songID);
    // Validate nong paths and delete those that don't exist anymore
    std::vector<SongInfo> invalidSongs;
    std::vector<SongInfo> validSongs;
    if (!result.has_value()) {
        return invalidSongs;
    }
    auto currentData = result.value();

    for (auto &song : currentData.songs) {
        if (!fs::exists(song.path) && currentData.defaultPath != song.path && song.songUrl == "local") {
            invalidSongs.push_back(song);
            if (song.path == currentData.active) {
                currentData.active = currentData.defaultPath;
            }
        } else {
            validSongs.push_back(song);
        }
    }

    if (invalidSongs.size() > 0) {
        NongData newData = {
            .active = currentData.active,
            .defaultPath = currentData.defaultPath,
            .songs = validSongs,
        };

        this->saveNongs(newData, songID);
    }

    return invalidSongs;
}

int NongManager::getCurrentManifestVersion() {
    return m_state.m_manifestVersion;
}

int NongManager::getStoredIDCount() {
    return m_state.m_nongs.size();
}

void NongManager::saveNongs(NongData const& data, int songID) {
    m_state.m_nongs[songID] = data;
    this->writeJson();
}

void NongManager::writeJson() {
    auto json = matjson::Serialize<NongState>::to_json(m_state);
    auto path = this->getJsonPath();
    std::ofstream output(path.c_str());
    output << json.dump(matjson::NO_INDENTATION);
    output.close();
}

void NongManager::addNong(SongInfo const& song, int songID) {
    auto result = this->getNongs(songID);
    if (!result.has_value()) {
        return;
    }
    auto existingData = result.value();

    for (auto const& savedSong : existingData.songs) {
        if (song.path.string() == savedSong.path.string()) {
            return;
        }
    }
    existingData.songs.push_back(song);
    this->saveNongs(existingData, songID);
}

void NongManager::deleteAll(int songID) {
    std::vector<SongInfo> newSongs;
    auto result = this->getNongs(songID);
    if (!result.has_value()) {
        return;
    }
    auto existingData = result.value();

    for (auto savedSong : existingData.songs) {
        if (savedSong.path != existingData.defaultPath) {
            if (fs::exists(savedSong.path)) {
                std::error_code ec;
                fs::remove(savedSong.path, ec);
                if (ec) {
                    log::error("Couldn't delete nong. Category: {}, message: {}", ec.category().name(), ec.category().message(ec.value()));
                    return;
                }
            }
            continue;
        }
        newSongs.push_back(savedSong);
    }

    NongData newData = {
        .active = existingData.defaultPath,
        .defaultPath = existingData.defaultPath,
        .songs = newSongs,
    };
    this->saveNongs(newData, songID);
}

void NongManager::deleteNong(SongInfo const& song, int songID, bool deleteFile) {
    std::vector<SongInfo> newSongs;
    auto result = this->getNongs(songID);
    if(!result.has_value()) {
        return;
    }
    auto existingData = result.value();
    for (auto savedSong : existingData.songs) {
        if (savedSong.path == song.path) {
            if (song.path == existingData.active) {
                existingData.active = existingData.defaultPath;
            }
            if (deleteFile && existingData.defaultPath != song.path && fs::exists(song.path)) {
                std::error_code ec;
                fs::remove(song.path, ec);
                if (ec) {
                    log::error("Couldn't delete nong. Category: {}, message: {}", ec.category().name(), ec.category().message(ec.value()));
                    return;
                }
            }
            continue;
        }
        newSongs.push_back(savedSong);
    }
    NongData newData = {
        .active = existingData.active,
        .defaultPath = existingData.defaultPath,
        .songs = newSongs,
    };
    this->saveNongs(newData, songID);
}

void NongManager::createDefault(SongInfoObject* object, int songID, bool robtop) {
    if (m_state.m_nongs.contains(songID)) {
        return;
    }
    if (!robtop) {
        object = MusicDownloadManager::sharedState()->getSongInfoObject(songID);
    }
    if (!robtop && object == nullptr && !m_getSongInfoCallbacks.contains(songID)) {
        MusicDownloadManager::sharedState()->getSongInfo(songID, true);
        this->addSongIDAction(songID, SongInfoGetAction::CreateDefault);
        return;
    }
    if (object == nullptr && !robtop) {
        return;
    }

    this->createDefaultCallback(object, songID);
}

void NongManager::createUnknownDefault(int songID) {
    if (this->getNongs(songID).has_value()) {
        return;
    }
    fs::path songPath = fs::path(std::string(MusicDownloadManager::sharedState()->pathForSong(songID)));
    NongData data;
    SongInfo defaultSong;
    defaultSong.authorName = "";
    defaultSong.songName = "Unknown";
    defaultSong.path = songPath;
    defaultSong.songUrl = "";
    data.active = songPath;
    data.defaultPath = songPath;
    data.songs.push_back(defaultSong);
    data.defaultValid = false;
    m_state.m_nongs[songID] = data;
    this->writeJson();
}

std::string NongManager::getFormattedSize(SongInfo const& song) {
    std::error_code code;
    auto size = fs::file_size(song.path, code);
    if (code) {
        return "N/A";
    }
    double toMegabytes = size / 1024.f / 1024.f;
    std::stringstream ss;
    ss << std::setprecision(3) << toMegabytes << "MB";
    return ss.str();
}

NongManager::MultiAssetSizeTask NongManager::getMultiAssetSizes(std::string songs, std::string sfx) {
    fs::path resources = fs::path(CCFileUtils::get()->getWritablePath2().c_str()) / "Resources";
    fs::path songDir = fs::path(CCFileUtils::get()->getWritablePath().c_str());

    return MultiAssetSizeTask::run([this, songs, sfx, resources, songDir](auto progress, auto hasBeenCanceled) -> MultiAssetSizeTask::Result {
        float sum = 0.f;
        std::istringstream stream(songs);
        std::string s;
        while (std::getline(stream, s, ',')) {
            int id = std::stoi(s);
            auto result = this->getActiveNong(id);
            if (!result.has_value()) {
                continue;
            }
            auto path = result->path;
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
            std::error_code _ec;
            if (fs::exists(localPath, _ec)) {
                sum += fs::file_size(localPath);
                continue;
            }
            auto path = songDir / filename;
            if (fs::exists(path, _ec)) {
                sum += fs::file_size(path);
            }
        }

        double toMegabytes = sum / 1024.f / 1024.f;
        std::stringstream ss;
        ss << std::setprecision(3) << toMegabytes << "MB";
        return ss.str();
    }, fmt::format("Multiasset: {}|{}", songs, sfx));
}

fs::path NongManager::getJsonPath() {
    auto savedir = Mod::get()->getSaveDir();
    return savedir / "nong_data.json";
}

void NongManager::loadSongs() {
    if (m_initialized) {
        return;
    }
    auto path = this->getJsonPath();
    if (!fs::exists(path)) {
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
    m_state = matjson::Serialize<NongState>::from_json(json.value());
    m_initialized = true;
}

void NongManager::backupCurrentJSON() {
    auto savedir = Mod::get()->getSaveDir();
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
    m_state.m_manifestVersion = jukebox::getManifestVersion();
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

void NongManager::createDefaultCallback(SongInfoObject* obj, int songID) {
    int id = obj->m_songID;
    if (songID != 0) {
        id = songID;
    }
    if (auto nongs = NongManager::get()->getNongs(songID)) {
        return;
    }

    fs::path songPath;
    if (id < 0) {
        gd::string filename = LevelTools::getAudioFileName((-id) - 1);
        fs::path gdDir = fs::path(CCFileUtils::sharedFileUtils()->getWritablePath2().c_str());
        songPath = gdDir / "Resources" / filename.c_str();
    } else {
        songPath = fs::path(MusicDownloadManager::sharedState()->pathForSong(obj->m_songID).c_str());
    }

    NongData data;
    SongInfo defaultSong;
    defaultSong.authorName = obj->m_artistName;
    defaultSong.songName = obj->m_songName;
    defaultSong.path = songPath;
    defaultSong.songUrl = obj->m_songUrl;
    data.active = songPath;
    data.defaultPath = songPath;
    data.songs.push_back(defaultSong);
    data.defaultValid = true;
    m_state.m_nongs[id] = data;
}

ListenerResult NongManager::onSongInfoFetched(GetSongInfoEvent* event) {
    SongInfoObject* obj = event->getObject();
    int id = event->getID();

    auto res = this->getSongIDActions(id);
    if (!res.has_value()) {
        return ListenerResult::Propagate;
    }

    auto actions = res.value();
    if (actions.contains(SongInfoGetAction::CreateDefault)) {
        if (obj == nullptr) {
            this->createUnknownDefault(id);
        } else {
            this->createDefaultCallback(obj);
        }
    }
    if (actions.contains(SongInfoGetAction::FixDefault)) {
        if (obj != nullptr) {
            this->fixDefault(obj);
        }
    }
    m_getSongInfoActions.erase(id);
    return ListenerResult::Propagate;
}

std::optional<std::unordered_set<SongInfoGetAction>> NongManager::getSongIDActions(int songID) {
    if (!m_getSongInfoActions.contains(songID)) {
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

    if (m_getSongInfoActions[songID].contains(action)) {
        return;
    }

    m_getSongInfoActions[songID].insert(action);
}

}
