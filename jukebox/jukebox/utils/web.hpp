#pragma once

#include <string>

#include <Geode/utils/web.hpp>

namespace jukebox::utils::web {

std::string getErrorFromResponse(const geode::utils::web::WebResponse& response);

} // namespace jukebox::utils::web
