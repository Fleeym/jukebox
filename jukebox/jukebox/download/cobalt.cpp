#include <jukebox/download/cobalt.hpp>

#include <string>
#include <string_view>

#include <matjson.hpp>
#include <fmt/format.h>
#include <Geode/loader/Mod.hpp>
#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;
using namespace arc;

std::string interpretCobaltError(web::WebResponse r);

namespace jukebox::download::cobalt {

Future<Result<std::string>> process(std::string_view url) {
    int timeout = Mod::get()->getSettingValue<int>("download-timeout");
    const auto cobaltUrl = Mod::get()->getSettingValue<std::string>("cobalt-instance-url");
    const auto cobaltApiKey = Mod::get()->getSettingValue<std::string>("cobalt-instance-api-key");

    if (cobaltUrl.empty()) {
        co_return Err("No cobalt instance configured. You can configure one in the Jukebox settings menu.");
    }

    if (timeout < 30) {
        timeout = 30;
    }

    auto request = web::WebRequest()
        .timeout(std::chrono::seconds(timeout))
        .bodyJSON(matjson::makeObject({
            {"url", url},
            {"aFormat", "mp3"},
            {"isAudioOnly", "true"}
            }))
        .header("Accept", "application/json")
        .header("Content-Type", "application/json");

    if (!cobaltApiKey.empty()) {
        request.header("Authorization", fmt::format("Api-Key {}", cobaltApiKey));
    }

    const web::WebResponse response = co_await request.post(cobaltUrl);

    if (response.badServer()) {
        co_return Err("The cobalt instance encountered a server error. Code {}.", response.code());
    }
    if (response.badClient()) {
        co_return Err(interpretCobaltError(response));
    }

    GEODE_UNWRAP_OR_ELSE(json, err, std::move(response).json()) {
        co_return Err("Invalid response received from cobalt");
    }

    const auto status = json["status"].asString().unwrapOr("unknown");

    if (status != "tunnel" && status != "redirect") {
        co_return Err(fmt::format("Invalid status received from cobalt: {}", status));
    }

    if (GEODE_UNWRAP_EITHER(downloadUrl, err, std::move(json)["url"].asString())) {
        co_return Ok(downloadUrl);
    }

    co_return Err("No URL found in cobalt metadata request");
}

}  // namespace jukebox::download

std::string interpretCobaltError(web::WebResponse r) {
    if (r.ok()) {
        return "";
    }

    GEODE_UNWRAP_OR_ELSE(json, err, std::move(r).json()) {
        return "Invalid response received from cobalt";
    }

    if (json["status"].asString().unwrapOr("unknown") != "error") {
        return "Invalid response received from cobalt";
    }

    auto error = json["error"].asString().unwrapOr("no.error");

    if (error == "no.error") {
        return "No error provided";
    }
    if (error == "error.api.content.video.unavailable" || error == "error.api.content.post.unavailable") {
        return "cobalt couldn't find the URL provided";
    }
    if (error == "error.auth.jwt.missing" || error == "error.auth.key.missing" || error == "error.auth.key.not_api_key") {
        return "This cobalt instance requires an API key to work, you can configure one in the Jukebox settings menu.";
    }
    if (error.starts_with("error.auth.key")) {
        return "The provided API key is invalid.";
    }

    return fmt::format("Cobalt error: {}", error);
}
