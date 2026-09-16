// brand_crafting_layout_tests.cpp — TASK-0198 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "brand_crafting_layout.hpp"

using namespace brand_crafting_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_brand_buttons_enabled() {
  brand_crafting::CraftState state =
      brand_crafting::make_owner_demo_craft_state();
  const LayoutPlan plan = plan_brand_workshop(Viewport{1920, 1080}, state);
  check(plan.valid, "layout valid");
  check(plan.brand_buttons[0].enabled, "keen eye enabled");
  check(plan.displayed_coin_cost == brand_crafting::kOwnerDemoBrandCoinCost,
        "coin cost shown");
  check(!plan.socket_enabled, "socket needs fragments");
}

void test_socket_enabled_with_fragments() {
  brand_crafting::CraftState state =
      brand_crafting::make_owner_demo_craft_state();
  brand_crafting::add_trophy_fragment(state.stash,
                                      brand_crafting::kBoarTuskFragmentsRequired);
  const LayoutPlan plan = plan_brand_workshop(Viewport{1280, 720}, state);
  check(plan.socket_enabled, "socket enabled");
}

void test_checksum_stable() {
  const LayoutPlan plan = plan_brand_workshop(
      Viewport{1366, 768}, brand_crafting::make_owner_demo_craft_state());
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
}

}  // namespace

int main() {
  test_brand_buttons_enabled();
  test_socket_enabled_with_fragments();
  test_checksum_stable();
  std::cout << "brand_crafting_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
