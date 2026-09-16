// brand_crafting_layout.hpp — TASK-0198 prep: Brand crafting workshop layout.
//
// Plans atelier panel geometry and enabled brand/socket actions from craft
// state. No main.cpp in this packet.
#pragma once

#include <cstdint>
#include <limits>

#include "brand_crafting.hpp"
#include "framekit_renderer.hpp"

namespace brand_crafting_layout {

struct Viewport {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct PixelRect {
  std::int16_t x = 0;
  std::int16_t y = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct BrandButton {
  brand_crafting::DeliberateBrand brand =
      brand_crafting::DeliberateBrand::KeenEye;
  PixelRect bounds{};
  bool enabled = false;

  [[nodiscard]] constexpr bool operator==(const BrandButton&) const = default;
};

struct LayoutPlan {
  PixelRect workshop_panel{};
  framekit_renderer::NineSlicePlan panel_frame{};
  std::array<BrandButton, 3> brand_buttons{};
  PixelRect socket_button{};
  bool socket_enabled = false;
  std::uint16_t displayed_coin_cost = 0;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kPanelWidth = 300;
inline constexpr std::uint16_t kPanelHeight = 220;
inline constexpr std::uint16_t kButtonHeight = 40;
inline constexpr std::uint16_t kButtonGap = 8;
inline constexpr std::uint16_t kPanelMargin = 18;

[[nodiscard]] constexpr std::int16_t clamp_i16(std::int32_t value) {
  if (value < static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::min())) {
    return std::numeric_limits<std::int16_t>::min();
  }
  if (value > static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::max())) {
    return std::numeric_limits<std::int16_t>::max();
  }
  return static_cast<std::int16_t>(value);
}

[[nodiscard]] constexpr bool brand_enabled(const brand_crafting::CraftState& state,
                                           brand_crafting::DeliberateBrand brand) {
  brand_crafting::CraftState probe = state;
  return brand_crafting::apply_deliberate_brand(probe, brand) ==
         brand_crafting::Status::Ok;
}

[[nodiscard]] constexpr bool socket_enabled(const brand_crafting::CraftState& state) {
  brand_crafting::CraftState probe = state;
  return brand_crafting::socket_boar_tusk(probe) == brand_crafting::Status::Ok;
}

[[nodiscard]] constexpr LayoutPlan plan_brand_workshop(
    const Viewport& vp, const brand_crafting::CraftState& state) {
  LayoutPlan plan;
  if (!vp.valid()) return plan;

  plan.workshop_panel.x =
      clamp_i16(static_cast<std::int32_t>(vp.width) -
                static_cast<std::int32_t>(kPanelWidth) -
                static_cast<std::int32_t>(kPanelMargin));
  plan.workshop_panel.y =
      clamp_i16(static_cast<std::int32_t>(vp.height) / 4);
  plan.workshop_panel.width = kPanelWidth;
  plan.workshop_panel.height = kPanelHeight;

  plan.panel_frame =
      framekit_renderer::plan_nine_slice(
          framekit_renderer::Rect{plan.workshop_panel.x, plan.workshop_panel.y,
                                  plan.workshop_panel.width,
                                  plan.workshop_panel.height},
          framekit_renderer::default_panel_asset());

  const std::int32_t button_x =
      static_cast<std::int32_t>(plan.workshop_panel.x) + 16;
  const std::int32_t button_w =
      static_cast<std::int32_t>(kPanelWidth) - 32;
  std::int32_t button_y =
      static_cast<std::int32_t>(plan.workshop_panel.y) + 48;

  const brand_crafting::DeliberateBrand brands[] = {
      brand_crafting::DeliberateBrand::KeenEye,
      brand_crafting::DeliberateBrand::Wealthy,
      brand_crafting::DeliberateBrand::Beastbane,
  };
  for (std::uint8_t i = 0; i < 3; ++i) {
    plan.brand_buttons[static_cast<std::size_t>(i)].brand = brands[i];
    plan.brand_buttons[static_cast<std::size_t>(i)].bounds.x =
        clamp_i16(button_x);
    plan.brand_buttons[static_cast<std::size_t>(i)].bounds.y =
        clamp_i16(button_y);
    plan.brand_buttons[static_cast<std::size_t>(i)].bounds.width =
        static_cast<std::uint16_t>(button_w);
    plan.brand_buttons[static_cast<std::size_t>(i)].bounds.height =
        kButtonHeight;
    plan.brand_buttons[static_cast<std::size_t>(i)].enabled =
        brand_enabled(state, brands[i]);
    button_y += static_cast<std::int32_t>(kButtonHeight + kButtonGap);
  }

  plan.socket_button.x = clamp_i16(button_x);
  plan.socket_button.y = clamp_i16(button_y);
  plan.socket_button.width = static_cast<std::uint16_t>(button_w);
  plan.socket_button.height = kButtonHeight;
  plan.socket_enabled = socket_enabled(state);
  plan.displayed_coin_cost = brand_crafting::kOwnerDemoBrandCoinCost;

  plan.valid = plan.panel_frame.valid && plan.workshop_panel.valid();
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.workshop_panel.x) * 17u;
  hash ^= plan.socket_enabled ? 3u : 0u;
  hash ^= plan.displayed_coin_cost * 7u;
  hash ^= framekit_renderer::plan_checksum(plan.panel_frame);
  return hash;
}

}  // namespace brand_crafting_layout
