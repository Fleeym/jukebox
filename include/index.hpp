#pragma once

#include <fmt/core.h>
#include <matjson.hpp>
#include <optional>

#include "platform.hpp"

namespace jukebox {

struct JUKEBOX_DLL IndexSource final {
    std::string m_url;
    bool m_userAdded;
    bool m_enabled;

    bool operator==(IndexSource const& other) const {
        return m_url == other.m_url && m_userAdded == other.m_userAdded && m_enabled == other.m_enabled;
    }
};

struct IndexMetadata final {
  struct Links final {
      std::optional<std::string> m_discord = std::nullopt;
  };

  struct Features {
      struct RequestParams final {
          std::string m_url;
      };

      struct Submit final {
          std::optional<RequestParams> m_requestParams = std::nullopt;
          std::optional<std::string> m_preSubmitMessage;
      };

      struct Report final {
          std::optional<RequestParams> m_requestParams = std::nullopt;
      };

      std::optional<Submit> m_submit = std::nullopt;
      std::optional<Report> m_report = std::nullopt;
  };

  int m_manifest;
  std::string m_url;
  std::string m_id;
  std::string m_name;
  std::optional<std::string> m_description;
  std::optional<int> m_lastUpdate;
  Links m_links;
  Features m_features;
};

}
