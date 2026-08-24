// instance_gate_bridge_tests.cpp — TASK-0189 bridge tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "instance_gate_bridge.hpp"

using namespace instance_gate_bridge;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

gate_interaction::GateDef make_gate(std::uint32_t id, std::uint32_t zone) {
  gate_interaction::GateDef gate;
  gate.id = id;
  gate.destination_zone = zone;
  gate.center = {200, 200};
  gate.radius = 40;
  gate.accessible = true;
  gate.label[0] = 'G';
  return gate;
}

instance_refresh::ZonePolicy combat_zone(std::uint32_t zone_id) {
  instance_refresh::ZonePolicy zone;
  zone.zone_id = zone_id;
  zone.kind = instance_refresh::ZoneKind::Combat;
  zone.allow_fresh = true;
  zone.lifetime_ticks = 1000;
  return zone;
}

instance_refresh::ZonePolicy town_zone(std::uint32_t zone_id) {
  instance_refresh::ZonePolicy zone;
  zone.zone_id = zone_id;
  zone.kind = instance_refresh::ZoneKind::Town;
  zone.allow_fresh = false;
  return zone;
}

void test_enter_zone_reuses_instance() {
  gate_interaction::World world;
  world.gates[0] = make_gate(1, 42);
  world.count = 1;

  ZoneTable zones;
  zones.zones[0] = combat_zone(42);
  zones.count = 1;

  instance_refresh::Registry registry;
  const gate_interaction::State state =
      gate_interaction::update_hover(world, {200, 200});

  const ResolveResult first =
      activate_gate(world, state, false, registry, zones);
  check(first.issued_travel, "first travel issued");
  check(first.instance.outcome == instance_refresh::Outcome::CreateFresh,
        "fresh on first entry");
  const std::uint32_t first_id = first.instance.instance_id;

  const ResolveResult second =
      activate_gate(world, state, false, registry, zones);
  check(second.issued_travel, "reuse travel issued");
  check(second.instance.outcome == instance_refresh::Outcome::ReuseExisting,
        "reuse existing");
  check(second.instance.instance_id == first_id, "same instance id");
}

void test_ctrl_click_fresh_instance() {
  gate_interaction::World world;
  world.gates[0] = make_gate(2, 77);
  world.count = 1;

  ZoneTable zones;
  zones.zones[0] = combat_zone(77);
  zones.count = 1;

  instance_refresh::Registry registry;
  const gate_interaction::State state =
      gate_interaction::update_hover(world, {200, 200});

  const ResolveResult first =
      activate_gate(world, state, false, registry, zones);
  const std::uint32_t first_id = first.instance.instance_id;

  const ResolveResult fresh =
      activate_gate(world, state, true, registry, zones);
  check(fresh.gate.command == gate_interaction::Command::FreshInstance,
        "fresh command");
  check(fresh.instance.outcome == instance_refresh::Outcome::CreateFresh,
        "fresh instance created");
  check(fresh.instance.instance_id != first_id, "new instance id");
  check(fresh.issued_travel, "travel on fresh");
}

void test_town_rejects_fresh() {
  gate_interaction::World world;
  world.gates[0] = make_gate(3, 5);
  world.count = 1;

  ZoneTable zones;
  zones.zones[0] = town_zone(5);
  zones.count = 1;

  instance_refresh::Registry registry;
  const gate_interaction::State state =
      gate_interaction::update_hover(world, {200, 200});

  const ResolveResult result =
      activate_gate(world, state, true, registry, zones);
  check(result.instance.outcome == instance_refresh::Outcome::RejectedTownFresh,
        "town rejects fresh");
  check(!result.issued_travel, "no travel on rejection");
  check(result.instance.message ==
            instance_refresh::MessageCode::TownNoFresh,
        "town message");
}

void test_out_of_range_no_travel() {
  gate_interaction::World world;
  world.gates[0] = make_gate(4, 90);
  world.count = 1;

  ZoneTable zones;
  zones.zones[0] = combat_zone(90);
  zones.count = 1;

  instance_refresh::Registry registry;
  const gate_interaction::State state =
      gate_interaction::update_hover(world, {250, 200});

  const ResolveResult result =
      activate_gate(world, state, false, registry, zones);
  check(result.gate.status == gate_interaction::Status::OutOfRange,
        "gate out of range");
  check(!result.issued_travel, "no travel");
}

}  // namespace

int main() {
  test_enter_zone_reuses_instance();
  test_ctrl_click_fresh_instance();
  test_town_rejects_fresh();
  test_out_of_range_no_travel();
  std::cout << "instance_gate_bridge_tests: " << g_checks << " checks passed\n";
  return 0;
}
