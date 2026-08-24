// splash_menu_layout_tests.cpp — TASK-0183 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "splash_menu_layout.hpp"

using namespace splash_menu_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_title_safe_region_1080p() {
  const Viewport vp{1920, 1080};
  const LayoutPlan plan =
      plan_for_root(menu_scene::Root::Title, vp);
  check(plan.valid, "title layout valid");
  check(plan.title_safe.x == 288, "title safe x");
  check(plan.title_safe.y == 86, "title safe y");
  check(plan.title_safe.width == 1344, "title safe width");
  check(plan.title_safe.height == 238, "title safe height");
  check(plan.title_panel.valid, "title panel chrome valid");
  check(plan.pause_panel.valid == false, "no pause panel on title");
}

void test_pause_centered_panel() {
  const Viewport vp{960, 600};
  const LayoutPlan plan = plan_for_root(menu_scene::Root::Paused, vp);
  check(plan.valid, "pause layout valid");
  check(plan.pause_panel.valid, "pause chrome valid");
  const PixelRect& p = plan.pause_panel.panel;
  check(p.width == 268, "pause width ~28%");
  check(p.height == 240, "pause height ~40%");
  check(p.x == 346, "pause centered x");
  check(p.y == 180, "pause centered y");
}

void test_playing_has_background_only() {
  const Viewport vp{1280, 720};
  const LayoutPlan plan = plan_for_root(menu_scene::Root::Playing, vp);
  check(plan.valid, "playing layout valid");
  check(plan.background.visible, "background visible");
  check(!plan.title_panel.valid, "no title chrome while playing");
  check(!plan.pause_panel.valid, "no pause chrome while playing");
}

void test_invalid_viewport_rejected() {
  const Viewport bad{0, 720};
  check(!plan_for_root(menu_scene::Root::Title, bad).valid,
        "zero width rejected");
}

void test_deterministic_checksum() {
  const Viewport vp{1366, 768};
  const std::uint32_t a =
      plan_checksum(plan_for_root(menu_scene::Root::Title, vp));
  const std::uint32_t b =
      plan_checksum(plan_for_root(menu_scene::Root::Title, vp));
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_title_safe_region_1080p();
  test_pause_centered_panel();
  test_playing_has_background_only();
  test_invalid_viewport_rejected();
  test_deterministic_checksum();
  std::cout << "splash_menu_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
