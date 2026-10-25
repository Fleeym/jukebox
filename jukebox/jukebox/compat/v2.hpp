#pragma once

#include <unordered_map>

#include <Geode/Result.hpp>

#include "jukebox/compat/compat.hpp"

namespace jukebox::compat::v2 {

bool manifestExists();
geode::Result<> backupManifest(bool deleteOrig = false);
std::filesystem::path manifestPath();
geode::Result<std::unordered_map<int, CompatManifest>> parseManifest();

}  // namespace jukebox::compat::v2
