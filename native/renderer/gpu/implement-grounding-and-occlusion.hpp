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

}  // namespace verdigris::gpu
