#include <jukebox/download/youtube.hpp>

#include <matjson.hpp>
#include <string>

#include <Geode/loader/Mod.hpp>
#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>

#include <jukebox/download/hosted.hpp>

using namespace geode::prelude;
using namespace arc;

web::WebFuture getMetadata(const std::string& id);
Result<std::string> getUrlFromMetadataPayload(web::WebResponse r);
Future<Result<ByteVector>> onMetadata(web::WebResponse result);

namespace jukebox::download {

Future<Result<ByteVector>> startYoutubeDownload(const std::string& id) {
    if (id.length() != 11) {
        co_return Err("Invalid YouTube ID");
    }

    auto metadata = co_await getMetadata(id);
    co_return co_await onMetadata(std::move(metadata));
}

}  // namespace jukebox::download

Result<std::string> getUrlFromMetadataPayload(web::WebResponse r) {
    if (!r.ok()) {
        return Err(fmt::format(
            "cobalt metadata query failed with status code {}", r.code()));
    }

    Result<matjson::Value> jsonRes = r.json();
    if (jsonRes.isErr()) {
        return Err("cobalt metadata query returned invalid JSON");
    }

    matjson::Value payload = jsonRes.unwrap();
    if (!payload.contains("status") || !payload["status"].isString() ||
        payload["status"].asString().unwrap() != "stream") {
        return Err("Invalid metadata status");
    }

    if (!payload.contains("url") || !payload["url"].isString()) {
        return Err("No download URL returned");
    }

    return Ok(payload["url"].asString().unwrap());
}

web::WebFuture getMetadata(const std::string& id) {
    int timeout = Mod::get()->getSettingValue<int>("download-timeout");

    if (timeout < 30) {
        timeout = 30;
    }

    return web::WebRequest()
        .timeout(std::chrono::seconds(timeout))
        .bodyJSON(matjson::makeObject(
            {{"url", fmt::format("https://www.youtube.com/watch?v={}", id)},
             {"aFormat", "mp3"},
             {"isAudioOnly", "true"}}))
        .header("Accept", "application/json")
        .header("Content-Type", "application/json")
        .post("https://api.cobalt.tools/api/json");
}

Future<Result<ByteVector>> onMetadata(web::WebResponse result) {
    Result<std::string> res = getUrlFromMetadataPayload(std::move(result));
    if (res.isErr()) {
        co_return Err(res.unwrapErr());
    }

    co_return co_await jukebox::download::startHostedDownload(res.unwrap());
}
