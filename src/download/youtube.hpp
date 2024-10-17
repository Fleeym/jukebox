#pragma once

#include <string>

#include "download/download.hpp"

namespace jukebox {

namespace download {

DownloadTask startYoutubeDownload(const std::string& id);

}

}
