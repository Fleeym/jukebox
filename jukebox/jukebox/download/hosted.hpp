#pragma once

#include <string>

#include <jukebox/download/download.hpp>

namespace jukebox {

namespace download {

DownloadTask startHostedDownload(const std::string& url);

}

}  // namespace jukebox
