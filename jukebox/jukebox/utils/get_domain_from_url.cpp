#include "get_domain_from_url.hpp"

#include <optional>
#include <string>
#include <regex>

std::optional<std::string> getDomainFromUrl(const std::string& url) {
    std::regex re(R"(^(?:.*?\/\/)?([^\/:]+))");
    std::smatch match;
    if (std::regex_search(url, match, re) && match.size() > 1) {
        return match[1].str();
    }
    return std::nullopt;
}