// brand_crafting_tests.cpp — TASK-0198 model-slice acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "brand_crafting.hpp"

using namespace brand_crafting;

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

void test_coin_brand_application() {
  CraftState s = make_owner_demo_craft_state();
  expect_status(apply_deliberate_brand(s, DeliberateBrand::KeenEye), Status::Ok,
                "apply keen eye");
  check(s.coins == 100, "spent 100 coins");
  check(s.brand_applied, "brand applied");
  check(std::string(brand_label(s.applied_brand)).find("Critical") !=
            std::string::npos,
        "keen eye label");
  expect_status(apply_deliberate_brand(s, DeliberateBrand::Wealthy),
                Status::BrandLimitReached, "one brand cap");
}

void test_trophy_socket_path() {
  CraftState s = make_owner_demo_craft_state();
  for (int i = 0; i < 5; ++i) {
    add_trophy_fragment(s.stash);
  }
  expect_status(socket_boar_tusk(s), Status::Ok, "socket tusk");
  check(s.item.trophy_count == 1, "trophy socketed");
  check(s.stash.boar_tusk_fragments == 0, "fragments spent");
}

void test_insufficient_coins() {
  CraftState s = make_owner_demo_craft_state();
  s.coins = 50;
  expect_status(apply_deliberate_brand(s, DeliberateBrand::Beastbane),
                Status::InsufficientCoins, "need coins");
}

}  // namespace

int main() {
  test_coin_brand_application();
  test_trophy_socket_path();
  test_insufficient_coins();
  std::cout << "brand_crafting_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
