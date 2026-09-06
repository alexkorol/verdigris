#pragma once

// VG-UI-005: side map + route card. Zoom and opacity are overlay
// settings; they cannot change world coordinates or invent targets the
// snapshot did not publish. Owner Demo journeys stay untouched.

#include <cctype>
#include <string>

namespace verdigris::client::ui {

struct MapOverlay {
  int zoom_step = 0;
  int opacity = 220;
};

struct MapTarget {
  const char* id = "";
  bool alive = true;
  bool on_snapshot = true;
};

inline bool leaky_zoom_paints_off_snapshot(int zoom_step, bool on_snapshot) {
  return zoom_step >= 2 && !on_snapshot;
}

inline bool overlay_paints_blip(const MapOverlay& overlay, const MapTarget& target) {
  (void)overlay;
  return target.alive && target.on_snapshot;
}

inline bool overlay_mutates_world(const MapOverlay&) { return false; }

inline int clamp_zoom(int step) {
  if (step < 0) return 0;
  if (step > 2) return 2;
  return step;
}

inline int clamp_opacity(int opacity) {
  if (opacity < 40) return 40;
  if (opacity > 255) return 255;
  return opacity;
}

inline const char* route_risk_fact(bool slay_wardens, bool extract) {
  if (extract) return "risk extract";
  if (slay_wardens) return "risk wardens";
  return "risk none posted";
}

inline const char* route_return_fact(bool has_extraction) {
  return has_extraction ? "return pad" : "return town";
}

// Owner-facing title. A colon-protocol id (`route:tin:1:0`) cannot be the
// card heading; F3 keeps the raw id. Unknown families stay "Expedition"
// rather than dumping the wire token.
inline std::string route_owner_title(const std::string& route_id) {
  if (route_id.empty() || route_id == "surface") return "Surface";
  if (route_id == "route:tin:1:0") return "Tin village";
  if (route_id == "route:tin:2:0") return "Tin fields";
  if (route_id == "route:salt:1:0") return "Salt village";
  if (route_id.rfind("town:", 0) == 0) return "Town square";
  if (route_id.rfind("branch:", 0) == 0) return "Side path";
  if (route_id.rfind("route:", 0) == 0) {
    const auto first = route_id.find(':');
    const auto second = route_id.find(':', first + 1);
    if (first != std::string::npos && second != std::string::npos &&
        second > first + 1) {
      std::string family = route_id.substr(first + 1, second - first - 1);
      if (!family.empty())
        family[0] = static_cast<char>(
            std::toupper(static_cast<unsigned char>(family[0])));
      const bool deep = route_id.find(":2:") != std::string::npos;
      return family + (deep ? " fields" : " village");
    }
  }
  if (route_id.find(':') != std::string::npos) return "Expedition";
  return route_id;
}

inline std::string route_theme_label(const std::string& theme) {
  if (theme.empty()) return "Unknown ground";
  if (theme == "town") return "Town road";
  if (theme == "grove") return "Grove";
  if (theme == "crypt") return "Crypt";
  if (theme == "wilds") return "Wilds";
  if (theme == "marsh") return "Marsh";
  if (theme == "dungeon") return "Dungeon";
  std::string label = theme;
  label[0] = static_cast<char>(
      std::toupper(static_cast<unsigned char>(label[0])));
  return label;
}

}  // namespace verdigris::client::ui
