#include <jukebox/utils/random_string.hpp>
#include <Geode/utils/random.hpp>

namespace jukebox {

std::string random_string(size_t length) {
    static constexpr std::string_view charset =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    return geode::utils::random::generateString(length, charset);
}

}  // namespace jukebox