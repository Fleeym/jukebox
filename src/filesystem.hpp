#pragma once

#include <Geode/Geode.hpp>

#ifndef GEODE_IS_MACOS
#include <filesystem>
namespace fs = std::filesystem;
#else
namespace fs = ghc::filesystem;
#endif