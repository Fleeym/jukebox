#pragma once

#include <string>
#include <string_view>

#include <Geode/utils/general.hpp>
#include <Geode/Result.hpp>
#include <arc/future/Future.hpp>

namespace jukebox::download::cobalt {

arc::Future<geode::Result<std::string>> process(std::string_view url);

} // namespace jukebox::download
