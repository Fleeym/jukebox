#include <jukebox/utils/trim.hpp>

#include <algorithm>
#include <string>

namespace jukebox {

void left_trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

void right_trim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

void trim(std::string& s) {
    right_trim(s);
    left_trim(s);
}

}  // namespace jukebox
