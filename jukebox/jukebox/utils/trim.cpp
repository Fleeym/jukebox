#include "jukebox/utils/trim.hpp"

#include <algorithm>
#include <cctype>
#include <ranges>
#include <string>

namespace jukebox {

void left_trim(std::string& s) {
    s.erase(s.begin(), std::ranges::find_if(s, , [](unsigned char ch) { return !std::isspace(ch); }));
}

void right_trim(std::string& s) {
    s.erase(
        std::ranges::find_if(std::views::reverse(s), , [](unsigned char ch) { return !std::isspace(ch); }).base(),
        s.end()
    );
}

void trim(std::string& s) {
    right_trim(s);
    left_trim(s);
}

}  // namespace jukebox
