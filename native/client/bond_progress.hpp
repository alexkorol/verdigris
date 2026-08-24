// bond_progress.hpp — TASK-0199 early Bond progress visibility model.
//
// Slow, incomplete attunement toward a first-tier Bond from item use. Owner Demo
// deliberately caps before mature awakening (mirrors WIZARD attune/evolve loop).
#pragma once

#include <cstdint>

namespace bond_progress {

inline constexpr std::uint16_t kOwnerDemoXpPerUse = 60;
inline constexpr std::uint16_t kXpToFirstBond = 360;
inline constexpr std::uint16_t kXpPerAdditionalTier = 600;
inline constexpr std::uint8_t kMaxBondTier = 3;
inline constexpr std::uint8_t kOwnerDemoVisibleTierCap = 1;
inline constexpr std::uint8_t kOwnerDemoMaxDisplayPercent = 72;

enum class BondTheme : std::uint8_t {
  None = 0,
  Warding,
  Slaughter,
  Fortune,
};

enum class Status : std::uint8_t {
  Ok,
  AlreadyMature,
  InvalidUse,
};

struct State {
  std::uint16_t attunement_xp = 0;
  std::uint8_t use_count = 0;
  bool bond_formed = false;
  std::uint8_t bond_tier = 0;
  BondTheme theme = BondTheme::None;
  bool mature_bond = false;
  std::uint8_t display_percent = 0;

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

[[nodiscard]] constexpr const char* status_name(Status status) {
  switch (status) {
    case Status::Ok:
      return "Ok";
    case Status::AlreadyMature:
      return "AlreadyMature";
    case Status::InvalidUse:
      return "InvalidUse";
  }
  return "Unknown";
}

[[nodiscard]] constexpr State make_owner_demo_attunement(BondTheme theme) {
  State s{};
  s.theme = theme;
  return s;
}

[[nodiscard]] constexpr std::uint8_t compute_display_percent(const State& state) {
  if (state.mature_bond) {
    return 100;
  }
  if (!state.bond_formed) {
    const std::uint32_t pct =
        (static_cast<std::uint32_t>(state.attunement_xp) * 100u) /
        kXpToFirstBond;
    return static_cast<std::uint8_t>(
        pct > kOwnerDemoMaxDisplayPercent ? kOwnerDemoMaxDisplayPercent
                                          : pct);
  }
  if (state.bond_tier >= kOwnerDemoVisibleTierCap) {
    const std::uint16_t into_next =
        static_cast<std::uint16_t>(state.attunement_xp - kXpToFirstBond);
    const std::uint32_t pct =
        40u + (static_cast<std::uint32_t>(into_next) * 32u) /
                  kXpPerAdditionalTier;
    const std::uint8_t capped =
        static_cast<std::uint8_t>(pct > kOwnerDemoMaxDisplayPercent
                                      ? kOwnerDemoMaxDisplayPercent
                                      : pct);
    return capped < 40 ? 40 : capped;
  }
  return kOwnerDemoMaxDisplayPercent;
}

[[nodiscard]] constexpr bool shows_slow_incomplete_progress(
    const State& state) {
  return state.use_count > 0 && !state.mature_bond &&
         state.display_percent > 0 &&
         state.display_percent < 100;
}

[[nodiscard]] constexpr Status record_item_use(State& state,
                                               std::uint16_t xp_gain =
                                                   kOwnerDemoXpPerUse) {
  if (state.mature_bond) {
    return Status::AlreadyMature;
  }
  if (xp_gain == 0) {
    return Status::InvalidUse;
  }
  state.use_count =
      static_cast<std::uint8_t>(state.use_count + 1);
  state.attunement_xp =
      static_cast<std::uint16_t>(state.attunement_xp + xp_gain);

  if (!state.bond_formed && state.attunement_xp >= kXpToFirstBond) {
    state.bond_formed = true;
    state.bond_tier = 1;
  } else if (state.bond_formed &&
             state.bond_tier < kOwnerDemoVisibleTierCap &&
             state.attunement_xp >=
                 static_cast<std::uint16_t>(kXpToFirstBond + kXpPerAdditionalTier)) {
    state.bond_tier =
        static_cast<std::uint8_t>(state.bond_tier + 1);
  }

  if (state.bond_tier >= kMaxBondTier && state.bond_formed) {
    state.mature_bond = true;
  }

  state.display_percent = compute_display_percent(state);
  return Status::Ok;
}

[[nodiscard]] constexpr bool owner_demo_stays_immature(const State& state) {
  return !state.mature_bond &&
         state.bond_tier <= kOwnerDemoVisibleTierCap;
}

}  // namespace bond_progress
