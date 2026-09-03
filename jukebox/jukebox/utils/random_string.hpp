#pragma once

#include <cstddef>
#include <string>

namespace jukebox {

/**
 * Generates a random string, was formerly gracefully stolen from StackOverflow
 */
std::string random_string(size_t length);

}  // namespace jukebox
