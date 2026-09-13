// house_progression.hpp — TASK-0200 House versus Scion first-investment model.
//
// Deterministic presentation model for the Owner Demo first-clear reward:
// choose immediate Scion gear or House production/income via Rhea's coffer.
#pragma once

#include <cstdint>

namespace verdigris {

enum class FirstInvestmentChoice : std::uint8_t {
  Unchosen = 0,
  ScionGear,
  HouseProduction,
};

enum class InvestmentStatus : std::uint8_t {
  Ok,
  NotEligible,
  AlreadyChosen,
  InvalidChoice,
};

struct HouseProgressionState {
  FirstInvestmentChoice choice = FirstInvestmentChoice::Unchosen;
  bool first_clear_completed = false;
  bool reward_claimed = false;
  std::uint16_t scion_gear_tier = 0;
  std::uint16_t house_income_per_tick = 0;

  [[nodiscard]] constexpr bool operator==(const HouseProgressionState&) const =
      default;
};

inline constexpr std::uint16_t kScionGearTierFirstClear = 1;
inline constexpr std::uint16_t kHouseIncomePerTickFirstClear = 5;
inline constexpr const char* kHouseCofferFacilityId = "house-coffer";
inline constexpr const char* kCountinghouseNpcId = "rhea-countinghouse";

[[nodiscard]] constexpr const char* investment_status_name(
    InvestmentStatus status) {
  switch (status) {
    case InvestmentStatus::Ok:
      return "Ok";
    case InvestmentStatus::NotEligible:
      return "NotEligible";
    case InvestmentStatus::AlreadyChosen:
      return "AlreadyChosen";
    case InvestmentStatus::InvalidChoice:
      return "InvalidChoice";
  }
  return "Unknown";
}

[[nodiscard]] constexpr const char* choice_name(FirstInvestmentChoice choice) {
  switch (choice) {
    case FirstInvestmentChoice::Unchosen:
      return "unchosen";
    case FirstInvestmentChoice::ScionGear:
      return "scion_gear";
    case FirstInvestmentChoice::HouseProduction:
      return "house_production";
  }
  return "unknown";
}

[[nodiscard]] constexpr bool first_clear_eligible(
    const HouseProgressionState& state) {
  return state.first_clear_completed && !state.reward_claimed &&
         state.choice == FirstInvestmentChoice::Unchosen;
}

[[nodiscard]] constexpr InvestmentStatus mark_first_clear(
    HouseProgressionState& state) {
  state.first_clear_completed = true;
  return InvestmentStatus::Ok;
}

[[nodiscard]] constexpr InvestmentStatus apply_first_investment(
    HouseProgressionState& state, FirstInvestmentChoice choice) {
  if (choice == FirstInvestmentChoice::Unchosen) {
    return InvestmentStatus::InvalidChoice;
  }
  if (!state.first_clear_completed) {
    return InvestmentStatus::NotEligible;
  }
  if (state.reward_claimed || state.choice != FirstInvestmentChoice::Unchosen) {
    return InvestmentStatus::AlreadyChosen;
  }

  state.choice = choice;
  state.reward_claimed = true;
  if (choice == FirstInvestmentChoice::ScionGear) {
    state.scion_gear_tier = kScionGearTierFirstClear;
    state.house_income_per_tick = 0;
  } else {
    state.scion_gear_tier = 0;
    state.house_income_per_tick = kHouseIncomePerTickFirstClear;
  }
  return InvestmentStatus::Ok;
}

[[nodiscard]] constexpr bool grants_immediate_gear(
    const HouseProgressionState& state) {
  return state.reward_claimed &&
         state.choice == FirstInvestmentChoice::ScionGear &&
         state.scion_gear_tier > 0;
}

[[nodiscard]] constexpr bool grants_house_income(
    const HouseProgressionState& state) {
  return state.reward_claimed &&
         state.choice == FirstInvestmentChoice::HouseProduction &&
         state.house_income_per_tick > 0;
}

}  // namespace verdigris
