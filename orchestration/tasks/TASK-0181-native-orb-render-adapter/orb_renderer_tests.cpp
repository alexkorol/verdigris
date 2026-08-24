// orb_renderer_tests.cpp — TASK-0181 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "orb_renderer.hpp"

using namespace orb_renderer;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_full_life_orb() {
  OrbState state;
  state.kind = OrbKind::Life;
  state.current = 900;
  state.max = 1000;
  const OrbDrawPlan plan = plan_orb(state);
  check(plan.status == Status::Ok, "valid life");
  check(plan.band == DepletionBand::Full, "full band");
  check(plan.layers[0].active, "color plate active");
  check(!plan.layers[4].active, "no stone without reserve");
  check(!plan.pulse_low, "no pulse at full");
}

void test_low_mana_pulse() {
  OrbState state;
  state.kind = OrbKind::Mana;
  state.current = 150;
  state.max = 1000;
  const OrbDrawPlan plan = plan_orb(state);
  check(plan.band == DepletionBand::Low, "low band");
  check(plan.pulse_low, "pulse on low");
  check(plan.layers[0].active, "liquid still visible on low");
}

void test_reserved_stone_layer() {
  OrbState state;
  state.current = 800;
  state.max = 1000;
  state.reserved = 300;
  const OrbDrawPlan plan = plan_orb(state);
  check(plan.reserve_ratio == 300, "reserve ratio");
  check(plan.layers[4].active, "stone layer active");
  check(plan.layers[4].weight == 300, "stone weight");
}

void test_empty_orb() {
  OrbState state;
  state.current = 0;
  state.max = 500;
  const OrbDrawPlan plan = plan_orb(state);
  check(plan.band == DepletionBand::Empty, "empty band");
  check(!plan.layers[0].active, "no liquid color");
  check(plan.layers[5].active, "empty glass");
}

void test_invalid_state() {
  OrbState state;
  state.current = 10;
  state.max = 0;
  check(plan_orb(state).status == Status::Invalid, "invalid max");
}

void test_deterministic_checksum() {
  OrbState state;
  state.current = 640;
  state.max = 800;
  state.reserved = 100;
  const std::uint32_t a = plan_checksum(plan_orb(state));
  const std::uint32_t b = plan_checksum(plan_orb(state));
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_full_life_orb();
  test_low_mana_pulse();
  test_reserved_stone_layer();
  test_empty_orb();
  test_invalid_state();
  test_deterministic_checksum();

  std::cout << "TASK-0181 orb renderer acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
