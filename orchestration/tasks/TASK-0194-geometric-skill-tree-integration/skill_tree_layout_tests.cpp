// skill_tree_layout_tests.cpp — TASK-0194 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "skill_tree_layout.hpp"

using namespace skill_tree_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_center_seat_at_panel_origin() {
  geometric_skill_tree::State state =
      geometric_skill_tree::make_owner_demo_first_level_slice();
  check(geometric_skill_tree::on_player_level(state,
                                              geometric_skill_tree::kOwnerDemoFirstLevel) ==
            geometric_skill_tree::Status::Ok,
        "unlock tree");

  const Viewport vp{1920, 1080};
  const LayoutPlan plan = plan_level_up_panel(vp, state);
  check(plan.valid, "layout valid");
  check(plan.seat_count == 7u, "seven seats");
  check(plan.tree_unlocked, "tree unlocked");
  check(plan.spendable_points == 1u, "one point");
  check(plan.seats[0].center.x == plan.panel_center.x, "center seat x");
  check(plan.seats[0].center.y == plan.panel_center.y, "center seat y");
  check(plan.seats[0].active, "center active");
}

void test_sign_seat_clickable() {
  geometric_skill_tree::State state =
      geometric_skill_tree::make_owner_demo_first_level_slice();
  check(geometric_skill_tree::on_player_level(state,
                                              geometric_skill_tree::kOwnerDemoFirstLevel) ==
            geometric_skill_tree::Status::Ok,
        "unlock tree");
  const LayoutPlan plan =
      plan_level_up_panel(Viewport{1280, 720}, state);
  check(plan.seats[1].type == geometric_skill_tree::SeatType::Sign, "sign seat");
  check(plan.seats[1].clickable, "sign clickable");
  check(plan.seats[1].hit.valid(), "sign hit rect");
}

void test_allocation_clears_clickable() {
  geometric_skill_tree::State state =
      geometric_skill_tree::make_owner_demo_first_level_slice();
  check(geometric_skill_tree::on_player_level(state,
                                              geometric_skill_tree::kOwnerDemoFirstLevel) ==
            geometric_skill_tree::Status::Ok,
        "unlock tree");
  check(geometric_skill_tree::allocate_at(state, {1, 0}) ==
            geometric_skill_tree::Status::Ok,
        "allocate sign");
  const LayoutPlan plan = plan_level_up_panel(Viewport{1366, 768}, state);
  check(!plan.seats[1].clickable, "sign no longer clickable");
  check(plan.spendable_points == 0u, "points spent");
}

void test_locked_tree_not_clickable() {
  geometric_skill_tree::State state =
      geometric_skill_tree::make_owner_demo_first_level_slice();
  const LayoutPlan plan = plan_level_up_panel(Viewport{960, 600}, state);
  check(!plan.tree_unlocked, "tree locked");
  check(!plan.seats[1].clickable, "no spend while locked");
}

void test_checksum_stable() {
  geometric_skill_tree::State state =
      geometric_skill_tree::make_owner_demo_first_level_slice();
  check(geometric_skill_tree::on_player_level(state,
                                              geometric_skill_tree::kOwnerDemoFirstLevel) ==
            geometric_skill_tree::Status::Ok,
        "unlock tree");
  const LayoutPlan plan = plan_level_up_panel(Viewport{1920, 1080}, state);
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_center_seat_at_panel_origin();
  test_sign_seat_clickable();
  test_allocation_clears_clickable();
  test_locked_tree_not_clickable();
  test_checksum_stable();
  std::cout << "skill_tree_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
