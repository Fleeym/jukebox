#pragma once

#include <string>

#include <jukebox/download/download.hpp>

namespace jukebox {

namespace download {

DownloadTask startYoutubeDownload(const std::string& id);

}

}
