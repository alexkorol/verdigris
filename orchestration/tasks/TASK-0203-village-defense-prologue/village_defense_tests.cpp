// village_defense_tests.cpp — TASK-0203 model-slice acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "village_defense.hpp"

using namespace village_defense;

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
        label + ": expected " + status_name(expected) + " got " +
            status_name(got));
}

void test_full_prologue_arc() {
  State s = make_owner_demo_prologue();
  expect_status(choose_occupation(s, Occupation::FieldHand), Status::Ok,
                "occupation");
  check(s.nudge.strength == 1, "str nudge");
  expect_status(begin_crisis(s), Status::Ok, "crisis");
  expect_status(equip_civilian_tool(s), Status::Ok, "tool");
  expect_status(clear_pack_wave(s), Status::Ok, "pack1");
  expect_status(clear_pack_wave(s), Status::Ok, "pack2");
  check(s.phase == Phase::BossSquareWell, "boss phase");
  expect_status(defeat_boss(s), Status::Ok, "boss");
  check(ready_for_skill_tree(s), "skill tree ready");
  check(s.player_level == kVictoryPlayerLevel, "level 2");
}

void test_forgiving_restart() {
  State s = make_owner_demo_prologue();
  choose_occupation(s, Occupation::Scout);
  begin_crisis(s);
  equip_civilian_tool(s);
  clear_pack_wave(s);
  expect_status(forgiving_restart(s), Status::Ok, "restart");
  check(s.restart_count == 1, "restart count");
  check(s.player_level == 1, "level preserved at 1");
  check(s.phase == Phase::CrisisActive, "back to crisis");
}

void test_boss_gated() {
  State s = make_owner_demo_prologue();
  choose_occupation(s, Occupation::Scribe);
  begin_crisis(s);
  expect_status(defeat_boss(s), Status::BossNotReady, "boss early");
  check(s.nudge.intelligence == 1, "int nudge");
}

}  // namespace

int main() {
  test_full_prologue_arc();
  test_forgiving_restart();
  test_boss_gated();
  std::cout << "village_defense_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
