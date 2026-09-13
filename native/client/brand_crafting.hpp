// brand_crafting.hpp — TASK-0198 first Brand crafting loop model.
//
// Deliberate Brand application using coins (atelier service) or completed
// trophy fragments (boar tusk). Mirrors WIZARD sear/socket without RNG in
// this Owner Demo slice.
#pragma once

#include <cstdint>

namespace brand_crafting {

inline constexpr std::uint16_t kOwnerDemoBrandCoinCost = 100;
inline constexpr std::uint8_t kBoarTuskFragmentsRequired = 5;
inline constexpr std::uint8_t kOwnerDemoMaxBrands = 1;

enum class DeliberateBrand : std::uint8_t {
  KeenEye = 0,
  Wealthy,
  Beastbane,
};

enum class Status : std::uint8_t {
  Ok,
  NoVessel,
  NoFreeSlot,
  InsufficientCoins,
  InsufficientFragments,
  TrophyAlreadySocketed,
  BrandLimitReached,
  InvalidBrand,
};

struct ItemVessel {
  bool has_vessel = true;
  std::uint8_t vessel_slots = 2;
  std::uint8_t brand_count = 0;
  std::uint8_t trophy_count = 0;
  std::uint8_t patience = 3;

  [[nodiscard]] constexpr std::uint8_t free_slots() const {
    if (!has_vessel) {
      return 0;
    }
    const std::uint8_t used =
        static_cast<std::uint8_t>(brand_count + trophy_count);
    return vessel_slots > used ? static_cast<std::uint8_t>(vessel_slots - used)
                               : 0;
  }

  [[nodiscard]] constexpr bool operator==(const ItemVessel&) const = default;
};

struct TrophyStash {
  std::uint8_t boar_tusk_fragments = 0;
  bool boar_tusk_socketed = false;

  [[nodiscard]] constexpr bool operator==(const TrophyStash&) const = default;
};

struct CraftState {
  ItemVessel item{};
  TrophyStash stash{};
  std::uint16_t coins = 0;
  DeliberateBrand applied_brand = DeliberateBrand::KeenEye;
  bool brand_applied = false;

  [[nodiscard]] constexpr bool operator==(const CraftState&) const = default;
};

[[nodiscard]] constexpr const char* status_name(Status status) {
  switch (status) {
    case Status::Ok:
      return "Ok";
    case Status::NoVessel:
      return "NoVessel";
    case Status::NoFreeSlot:
      return "NoFreeSlot";
    case Status::InsufficientCoins:
      return "InsufficientCoins";
    case Status::InsufficientFragments:
      return "InsufficientFragments";
    case Status::TrophyAlreadySocketed:
      return "TrophyAlreadySocketed";
    case Status::BrandLimitReached:
      return "BrandLimitReached";
    case Status::InvalidBrand:
      return "InvalidBrand";
  }
  return "Unknown";
}

[[nodiscard]] constexpr const char* brand_label(DeliberateBrand brand) {
  switch (brand) {
    case DeliberateBrand::KeenEye:
      return "+22% Critical Chance";
    case DeliberateBrand::Wealthy:
      return "+24% Goods Found";
    case DeliberateBrand::Beastbane:
      return "+13% Damage against Beasts";
  }
  return "unknown";
}

[[nodiscard]] constexpr CraftState make_owner_demo_craft_state() {
  CraftState s{};
  s.coins = 200;
  s.item = ItemVessel{true, 2, 0, 0, 3};
  return s;
}

[[nodiscard]] constexpr Status add_trophy_fragment(TrophyStash& stash,
                                                 std::uint8_t count = 1) {
  stash.boar_tusk_fragments =
      static_cast<std::uint8_t>(stash.boar_tusk_fragments + count);
  return Status::Ok;
}

[[nodiscard]] constexpr Status socket_boar_tusk(CraftState& state) {
  if (!state.item.has_vessel) {
    return Status::NoVessel;
  }
  if (state.stash.boar_tusk_socketed) {
    return Status::TrophyAlreadySocketed;
  }
  if (state.stash.boar_tusk_fragments < kBoarTuskFragmentsRequired) {
    return Status::InsufficientFragments;
  }
  if (state.item.free_slots() == 0) {
    return Status::NoFreeSlot;
  }
  state.stash.boar_tusk_fragments =
      static_cast<std::uint8_t>(
          state.stash.boar_tusk_fragments - kBoarTuskFragmentsRequired);
  state.stash.boar_tusk_socketed = true;
  state.item.trophy_count = 1;
  return Status::Ok;
}

[[nodiscard]] constexpr Status apply_deliberate_brand(CraftState& state,
                                                      DeliberateBrand brand) {
  if (!state.item.has_vessel) {
    return Status::NoVessel;
  }
  if (state.brand_applied ||
      state.item.brand_count >= kOwnerDemoMaxBrands) {
    return Status::BrandLimitReached;
  }
  if (state.item.free_slots() == 0) {
    return Status::NoFreeSlot;
  }
  if (state.item.patience == 0) {
    return Status::NoFreeSlot;
  }
  if (state.coins < kOwnerDemoBrandCoinCost) {
    return Status::InsufficientCoins;
  }

  state.coins =
      static_cast<std::uint16_t>(state.coins - kOwnerDemoBrandCoinCost);
  state.item.brand_count = 1;
  state.item.patience =
      static_cast<std::uint8_t>(state.item.patience - 1);
  state.applied_brand = brand;
  state.brand_applied = true;
  return Status::Ok;
}

}  // namespace brand_crafting
