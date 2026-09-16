// chronicles_owner_pane_layout_tests.cpp — TASK-0197 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "chronicles_owner_pane_layout.hpp"

using namespace chronicles_owner_pane_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_journey_pane_rows() {
  const chronicles_owner_pane::PaneState pane =
      chronicles_owner_pane::make_owner_demo_journey_pane();
  const LayoutPlan plan = plan_chronicles_pane(Viewport{1920, 1080}, pane);
  check(plan.valid, "layout valid");
  check(plan.row_count == pane.count, "all rows laid out");
  check(plan.panel_frame.valid, "framekit chrome valid");
  check(plan.rows[0].headline.valid(), "headline band");
  check(plan.rows[0].detail.valid(), "detail band");
}

void test_checksum_stable() {
  const LayoutPlan plan = plan_chronicles_pane(
      Viewport{1366, 768},
      chronicles_owner_pane::make_owner_demo_journey_pane());
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_journey_pane_rows();
  test_checksum_stable();
  std::cout << "chronicles_owner_pane_layout_tests: " << g_checks
            << " checks passed\n";
  return 0;
}
