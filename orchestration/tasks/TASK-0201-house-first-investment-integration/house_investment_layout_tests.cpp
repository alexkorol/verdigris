// house_investment_layout_tests.cpp — TASK-0201 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "house_investment_layout.hpp"

using namespace house_investment_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_dialog_visible_after_first_clear() {
  verdigris::HouseProgressionState progression{};
  check(verdigris::mark_first_clear(progression) ==
            verdigris::InvestmentStatus::Ok,
        "mark clear");
  const town_runtime_layout::TownLayoutPlan town =
      town_runtime_layout::plan_crossroads(town_runtime_layout::Viewport{
          1920, 1080});
  const LayoutPlan plan =
      plan_investment_dialog(town_runtime_layout::Viewport{1920, 1080},
                             progression, town);
  check(plan.valid, "layout valid");
  check(plan.dialog_visible, "dialog visible");
  check(plan.steward_interact.valid(), "steward interact");
  check(plan.scion_option.enabled, "scion enabled");
  check(plan.house_option.enabled, "house enabled");
  check(plan.dialog_panel.valid(), "dialog panel");
}

void test_hidden_before_clear() {
  verdigris::HouseProgressionState progression{};
  const town_runtime_layout::TownLayoutPlan town =
      town_runtime_layout::plan_crossroads(
          town_runtime_layout::Viewport{1280, 720});
  const LayoutPlan plan =
      plan_investment_dialog(town_runtime_layout::Viewport{1280, 720},
                             progression, town);
  check(plan.valid, "layout valid without dialog");
  check(!plan.dialog_visible, "dialog hidden pre-clear");
}

void test_hidden_after_choice() {
  verdigris::HouseProgressionState progression{};
  check(verdigris::mark_first_clear(progression) ==
            verdigris::InvestmentStatus::Ok,
        "mark clear");
  check(verdigris::apply_first_investment(
            progression, verdigris::FirstInvestmentChoice::HouseProduction) ==
            verdigris::InvestmentStatus::Ok,
        "house chosen");
  const LayoutPlan plan = plan_investment_dialog(
      town_runtime_layout::Viewport{1366, 768}, progression,
      town_runtime_layout::plan_crossroads(
          town_runtime_layout::Viewport{1366, 768}));
  check(!plan.dialog_visible, "dialog hidden after choice");
}

void test_checksum_stable() {
  verdigris::HouseProgressionState progression{};
  check(verdigris::mark_first_clear(progression) ==
            verdigris::InvestmentStatus::Ok,
        "mark clear");
  const LayoutPlan plan = plan_investment_dialog(
      town_runtime_layout::Viewport{1920, 1080}, progression,
      town_runtime_layout::plan_crossroads(
          town_runtime_layout::Viewport{1920, 1080}));
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_dialog_visible_after_first_clear();
  test_hidden_before_clear();
  test_hidden_after_choice();
  test_checksum_stable();
  std::cout << "house_investment_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
