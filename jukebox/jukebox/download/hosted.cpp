#include <jukebox/download/hosted.hpp>

#include <fmt/core.h>
#include <Geode/loader/Mod.hpp>
#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>

#include <jukebox/download/download.hpp>

using namespace geode::prelude;

namespace jukebox {

namespace download {

DownloadTask startHostedDownload(const std::string& url) {
    int timeout = Mod::get()->getSettingValue<int>("download-timeout");

    if (timeout < 30) {
        timeout = 30;
    }

    return web::WebRequest()
        .timeout(std::chrono::seconds(timeout))
        .get(url)
        .map(
            [](web::WebResponse* response) -> DownloadTask::Value {
                if (!response->ok()) {
                    if (response->code() == 502) {
                        return Err(
                            "Web request failed. Service is currently "
                            "unavailable or under maintenance. Please try "
                            "again later.");
                    } else {
                        return Err(fmt::format("Web request failed. Status {}",
                                               response->code()));
                    }
                }
                return Ok(std::move(response->data()));
            },
            [](web::WebProgress* progress) {
                return progress->downloadProgress().value_or(0);
            });
}

}  // namespace download

}  // namespace jukebox
