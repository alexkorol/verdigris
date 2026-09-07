// camera2d.hpp — D-118 orthographic top-down camera (architect scaffolding,
// TASK-0050). Pure math, no windowing dependencies, header-only.
//
// THE INVARIANT that killed the old projection (and must never regress):
// for any two world points A and B, the screen-space delta between them is
//   (B - A) * zoom
// exactly — independent of camera position. No coordinate of any entity may
// depend on its depth relative to the camera. Scenery cannot slide against
// player motion under this transform; camera2d_tests.cpp locks it in.
//
// Usage (TASK-0050): replace main.cpp's project()/unproject() with
// camera2d::project()/camera2d::unproject(). Delete Camera::pitch_deg,
// perspective, anchor, fog, depth_scale(), ground_squash(), and
// update_camera_perspective() — the 2.5D path goes away entirely (a later
// wave reintroduces projection correctly, per the webchat demo reference).

#pragma once

#include <cmath>

namespace camera2d {

struct Camera {
  double x = 0.0;     // world-space center of the view
  double y = 0.0;
  double zoom = 48.0; // pixels per world unit (uniform, both axes)
};

struct Screen {
  int width = 0;
  int height = 0;
};

struct Point {
  int x = 0;
  int y = 0;
  double scale = 1.0; // sprite scale factor: always camera.zoom-derived,
                      // never depth-derived
};

inline Point project(const Camera& camera, const Screen& screen, double wx,
                     double wy) {
  Point out;
  out.x = screen.width / 2 +
          static_cast<int>(std::lround((wx - camera.x) * camera.zoom));
  out.y = screen.height / 2 +
          static_cast<int>(std::lround((wy - camera.y) * camera.zoom));
  out.scale = camera.zoom;
  return out;
}

// Exact inverse of project() up to integer rounding of the screen point.
inline void unproject(const Camera& camera, const Screen& screen, int sx,
                      int sy, double& wx, double& wy) {
  wx = camera.x + (sx - screen.width / 2) / camera.zoom;
  wy = camera.y + (sy - screen.height / 2) / camera.zoom;
}

// Draw-order key for painter's algorithm in top-down: sort by world y, then
// world x. Entities standing "lower" on screen draw later (in front).
inline double draw_order_key(double wy, double wx) { return wy * 1e6 + wx; }

inline const char* owner_uniform_pan_label() { return "Uniform pan"; }
inline const char* owner_zoom_lock_label() { return "Zoom lock"; }
inline bool zoom_strip_covers_hud_fails_review(bool overlap) { return overlap; }
inline const char* owner_kit_lock_label() { return "Kit lock"; }
inline const char* owner_same_delta_label() { return "Same delta"; }
inline bool kit_lock_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}

}  // namespace camera2d
