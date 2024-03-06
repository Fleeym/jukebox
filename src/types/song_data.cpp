#pragma once

#include "song_data.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <system_error>

#include <Geode/loader/Log.hpp>

namespace nongd {

void SongData::addSong(std::unique_ptr<Nong> song) {
    m_songs.push_back(std::move(song));
}

void SongData::deleteOne(Nong* song) {
    m_songs.erase(std::remove_if(
        m_songs.begin(),
        m_songs.end(),
        [this, song](const std::unique_ptr<Nong>& stored) {
            if (stored.get() != song) {
                return false;
            }

            this->deleteFileForNong(song);
            return true;
        }
    ), m_songs.end());
}

void SongData::deleteAll() {
    m_songs.erase(std::remove_if(
        m_songs.begin(),
        m_songs.end(),
        [this](const std::unique_ptr<Nong>& stored) {
            if (stored.get() == m_default) {
                return false;
            }

            this->deleteFileForNong(stored.get());
            return true;
        }
    ), m_songs.end());
}

void SongData::deleteFileForNong(Nong* nong) {
    if (std::filesystem::exists(nong->path)) {
        std::error_code ec;
        std::filesystem::remove(nong->path, ec);
        if (ec) {
            geode::log::error(
                "Couldn't delete nong. Category: {}, message: {}",
                ec.category().name(), ec.category().message(ec.value())
            );
        }
    }
}

}