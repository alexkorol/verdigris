#pragma once

// VG-SOUND-005: one region ambience loop, named in owner language. A
// protocol token (`ambience:route:tin:1:0`) or stacked reentry loops
// cannot certify. Zone graph authority stays with Kimi (VG-WORLD-007).

#include <string>

#include "ui/implement-map-and-route-explanation.hpp"

namespace verdigris::client::ambience {

inline std::string cue_id_for(const std::string& route_id) {
  const std::string route =
      route_id.empty() ? std::string("surface") : route_id;
  return std::string("ambience:") + route;
}

inline std::string owner_loop_label(const std::string& route_id) {
  return std::string("Loop ") +
         verdigris::client::ui::route_owner_title(route_id) + " wind";
}

inline bool protocol_token_fails_review(bool owner_chip_painted) {
  return !owner_chip_painted;
}

inline bool stacked_loops_fail_review(int voiced_count) {
  return voiced_count > 1;
}

inline bool protocol_text_fails_review(const std::string& painted) {
  return painted.find("ambience:") != std::string::npos ||
         painted.find("route:") != std::string::npos;
}

}  // namespace verdigris::client::ambience
