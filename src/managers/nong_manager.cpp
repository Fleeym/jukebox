#include "nong_manager.hpp"
#include "Geode/binding/MusicDownloadManager.hpp"
#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/loader/Event.hpp"
#include <optional>
#include <vector>
#include <string>

std::optional<NongData> NongManager::getNongs(int songID) {
    if (!m_state.m_nongs.contains(songID)) {
        return std::nullopt;
    }

    auto actions = this->getSongIDActions(songID);
    if (actions.has_value()) {
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
    std::ofstream output(path.string());
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
                fs::remove(savedSong.path);
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

void NongManager::deleteNong(SongInfo const& song, int songID) {
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
            if (song.songUrl != "local" && existingData.defaultPath != song.path && fs::exists(song.path)) {
                fs::remove(song.path);
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
    try {
        auto size = fs::file_size(song.path);
        double toMegabytes = size / 1024.f / 1024.f;
        std::stringstream ss;
        ss << std::setprecision(3) << toMegabytes << "MB";
        return ss.str();
    } catch (fs::filesystem_error) {
        return "N/A";
    }
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

fs::path NongManager::getJsonPath() {
    auto savedir = fs::path(Mod::get()->getSaveDir().string());
    return savedir / "nong_data.json";
}

void NongManager::loadSongs() {
    auto path = this->getJsonPath();
    if (!fs::exists(path)) {
        return;
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

    auto json = matjson::parse(std::string_view(fixed));
    m_state = matjson::Serialize<NongState>::from_json(json);
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
    nongs.defaultValid = true;
    this->saveNongs(nongs, songID);
    MusicDownloadManager::sharedState()->clearSong(songID);
    MusicDownloadManager::sharedState()->getSongInfo(songID, true);
    this->addSongIDAction(songID, SongInfoGetAction::FixDefault);
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
    this->saveNongs(nongs, obj->m_songID);
}

void NongManager::createDefaultCallback(SongInfoObject* obj) {
    if (auto nongs = NongManager::get()->getNongs(obj->m_songID)) {
        return;
    }
    fs::path songPath = fs::path(std::string(MusicDownloadManager::sharedState()->pathForSong(obj->m_songID)));
    NongData data;
    SongInfo defaultSong;
    defaultSong.authorName = obj->m_artistName;
    defaultSong.songName = obj->m_songName;
    defaultSong.path = songPath;
    defaultSong.songUrl = obj->m_songUrl;
    data.active = songPath;
    data.defaultPath = songPath;
    data.songs.push_back(defaultSong);
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