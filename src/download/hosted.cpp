#include "download/hosted.hpp"

#include <fmt/core.h>
#include "Geode/utils/Result.hpp"
#include "Geode/utils/web.hpp"

#include "download/download.hpp"

using namespace geode::prelude;

namespace jukebox {

namespace download {

DownloadTask startHostedDownload(const std::string& url) {
    return web::WebRequest()
        .timeout(std::chrono::seconds(30))
        .get(url)
        .map(
            [](web::WebResponse* response) -> DownloadTask::Value {
                if (!response->ok()) {
                    return Err(fmt::format("Web request failed. Status {}",
                                           response->code()));
                }
                return Ok(std::move(response->data()));
            },
            [](web::WebProgress* progress) {
                log::info("Progress: {}", progress->downloadProgress().value_or(0));
                log::info("total: {}", progress->downloadTotal());
                log::info("downloaded: {}", progress->downloaded());
                return progress->downloadProgress().value_or(0);
            });
}

}  // namespace download

}  // namespace jukebox
