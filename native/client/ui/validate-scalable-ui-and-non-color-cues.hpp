#pragma once

// VG-UI-007: HUD chrome must fit the pane at the type floor. Shrinking
// below that floor is not the overflow fix. A clipped gear footer cannot
// certify readable controls.

namespace verdigris::client::ui {

inline const char* owner_gear_footer_place_label() {
  return "Drag to place | drop on Weapon";
}

inline const char* owner_gear_close_label() {
  return "Enter equips | I or Esc closes";
}

inline bool rect_inside_pane(int x, int y, int w, int h, int px, int py, int pw,
                             int ph) {
  return w > 0 && h > 0 && x >= px && y >= py && x + w <= px + pw &&
         y + h <= py + ph;
}

inline bool clipped_gear_footer_fails_review(bool inside) { return !inside; }

inline bool missing_gear_close_fails_review(bool present) { return !present; }

inline bool clipped_gear_stats_fails_review(bool inside) { return !inside; }

inline bool missing_gear_def_fails_review(bool present) { return !present; }

}  // namespace verdigris::client::ui
