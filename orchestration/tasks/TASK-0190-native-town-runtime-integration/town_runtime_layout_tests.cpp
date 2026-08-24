// town_runtime_layout_tests.cpp — TASK-0190 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "gate_overlay_layout.hpp"
#include "instance_gate_bridge.hpp"
#include "town_runtime_layout.hpp"

using namespace town_runtime_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_crossroads_npc_roles() {
  const Viewport vp{1920, 1080};
  const TownLayoutPlan plan = plan_crossroads(vp);
  check(plan.valid, "layout valid");
  check(plan.npc_count == 4, "four npc anchors");
  check(plan.npcs[0].role == NpcRole::Elder, "elder role");
  check(plan.npcs[1].role == NpcRole::WeaponsToolsTrainer, "weapons role");
  check(plan.npcs[2].role == NpcRole::ArmorRitualMerchant, "armor role");
  check(plan.npcs[3].role == NpcRole::Steward, "steward role");
  check(plan.npcs[0].tile.x == 34, "elder tile x");
  check(plan.crisis_banner, "crisis banner flagged");
}

void test_exit_gates_match_zone_graph() {
  const TownLayoutPlan plan = plan_crossroads(Viewport{1280, 720});
  check(plan.exit_count == 2, "two exits");
  check(plan.exits[0].gate_id == 10u, "thornward gate id");
  check(plan.exits[0].destination_zone == kZoneThornward, "thornward zone hash");
  check(plan.exits[1].gate_id == 11u, "rift gate id");
  check(plan.exits[1].destination_zone == kZoneRiftHollow, "rift zone hash");
}

void test_exit_world_composes_gate_overlay() {
  const TownLayoutPlan plan = plan_crossroads(Viewport{1366, 768});
  const gate_interaction::World world = build_exit_world(plan);
  check(world.count == 2, "two gates in world");

  const gate_interaction::Point player =
      map_pixel_center(plan.exits[0].tile);
  const gate_interaction::State state =
      gate_interaction::update_hover(world, player);
  check(state.hover == gate_interaction::Hover::Highlighted, "player at exit");

  const gate_overlay_layout::LayoutPlan overlay =
      gate_overlay_layout::plan_gate_overlay(
          world, state, gate_overlay_layout::Viewport{1366, 768});
  check(overlay.valid, "overlay valid");
  check(overlay.gates[0].visible, "exit overlay visible");
}

void test_instance_bridge_from_town_exit() {
  const TownLayoutPlan plan = plan_crossroads(Viewport{960, 600});
  const gate_interaction::World world = build_exit_world(plan);
  const gate_interaction::Point player =
      map_pixel_center(plan.exits[0].tile);
  const gate_interaction::State state =
      gate_interaction::update_hover(world, player);

  instance_gate_bridge::ZoneTable zones;
  instance_refresh::ZonePolicy combat;
  combat.zone_id = kZoneThornward;
  combat.kind = instance_refresh::ZoneKind::Combat;
  combat.allow_fresh = true;
  combat.lifetime_ticks = 36000;
  zones.zones[0] = combat;
  zones.count = 1;

  instance_refresh::Registry registry;
  const instance_gate_bridge::ResolveResult result =
      instance_gate_bridge::activate_gate(world, state, false, registry, zones);
  check(result.issued_travel, "travel to thornward");
  check(result.instance.outcome == instance_refresh::Outcome::CreateFresh,
        "fresh combat instance");
}

void test_town_policy_no_fresh() {
  const instance_refresh::ZonePolicy policy = crossroads_town_policy();
  check(policy.zone_id == kZoneCrossroads, "crossroads zone id");
  check(policy.kind == instance_refresh::ZoneKind::Town, "town kind");
  check(!policy.allow_fresh, "town blocks fresh");
}

void test_checksum_stable() {
  const TownLayoutPlan plan = plan_crossroads(Viewport{1920, 1080});
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_crossroads_npc_roles();
  test_exit_gates_match_zone_graph();
  test_exit_world_composes_gate_overlay();
  test_instance_bridge_from_town_exit();
  test_town_policy_no_fresh();
  test_checksum_stable();
  std::cout << "town_runtime_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
