// cartographer_adapter_tests.cpp — TASK-0191 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "cartographer_adapter.hpp"

using namespace cartographer_adapter;

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

void test_owner_demo_graph() {
  expect_status(validate_owner_demo_graph(), Status::Ok, "owner demo graph");
}

void test_thornward_plan() {
  MapGenPlan plan{};
  expect_status(build_mapgen_plan(kOwnerDemoZones[2], plan), Status::Ok,
                "thornward plan");
  check(std::string(plan.mapgen_zone) == "wilds", "thornward mapgen zone");
  check(std::string(plan.native_layout) == "gauntlet", "thornward layout");
  check(plan.min_spawn_slots >= 6, "gauntlet spawn slots");
  check(plan.seed != 0, "combined seed nonzero");
  check(plan.width == kDefaultWidth && plan.height == kDefaultHeight,
        "default dimensions");
}

void test_rift_hollow_warren() {
  MapGenPlan plan{};
  build_mapgen_plan(kOwnerDemoZones[3], plan);
  check(std::string(plan.mapgen_zone) == "dungeon", "rift dungeon zone");
  check(std::string(plan.resolved_theme) == "crypt", "rift crypt theme");
  check(plan.min_spawn_slots == 8, "warren spawn slots");
}

void test_reject_invalid_template() {
  ZoneSeed bad{"bad-zone", "labyrinth", "clearings", "forest", 1u, 0, nullptr};
  MapGenPlan plan{};
  expect_status(build_mapgen_plan(bad, plan), Status::InvalidTemplate,
                "bad template");
}

void test_reject_invalid_gate_ref() {
  static constexpr GateAnchor bad_gate{1, 1, "missing-zone", false};
  static constexpr ZoneSeed bad{
      "tmp", "wilds", "clearings", "forest", 1u, 1, &bad_gate};
  expect_status(validate_gate_refs(bad, kOwnerDemoZones, kOwnerDemoZoneCount),
                Status::InvalidZoneRef, "missing gate target");
}

}  // namespace

int main() {
  test_owner_demo_graph();
  test_thornward_plan();
  test_rift_hollow_warren();
  test_reject_invalid_template();
  test_reject_invalid_gate_ref();
  std::cout << "cartographer_adapter_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
