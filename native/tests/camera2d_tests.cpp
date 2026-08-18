// camera2d_tests.cpp — locks the D-118 top-down invariants (architect
// scaffolding, TASK-0050). These tests define "correct"; the client
// implementation must keep them green.

#include "../client/camera2d.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

int failures = 0;

void expect(bool condition, const char* label) {
  if (!condition) {
    std::printf("FAIL: %s\n", label);
    ++failures;
  }
}

// 1. Translation invariance: moving the camera never changes the on-screen
//    delta between two world points. This is the exact bug the old
//    depth-scaled projection had — scenery sliding against player motion.
void translation_invariance() {
  camera2d::Screen screen{1280, 720};
  const double ax = 3.25, ay = -7.5;
  const double bx = 11.0, by = 4.75;
  camera2d::Camera cam;
  cam.zoom = 48.0;

  int base_dx = 0, base_dy = 0;
  bool first = true;
  for (double cx = -20.0; cx <= 20.0; cx += 1.37) {
    for (double cy = -20.0; cy <= 20.0; cy += 2.11) {
      cam.x = cx;
      cam.y = cy;
      const auto pa = camera2d::project(cam, screen, ax, ay);
      const auto pb = camera2d::project(cam, screen, bx, by);
      const int dx = pb.x - pa.x;
      const int dy = pb.y - pa.y;
      if (first) {
        base_dx = dx;
        base_dy = dy;
        first = false;
      } else {
        expect(dx == base_dx && dy == base_dy,
               "screen delta must be camera-independent");
        if (failures) return;
      }
    }
  }
  expect(base_dx == static_cast<int>(std::lround((bx - ax) * cam.zoom)) ||
             std::abs(base_dx - (bx - ax) * cam.zoom) <= 1.0,
         "screen delta equals world delta * zoom (x)");
  expect(std::abs(base_dy - (by - ay) * cam.zoom) <= 1.0,
         "screen delta equals world delta * zoom (y)");
}

// 2. Round-trip: unproject(project(w)) returns w within one pixel of world
//    error at any zoom.
void round_trip() {
  camera2d::Screen screen{1920, 1080};
  for (double zoom : {16.0, 48.0, 96.0}) {
    camera2d::Camera cam{5.5, -3.25, zoom};
    const double wx = -12.75, wy = 33.5;
    const auto p = camera2d::project(cam, screen, wx, wy);
    double rx = 0, ry = 0;
    camera2d::unproject(cam, screen, p.x, p.y, rx, ry);
    const double tol = 1.0 / zoom;  // one pixel of world error
    expect(std::abs(rx - wx) <= tol, "round trip x within one pixel");
    expect(std::abs(ry - wy) <= tol, "round trip y within one pixel");
  }
}

// 3. Uniform scale: sprite scale comes from zoom alone — identical for every
//    entity regardless of position (no depth scaling of any kind).
void uniform_scale() {
  camera2d::Screen screen{1280, 720};
  camera2d::Camera cam{0.0, 0.0, 40.0};
  const auto near = camera2d::project(cam, screen, 0.0, 0.5);
  const auto far = camera2d::project(cam, screen, 0.0, -200.0);
  expect(near.scale == far.scale, "scale must not depend on position");
  expect(near.scale == cam.zoom, "scale equals zoom");
}

// 4. Camera centering: the camera's own world position lands in the exact
//    screen center.
void centering() {
  camera2d::Screen screen{1000, 600};
  camera2d::Camera cam{123.4, -56.7, 32.0};
  const auto p = camera2d::project(cam, screen, cam.x, cam.y);
  expect(p.x == screen.width / 2 && p.y == screen.height / 2,
         "camera position projects to screen center");
}

}  // namespace

int main() {
  translation_invariance();
  round_trip();
  uniform_scale();
  centering();
  if (failures) {
    std::printf("camera2d tests: %d FAILURE(S)\n", failures);
    return 1;
  }
  std::printf("camera2d tests: PASS\n");
  return 0;
}
