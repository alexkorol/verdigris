// house_investment_layout.hpp — TASK-0201 prep: House vs Scion choice layout.
//
// Plans countinghouse dialog geometry from town steward anchor and
// house_progression eligibility. No main.cpp in this packet.
#pragma once

#include <cstdint>
#include <limits>

#include "../include/verdigris/house_progression.hpp"
#include "town_runtime_layout.hpp"

namespace house_investment_layout {

struct PixelRect {
  std::int16_t x = 0;
  std::int16_t y = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const {
    return width > 0 && height > 0;
  }
};

struct ChoiceLayout {
  verdigris::FirstInvestmentChoice choice =
      verdigris::FirstInvestmentChoice::Unchosen;
  PixelRect bounds{};
  bool enabled = false;

  [[nodiscard]] constexpr bool operator==(const ChoiceLayout&) const = default;
};

struct LayoutPlan {
  PixelRect steward_interact{};
  PixelRect dialog_panel{};
  ChoiceLayout scion_option{};
  ChoiceLayout house_option{};
  bool dialog_visible = false;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kDialogWidth = 320;
inline constexpr std::uint16_t kDialogHeight = 180;
inline constexpr std::uint16_t kChoiceHeight = 44;
inline constexpr std::uint16_t kChoiceGap = 12;
inline constexpr std::uint16_t kDialogMargin = 16;

[[nodiscard]] constexpr std::int16_t clamp_i16(std::int32_t value) {
  if (value < static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::min())) {
    return std::numeric_limits<std::int16_t>::min();
  }
  if (value > static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::max())) {
    return std::numeric_limits<std::int16_t>::max();
  }
  return static_cast<std::int16_t>(value);
}

[[nodiscard]] constexpr const town_runtime_layout::NpcAnchor*
find_steward(const town_runtime_layout::TownLayoutPlan& town) {
  for (std::uint8_t i = 0; i < town.npc_count; ++i) {
    const town_runtime_layout::NpcAnchor& npc =
        town.npcs[static_cast<std::size_t>(i)];
    if (npc.valid &&
        npc.role == town_runtime_layout::NpcRole::Steward) {
      return &npc;
    }
  }
  return nullptr;
}

[[nodiscard]] constexpr LayoutPlan plan_investment_dialog(
    const town_runtime_layout::Viewport& vp,
    const verdigris::HouseProgressionState& progression,
    const town_runtime_layout::TownLayoutPlan& town) {
  LayoutPlan plan;
  if (!vp.valid() || !town.valid) return plan;

  const town_runtime_layout::NpcAnchor* steward = find_steward(town);
  if (steward == nullptr) return plan;

  plan.steward_interact.x = steward->interact.x;
  plan.steward_interact.y = steward->interact.y;
  plan.steward_interact.width = steward->interact.width;
  plan.steward_interact.height = steward->interact.height;
  plan.dialog_visible = verdigris::first_clear_eligible(progression);
  if (!plan.dialog_visible) {
    plan.valid = true;
    return plan;
  }

  const std::int32_t dialog_x =
      static_cast<std::int32_t>(steward->interact.x) -
      static_cast<std::int32_t>(kDialogWidth) / 2;
  const std::int32_t dialog_y =
      static_cast<std::int32_t>(steward->interact.y) -
      static_cast<std::int32_t>(steward->interact.height) -
      static_cast<std::int32_t>(kDialogMargin) -
      static_cast<std::int32_t>(kDialogHeight);

  plan.dialog_panel.x = clamp_i16(dialog_x);
  plan.dialog_panel.y = clamp_i16(dialog_y);
  plan.dialog_panel.width = kDialogWidth;
  plan.dialog_panel.height = kDialogHeight;

  const std::int32_t choice_x =
      static_cast<std::int32_t>(plan.dialog_panel.x) +
      static_cast<std::int32_t>(kDialogMargin);
  const std::int32_t choice_w =
      static_cast<std::int32_t>(kDialogWidth) -
      static_cast<std::int32_t>(kDialogMargin) * 2;
  const std::int32_t scion_y =
      static_cast<std::int32_t>(plan.dialog_panel.y) +
      static_cast<std::int32_t>(kDialogMargin) + 40;
  const std::int32_t house_y = scion_y + static_cast<std::int32_t>(kChoiceHeight) +
                                 static_cast<std::int32_t>(kChoiceGap);

  plan.scion_option.choice = verdigris::FirstInvestmentChoice::ScionGear;
  plan.scion_option.bounds.x = clamp_i16(choice_x);
  plan.scion_option.bounds.y = clamp_i16(scion_y);
  plan.scion_option.bounds.width =
      static_cast<std::uint16_t>(choice_w);
  plan.scion_option.bounds.height = kChoiceHeight;
  plan.scion_option.enabled = true;

  plan.house_option.choice =
      verdigris::FirstInvestmentChoice::HouseProduction;
  plan.house_option.bounds.x = clamp_i16(choice_x);
  plan.house_option.bounds.y = clamp_i16(house_y);
  plan.house_option.bounds.width =
      static_cast<std::uint16_t>(choice_w);
  plan.house_option.bounds.height = kChoiceHeight;
  plan.house_option.enabled = true;

  plan.valid = plan.dialog_panel.valid() && plan.scion_option.bounds.valid() &&
               plan.house_option.bounds.valid();
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= plan.dialog_visible ? 3u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.dialog_panel.x) * 17u;
  hash ^= plan.scion_option.enabled ? 5u : 0u;
  hash ^= plan.house_option.enabled ? 7u : 0u;
  return hash;
}

}  // namespace house_investment_layout
