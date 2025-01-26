#pragma once

#include <Geode/Result.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/utils/general.hpp>

namespace jukebox {

namespace download {

using DownloadTask = geode::Task<geode::Result<geode::ByteVector>, float>;

}

}  // namespace jukebox
