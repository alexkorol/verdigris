#pragma once

// VG-UI-005: side map + route card. Zoom and opacity are overlay
// settings; they cannot change world coordinates or invent targets the
// snapshot did not publish. Owner Demo journeys stay untouched.

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

}  // namespace verdigris::client::ui
