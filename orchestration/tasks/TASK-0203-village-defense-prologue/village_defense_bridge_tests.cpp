// village_defense_bridge_tests.cpp — TASK-0203 bridge tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "village_defense_bridge.hpp"

using namespace village_defense_bridge;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_prologue_zone_entry() {
  const zone_runtime_bridge::RuntimePlan plan =
      zone_runtime_bridge::build_owner_demo_runtime();
  const zone_runtime_bridge::ZoneRuntimeEntry& entry = prologue_zone(plan);
  check(entry.valid, "prologue zone valid");
  check(!entry.policy.allow_fresh, "prologue no fresh");
  check(entry.gates.count == 1u, "one prologue gate");
}

void test_prologue_beat_to_boss() {
  village_defense::State defense =
      village_defense::make_owner_demo_prologue();
  check(run_prologue_beat(defense, village_defense::Occupation::Scout) ==
            village_defense::Status::Ok,
        "prologue beat");
  check(defense.phase == village_defense::Phase::BossSquareWell, "boss ready");
  check(prologue_combat_active(defense), "combat active at boss");
}

void test_boss_unlocks_skill_tree() {
  village_defense::State defense =
      village_defense::make_owner_demo_prologue();
  check(run_prologue_beat(defense, village_defense::Occupation::FieldHand) ==
            village_defense::Status::Ok,
        "reach boss");
  geometric_skill_tree::State tree =
      geometric_skill_tree::make_owner_demo_first_level_slice();
  const BossVictoryResult result = on_boss_defeated(defense, tree);
  check(result.defense_status == village_defense::Status::Ok, "boss defeated");
  check(result.skill_tree_unlocked, "tree unlocked");
  check(tree.skill_points == geometric_skill_tree::kOwnerDemoFirstLevelPoints,
        "first level point");
}

}  // namespace

int main() {
  test_prologue_zone_entry();
  test_prologue_beat_to_boss();
  test_boss_unlocks_skill_tree();
  std::cout << "village_defense_bridge_tests: " << g_checks << " checks passed\n";
  return 0;
}
