#include <jukebox/utils/web.hpp>

#include <string>

#include <fmt/format.h>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/web.hpp>

namespace jukebox::utils::web {

std::string getErrorFromResponse(const geode::utils::web::WebResponse& res) {
    if (res.ok()) {
        return "";
    }

    // Code < 0 => cURL / internal Geode error - very bad!
    if (res.code() < 0) {
        if (!res.errorMessage().empty()) {
            return std::string(res.errorMessage());
        }

        return res.string().unwrapOr(fmt::format("Failed to send request: internal code {}", res.code()));
    }

    if (res.code() == 502 || res.code() == 503 || res.code() == 530) {
        return
            "Web request failed. Service is currently "
            "unavailable or under maintenance. Please try "
            "again later.";
    }

    if (res.badServer()) {
        return fmt::format("Web request failed because of a server error, status code {}", res.code());
    }

    return fmt::format("Web request failed with status code {}", res.code());
}

}
