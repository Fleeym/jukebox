#include "random_string.hpp"
#include <string_view>

namespace nongd {
    std::string random_string(size_t length)
    {
        static std::string_view const charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (charset.size() - 1);
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        std::uniform_int_distribution<> dis(0, max_index);
        std::string str(length,0);
        for (size_t i = 0; i < length; i++) {
            str[i] = charset[dis(gen)];
        }
        return str;
    }
}