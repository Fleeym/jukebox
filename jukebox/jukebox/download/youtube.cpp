#include <jukebox/download/youtube.hpp>

#include <matjson.hpp>
#include <string>

#include <Geode/loader/Mod.hpp>
#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>

#include <jukebox/download/download.hpp>
#include <jukebox/download/hosted.hpp>

using namespace geode::prelude;

web::WebTask getMetadata(const std::string& id);
Result<std::string> getUrlFromMetadataPayload(web::WebResponse* resp);
jukebox::download::DownloadTask onMetadata(web::WebResponse*);

namespace jukebox {

namespace download {

DownloadTask startYoutubeDownload(const std::string& id) {
    if (id.length() != 11) {
        return DownloadTask::immediate(Err("Invalid YouTube ID"));
    }

    return getMetadata(id).chain(
        [](web::WebResponse* r) { return onMetadata(r); });
}

}  // namespace download

}  // namespace jukebox

Result<std::string> getUrlFromMetadataPayload(web::WebResponse* r) {
    if (!r->ok()) {
        return Err(fmt::format(
            "cobalt metadata query failed with status code {}", r->code()));
    }

    Result<matjson::Value> jsonRes = r->json();
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

web::WebTask getMetadata(const std::string& id) {
    int timeout = Mod::get()->getSettingValue<int>("download-timeout");

    if (timeout < 30) {
        timeout = 30;
    }

    return web::WebRequest()
        .timeout(std::chrono::seconds(30))
        .bodyJSON(matjson::makeObject(
            {{"url", fmt::format("https://www.youtube.com/watch?v={}", id)},
             {"aFormat", "mp3"},
             {"isAudioOnly", "true"}}))
        .header("Accept", "application/json")
        .header("Content-Type", "application/json")
        .post("https://api.cobalt.tools/api/json");
}

jukebox::download::DownloadTask onMetadata(web::WebResponse* result) {
    Result<std::string> res = getUrlFromMetadataPayload(result);
    if (res.isErr()) {
        return jukebox::download::DownloadTask::immediate(Err(res.unwrapErr()));
    }

    return jukebox::download::startHostedDownload(res.unwrap());
}
