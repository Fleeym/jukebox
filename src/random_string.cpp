#include "random_string.hpp"

namespace nongd {
    std::string random_string(size_t length)
    {
        auto randchar = []() -> char
        {
            const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<> dis(0, max_index);
            return charset[ dis(gen) ];
        };
        std::string str(length,0);
        std::generate_n(str.begin(), length, randchar);
        return str;
    }
}