// bond_progress_tests.cpp — TASK-0199 model-slice acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "bond_progress.hpp"

using namespace bond_progress;

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

void test_slow_visible_progress() {
  State s = make_owner_demo_attunement(BondTheme::Warding);
  for (int i = 0; i < 3; ++i) {
    record_item_use(s);
  }
  check(shows_slow_incomplete_progress(s), "partial progress visible");
  check(s.display_percent > 0 && s.display_percent < 100, "not complete");
  check(!s.bond_formed, "no bond yet at 3 uses");
}

void test_first_bond_without_maturity() {
  State s = make_owner_demo_attunement(BondTheme::Warding);
  for (int i = 0; i < 6; ++i) {
    record_item_use(s);
  }
  check(s.bond_formed, "bond formed at 360 xp");
  check(s.bond_tier == 1, "tier I only");
  check(owner_demo_stays_immature(s), "still immature");
  check(s.display_percent <= kOwnerDemoMaxDisplayPercent, "capped display");
}

void test_many_uses_stay_immature() {
  State s = make_owner_demo_attunement(BondTheme::Slaughter);
  for (int i = 0; i < 20; ++i) {
    record_item_use(s);
  }
  check(owner_demo_stays_immature(s), "owner demo cap holds");
  check(!s.mature_bond, "no awakening");
}

}  // namespace

int main() {
  test_slow_visible_progress();
  test_first_bond_without_maturity();
  test_many_uses_stay_immature();
  std::cout << "bond_progress_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
