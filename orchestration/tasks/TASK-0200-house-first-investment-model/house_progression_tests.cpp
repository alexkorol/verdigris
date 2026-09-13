// house_progression_tests.cpp — TASK-0200 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include "verdigris/house_progression.hpp"

using namespace verdigris;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void expect_status(InvestmentStatus got, InvestmentStatus expected,
                   const std::string& label) {
  check(got == expected,
        label + ": expected " + investment_status_name(expected) + " got " +
            investment_status_name(got));
}

void test_not_eligible_before_clear() {
  HouseProgressionState s{};
  expect_status(apply_first_investment(s, FirstInvestmentChoice::ScionGear),
                InvestmentStatus::NotEligible, "pre-clear scion");
  expect_status(apply_first_investment(s, FirstInvestmentChoice::HouseProduction),
                InvestmentStatus::NotEligible, "pre-clear house");
}

void test_scion_path() {
  HouseProgressionState s{};
  mark_first_clear(s);
  expect_status(apply_first_investment(s, FirstInvestmentChoice::ScionGear),
                InvestmentStatus::Ok, "scion choice");
  check(grants_immediate_gear(s), "scion grants gear");
  check(s.scion_gear_tier == kScionGearTierFirstClear, "scion tier");
  check(!grants_house_income(s), "no house income on scion");
}

void test_house_path() {
  HouseProgressionState s{};
  mark_first_clear(s);
  expect_status(apply_first_investment(s, FirstInvestmentChoice::HouseProduction),
                InvestmentStatus::Ok, "house choice");
  check(grants_house_income(s), "house grants income");
  check(s.house_income_per_tick == kHouseIncomePerTickFirstClear,
        "house income rate");
  check(!grants_immediate_gear(s), "no scion gear on house");
}

void test_single_choice_and_facility_ids() {
  HouseProgressionState s{};
  mark_first_clear(s);
  apply_first_investment(s, FirstInvestmentChoice::ScionGear);
  expect_status(apply_first_investment(s, FirstInvestmentChoice::HouseProduction),
                InvestmentStatus::AlreadyChosen, "second choice blocked");
  check(std::string(kHouseCofferFacilityId) == "house-coffer", "coffer id");
  check(std::string(kCountinghouseNpcId) == "rhea-countinghouse", "npc id");
}

}  // namespace

int main() {
  test_not_eligible_before_clear();
  test_scion_path();
  test_house_path();
  test_single_choice_and_facility_ids();
  std::cout << "house_progression_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
