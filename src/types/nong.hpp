#pragma once

#include <sstream>
#include <string>
#include <filesystem>
#include <system_error>

#include <matjson.hpp>

namespace nongd {

struct Nong {
    std::filesystem::path path;
    std::string songName;
    std::string authorName;
    std::string songUrl;
    std::string levelName;

    std::string formattedSize() {
        std::error_code code;
        auto size = std::filesystem::file_size(this->path, code);
        if (code) {
            return "N/A";
        }
        double toMegabytes = size / 1024.f / 1024.f;
        std::stringstream ss;
        ss << std::setprecision(3) << toMegabytes << "MB";
        return ss.str();
    }
};

}
