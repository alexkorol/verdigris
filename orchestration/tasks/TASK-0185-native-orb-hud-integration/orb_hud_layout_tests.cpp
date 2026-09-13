// orb_hud_layout_tests.cpp — TASK-0185 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "orb_hud_layout.hpp"

using namespace orb_hud_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_orb_anchors_1080p() {
  const Viewport vp{1920, 1080};
  const OrbAnchor life = vital_orb_anchor(vp, false);
  const OrbAnchor resource = vital_orb_anchor(vp, true);
  check(life.cx == 52, "life orb center x");
  check(resource.cx == 1868, "resource orb center x");
  check(life.cy > 0, "life orb center y positive");
  check(life.trace_bounds.width == 74, "life trace width includes pulse");
}

void test_draw_plans_from_stats() {
  const Viewport vp{1366, 768};
  VitalStats stats;
  stats.life = 25;
  stats.life_max = 100;
  stats.resource = 80;
  stats.resource_max = 100;
  const LayoutPlan plan = plan_vital_orbs(vp, stats);
  check(plan.valid, "layout valid");
  check(plan.life.draw.band == orb_renderer::DepletionBand::Low,
        "low life band");
  check(plan.resource.draw.band == orb_renderer::DepletionBand::Full,
        "full resource band");
  check(plan.life.draw.layers[1].active, "mask layer always active");
}

void test_reserved_stone_layer() {
  const Viewport vp{1280, 720};
  VitalStats stats;
  stats.life = 60;
  stats.life_max = 100;
  stats.resource = 40;
  stats.resource_max = 100;
  stats.resource_reserved = 20;
  const LayoutPlan plan = plan_vital_orbs(vp, stats);
  check(plan.resource.draw.layers[4].active, "reserved stone layer active");
  check(plan.resource.draw.reserve_ratio == 200, "20% reserved");
}

void test_invalid_stats_rejected() {
  const Viewport vp{960, 600};
  VitalStats bad;
  bad.life_max = 0;
  bad.resource_max = 100;
  check(!plan_vital_orbs(vp, bad).valid, "invalid stats rejected");
}

void test_checksum_stable() {
  const Viewport vp{1920, 1080};
  VitalStats stats;
  stats.life = 90;
  stats.life_max = 100;
  stats.resource = 50;
  stats.resource_max = 100;
  const std::uint32_t a = plan_checksum(plan_vital_orbs(vp, stats));
  const std::uint32_t b = plan_checksum(plan_vital_orbs(vp, stats));
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_orb_anchors_1080p();
  test_draw_plans_from_stats();
  test_reserved_stone_layer();
  test_invalid_stats_rejected();
  test_checksum_stable();
  std::cout << "orb_hud_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
