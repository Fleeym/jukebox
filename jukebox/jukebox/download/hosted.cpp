#include <jukebox/download/hosted.hpp>

#include <string_view>

#include <fmt/core.h>
#include <Geode/Result.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/web.hpp>

#include <jukebox/events/file_download_progress.hpp>

using namespace geode::prelude;

namespace jukebox::download {

arc::Future<Result<ByteVector>> startHostedDownload(const std::string_view url) {
    int timeout = Mod::get()->getSettingValue<int>("download-timeout");

    if (timeout < 30) {
        timeout = 30;
    }

    const web::WebResponse response =
        co_await web::WebRequest()
            .timeout(std::chrono::seconds(timeout))
            .onProgress([url = std::string(url)](const web::WebProgress& progress) {
                const float actual = progress.downloadProgress().value_or(0.0f);

                events::FileDownloadProgress(url).send(events::FileDownloadProgressData{url, actual});
            })
            .get(std::string(url));

    if (!response.ok()) {
        if (response.code() == 502) {
            co_return Err(
                "Web request failed. Service is currently "
                "unavailable or under maintenance. Please try "
                "again later.");
        }

        co_return Err(fmt::format("Web request failed. Status {}", response.code()));
    }

    co_return Ok(std::move(response).data());
}

}  // namespace jukebox::download
