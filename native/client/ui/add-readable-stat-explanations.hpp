#pragma once

// VG-UI-004: expandable attack sources. Dormant conditionals stay labeled
// inactive and cannot fold into the painted Attack total. Core STAT algebra
// stays with Kimi.

namespace verdigris::client::ui {

struct StatSources {
  int base = 0;
  int gear = 0;
  int passive = 0;
  int conditional = 0;
  bool conditional_active = false;
  bool expanded = false;
};

inline int active_attack(const StatSources& src) {
  int total = src.base + src.gear + src.passive;
  if (src.conditional_active) total += src.conditional;
  return total;
}

inline bool folds_dormant_into_attack(const StatSources& src, int painted) {
  if (src.conditional_active || src.conditional == 0) return false;
  return painted == src.base + src.gear + src.passive + src.conditional;
}

inline const char* conditional_label(const StatSources& src) {
  return src.conditional_active ? "active" : "inactive";
}

inline int extra_source_rows(const StatSources& src) {
  return src.expanded ? 4 : 0;
}

}  // namespace verdigris::client::ui
