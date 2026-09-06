#pragma once

// VG-MOVE-002: aim is a held intent. A move command must not replace it.

namespace verdigris::client::move {

struct AimHold {
  int dx = 1;
  int dy = 0;
  bool held = false;
};

inline void remember_aim(AimHold& hold, int dx, int dy) {
  if (dx == 0 && dy == 0) return;
  hold.dx = dx;
  hold.dy = dy;
  hold.held = true;
}

inline bool move_clobbered_aim(const AimHold& hold, int face_dx, int face_dy) {
  if (!hold.held) return false;
  return hold.dx != face_dx || hold.dy != face_dy;
}

}  // namespace verdigris::client::move
