// zone_runtime_bridge_tests.cpp — TASK-0192 bridge tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "instance_gate_bridge.hpp"
#include "zone_runtime_bridge.hpp"

using namespace zone_runtime_bridge;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_owner_demo_runtime_plan() {
  const RuntimePlan plan = build_owner_demo_runtime();
  check(plan.valid, "runtime plan valid");
  check(plan.count == cartographer_adapter::kOwnerDemoZoneCount, "five zones");
}

void test_thornward_combat_zone() {
  const RuntimePlan plan = build_owner_demo_runtime();
  const ZoneRuntimeEntry& thornward = plan.zones[2];
  check(thornward.valid, "thornward entry valid");
  check(thornward.policy.allow_fresh, "thornward allows fresh");
  check(thornward.policy.lifetime_ticks == 36000u, "thornward lifetime");
  check(std::string(thornward.mapgen.native_layout) == "gauntlet",
        "thornward gauntlet layout");
  check(thornward.gates.count == 3u, "three thornward gates");
}

void test_crossroads_town_policy() {
  const RuntimePlan plan = build_owner_demo_runtime();
  const ZoneRuntimeEntry& town = plan.zones[1];
  check(town.policy.kind == instance_refresh::ZoneKind::Town, "town kind");
  check(!town.policy.allow_fresh, "town no fresh");
  check(town.gates.count == 2u, "two crossroads gates");
}

void test_gate_travel_via_bridge() {
  const RuntimePlan plan = build_owner_demo_runtime();
  const gate_interaction::World& world = plan.zones[1].gates;
  const gate_interaction::GateDef& gate = world.gates[0];
  check(gate.id == 10u, "thornward exit gate");
  const gate_interaction::State state =
      gate_interaction::update_hover(world, gate.center);

  instance_gate_bridge::ZoneTable zones;
  zones.zones[0] = plan.zones[2].policy;
  zones.count = 1;

  instance_refresh::Registry registry;
  const instance_gate_bridge::ResolveResult result =
      instance_gate_bridge::activate_gate(world, state, true, registry, zones);
  check(result.issued_travel, "fresh travel to combat zone");
  check(result.instance.outcome == instance_refresh::Outcome::CreateFresh,
        "fresh instance");
}

void test_checksum_stable() {
  const RuntimePlan plan = build_owner_demo_runtime();
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_owner_demo_runtime_plan();
  test_thornward_combat_zone();
  test_crossroads_town_policy();
  test_gate_travel_via_bridge();
  test_checksum_stable();
  std::cout << "zone_runtime_bridge_tests: " << g_checks << " checks passed\n";
  return 0;
}
