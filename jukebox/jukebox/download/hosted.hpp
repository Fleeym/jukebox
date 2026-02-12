#pragma once

#include <string_view>

#include <Geode/Result.hpp>
#include <Geode/utils/general.hpp>
#include <arc/future/Future.hpp>

namespace jukebox::download {

arc::Future<geode::Result<geode::ByteVector>> startHostedDownload(std::string_view url);

}  // namespace jukebox::download
