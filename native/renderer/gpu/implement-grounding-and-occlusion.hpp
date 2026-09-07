#pragma once

// VG-GPU-005: one painter's-algorithm policy. World Y sorts scenery and
// actors; threat telegraphs paint after that pass so a foreground wall
// cannot erase the warning. Contact shadows stay at the feet.

#include "../../client/render_list.hpp"

namespace verdigris::gpu {

inline bool telegraph_draws_after_scenery(const render::List& list) {
  int last_scenery = -1;
  int last_telegraph = -1;
  for (int i = 0; i < static_cast<int>(list.size()); ++i) {
    if (list[static_cast<std::size_t>(i)].op == render::Op::Scenery)
      last_scenery = i;
    if (list[static_cast<std::size_t>(i)].op == render::Op::Telegraph)
      last_telegraph = i;
  }
  return last_telegraph >= 0 && last_telegraph > last_scenery;
}

inline bool hud_label_alone_fails_grounding_review(bool telegraph_painted) {
  return !telegraph_painted;
}

// A capture-black fill is how the old spawn still hid Sweep. Red warning
// chroma has to survive the 32bpp DIB path.
inline bool capture_black_telegraph_fails_review(int r, int g, int b) {
  return r < 32 && g < 32 && b < 32;
}

inline const char* owner_y_sort_label() { return "Y-sort"; }
inline const char* owner_sweep_disc_label() { return "Sweep disc"; }
inline bool grounding_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}

}  // namespace verdigris::gpu
