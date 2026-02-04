#pragma once

#include <string>

#include <Geode/utils/general.hpp>
#include <arc/future/Future.hpp>

namespace jukebox::download {

arc::Future<geode::Result<geode::ByteVector>> startYoutubeDownload(const std::string& id);

} // namespace jukebox::download
