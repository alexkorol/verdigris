#pragma once

// VG-UI-008: XInput pad sampled on the fixed tick (never from WM_MOUSEMOVE).
// Injected PadReport drives headless scenarios; mouse coordinates cannot
// mark the pad connected.

namespace verdigris::client {

struct PadReport {
  bool inject = false;     // scenario seam; skips XInput
  bool connected = false;
  int dx = 0;              // -1..1 left stick / d-pad
  int dy = 0;
  bool a = false;          // strike
  bool b = false;          // dash
  bool x = false;          // take
  bool y = false;          // gear / inventory focus
  bool start = false;
  const char* hotplug = "";  // "in" | "out" | ""
};

inline const char* owner_pad_glyphs_label() { return "Pad glyphs"; }
inline const char* owner_a_strike_label() { return "A strike"; }
inline bool pad_strip_covers_hud_fails_review(bool overlap) { return overlap; }

}  // namespace verdigris::client
