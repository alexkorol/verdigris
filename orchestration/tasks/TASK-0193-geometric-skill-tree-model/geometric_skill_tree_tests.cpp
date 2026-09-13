// geometric_skill_tree_tests.cpp — TASK-0193 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "geometric_skill_tree.hpp"

using namespace geometric_skill_tree;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void expect_status(Status got, Status expected, const std::string& label) {
  check(got == expected,
        label + ": expected " + name(expected) + " got " + name(got));
}

void test_demo_slice_geometry() {
  const State s = make_owner_demo_first_level_slice();
  check(s.seat_count == 7, "demo slice has seven seats");
  check(s.seats[0].active, "center seat starts active");
  check(s.seats[0].pos == Axial{0, 0}, "center axial coords");
  for (std::uint8_t i = 1; i < s.seat_count; ++i) {
    check(seat_ring(s.seats[i]) == 1, "ring-1 neighbor ring distance");
    check(is_adjacent_to_active(s, i), "ring-1 neighbor adjacent to center");
  }
  check(s.seats[1].type == SeatType::Sign, "east neighbor is sign seat");
  check(s.seats[4].type == SeatType::Socket, "socket seat on lattice");
}

void test_level_unlock_and_single_allocation() {
  State s = make_owner_demo_first_level_slice();
  expect_status(allocate_at(s, Axial{1, 0}), Status::TreeLocked, "locked pre-level");
  expect_status(on_player_level(s, kOwnerDemoFirstLevel), Status::Ok, "level up");
  check(s.tree_unlocked, "tree unlocked at first level-up");
  check(s.skill_points == kOwnerDemoFirstLevelPoints, "one skill point granted");

  expect_status(allocate_at(s, Axial{0, 1}), Status::Ok, "allocate passive");
  check(s.skill_points == 0, "points spent");
  check(s.allocated_count == 2, "two active seats");
  expect_status(allocate_at(s, Axial{1, 0}), Status::NoPoints, "no second point");
}

void test_sign_cap_and_adjacency() {
  State s = make_owner_demo_first_level_slice();
  on_player_level(s, kOwnerDemoFirstLevel);
  expect_status(allocate_at(s, Axial{1, 0}), Status::Ok, "allocate sign");
  check(s.sign_allocated, "sign flag set");

  State s2 = make_owner_demo_first_level_slice();
  on_player_level(s2, kOwnerDemoFirstLevel);
  expect_status(allocate_at(s2, Axial{10, 0}), Status::InvalidSeat,
                "invalid coordinate");
}

void test_wizard_point_pool_constant() {
  check(kWizardPhase0SkillPoints == 140, "WIZARD phase-0 pool preserved");
}

}  // namespace

int main() {
  test_demo_slice_geometry();
  test_level_unlock_and_single_allocation();
  test_sign_cap_and_adjacency();
  test_wizard_point_pool_constant();
  std::cout << "geometric_skill_tree_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
