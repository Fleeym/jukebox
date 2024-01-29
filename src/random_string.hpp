#pragma once

#include <string>
#include <algorithm>

namespace nongd {
    /**
     * Gracefully stolen from https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
    */
    std::string random_string(size_t length);
}
