// splash_menu_layout.hpp — TASK-0183 prep: splash + Framekit menu layout planner.
//
// Deterministic pixel layout for WIZARD splash tiers and Framekit panel chrome
// on title and pause surfaces. Consumes manifest composition fractions and
// framekit_renderer nine-slice plans. No main.cpp, asset loading, or GDI.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include "framekit_renderer.hpp"
#include "menu_scene.hpp"

namespace splash_menu_layout {

struct Viewport {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct FractionRect {
  float x = 0.f;
  float y = 0.f;
  float w = 0.f;
  float h = 0.f;

  [[nodiscard]] constexpr bool valid() const { return w > 0.f && h > 0.f; }
};

struct PixelRect {
  std::int16_t x = 0;
  std::int16_t y = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct SplashComposition {
  FractionRect safe_title_region{};
  FractionRect world_disc_center{};

  [[nodiscard]] constexpr bool valid() const {
    return safe_title_region.valid() && world_disc_center.valid();
  }
};

struct LayerPlan {
  PixelRect dest{};
  const char* tier_id = nullptr;
  bool visible = false;

  [[nodiscard]] constexpr bool operator==(const LayerPlan&) const = default;
};

struct MenuChromePlan {
  PixelRect panel{};
  framekit_renderer::NineSlicePlan frame{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const MenuChromePlan&) const = default;
};

struct LayoutPlan {
  LayerPlan background{};
  LayerPlan atmosphere{};
  PixelRect title_safe{};
  PixelRect world_disc_anchor{};
  MenuChromePlan title_panel{};
  MenuChromePlan pause_panel{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

[[nodiscard]] constexpr SplashComposition owner_demo_composition() {
  SplashComposition c;
  c.safe_title_region = {0.15f, 0.08f, 0.7f, 0.22f};
  c.world_disc_center = {0.5f, 0.56f, 0.01f, 0.01f};
  return c;
}

[[nodiscard]] constexpr std::int16_t clamp_i16(std::int32_t value) {
  if (value < static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::min())) {
    return std::numeric_limits<std::int16_t>::min();
  }
  if (value > static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::max())) {
    return std::numeric_limits<std::int16_t>::max();
  }
  return static_cast<std::int16_t>(value);
}

[[nodiscard]] constexpr std::uint16_t clamp_u16(std::int32_t value) {
  if (value <= 0) return 0;
  if (value > static_cast<std::int32_t>(std::numeric_limits<std::uint16_t>::max())) {
    return std::numeric_limits<std::uint16_t>::max();
  }
  return static_cast<std::uint16_t>(value);
}

[[nodiscard]] constexpr PixelRect scale_fraction(const Viewport& vp,
                                                 const FractionRect& fr) {
  PixelRect out;
  if (!vp.valid() || !fr.valid()) return out;
  const std::int32_t x =
      static_cast<std::int32_t>(std::lround(fr.x * static_cast<float>(vp.width)));
  const std::int32_t y =
      static_cast<std::int32_t>(std::lround(fr.y * static_cast<float>(vp.height)));
  const std::int32_t w =
      static_cast<std::int32_t>(std::lround(fr.w * static_cast<float>(vp.width)));
  const std::int32_t h =
      static_cast<std::int32_t>(std::lround(fr.h * static_cast<float>(vp.height)));
  out.x = clamp_i16(x);
  out.y = clamp_i16(y);
  out.width = clamp_u16(w);
  out.height = clamp_u16(h);
  return out;
}

[[nodiscard]] constexpr PixelRect full_background(const Viewport& vp) {
  PixelRect out;
  if (!vp.valid()) return out;
  out.x = 0;
  out.y = 0;
  out.width = vp.width;
  out.height = vp.height;
  return out;
}

[[nodiscard]] constexpr PixelRect atmosphere_band(const Viewport& vp) {
  PixelRect out;
  if (!vp.valid()) return out;
  const std::uint16_t band_h = clamp_u16(
      static_cast<std::int32_t>(vp.height) / 4);
  if (band_h == 0) return out;
  out.x = 0;
  out.y = 0;
  out.width = vp.width;
  out.height = band_h;
  return out;
}

[[nodiscard]] constexpr PixelRect centered_panel(const Viewport& vp,
                                                 std::uint16_t panel_w,
                                                 std::uint16_t panel_h) {
  PixelRect out;
  if (!vp.valid() || panel_w == 0 || panel_h == 0 || panel_w > vp.width ||
      panel_h > vp.height) {
    return out;
  }
  const std::int32_t x =
      static_cast<std::int32_t>(vp.width - panel_w) / 2;
  const std::int32_t y =
      static_cast<std::int32_t>(vp.height - panel_h) / 2;
  out.x = clamp_i16(x);
  out.y = clamp_i16(y);
  out.width = panel_w;
  out.height = panel_h;
  return out;
}

[[nodiscard]] constexpr PixelRect title_panel_rect(const Viewport& vp,
                                                   const PixelRect& title_safe) {
  if (!title_safe.valid()) return {};
  const std::uint16_t pad = 12;
  const std::int32_t inner_w =
      static_cast<std::int32_t>(title_safe.width) -
      static_cast<std::int32_t>(pad) * 2;
  const std::int32_t inner_h =
      static_cast<std::int32_t>(title_safe.height) -
      static_cast<std::int32_t>(pad) * 2;
  if (inner_w <= 0 || inner_h <= 0) return {};
  PixelRect out;
  out.x = clamp_i16(static_cast<std::int32_t>(title_safe.x) + pad);
  out.y = clamp_i16(static_cast<std::int32_t>(title_safe.y) + pad);
  out.width = clamp_u16(inner_w);
  out.height = clamp_u16(inner_h);
  return out;
}

[[nodiscard]] constexpr MenuChromePlan plan_chrome(const PixelRect& panel) {
  MenuChromePlan chrome;
  if (!panel.valid()) return chrome;
  framekit_renderer::Rect dest{
      panel.x, panel.y, panel.width, panel.height};
  chrome.panel = panel;
  chrome.frame = framekit_renderer::plan_nine_slice(
      dest, framekit_renderer::default_panel_asset());
  chrome.valid = chrome.frame.valid;
  return chrome;
}

[[nodiscard]] constexpr LayoutPlan plan_for_root(menu_scene::Root root,
                                                 const Viewport& vp,
                                                 const SplashComposition& comp =
                                                     owner_demo_composition()) {
  LayoutPlan plan;
  if (!vp.valid() || !comp.valid()) return plan;

  plan.background.dest = full_background(vp);
  plan.background.tier_id = "primary";
  plan.background.visible = true;

  plan.atmosphere.dest = atmosphere_band(vp);
  plan.atmosphere.tier_id = "atmosphere";
  plan.atmosphere.visible = true;

  plan.title_safe = scale_fraction(vp, comp.safe_title_region);
  plan.world_disc_anchor = scale_fraction(vp, comp.world_disc_center);

  const std::uint16_t pause_w =
      clamp_u16(static_cast<std::int32_t>(vp.width) * 28 / 100);
  const std::uint16_t pause_h =
      clamp_u16(static_cast<std::int32_t>(vp.height) * 40 / 100);
  const PixelRect pause_rect = centered_panel(vp, pause_w, pause_h);

  switch (root) {
    case menu_scene::Root::Title:
      plan.title_panel = plan_chrome(title_panel_rect(vp, plan.title_safe));
      plan.pause_panel.valid = false;
      plan.valid = plan.title_panel.valid && plan.title_safe.valid();
      return plan;
    case menu_scene::Root::Paused:
      plan.pause_panel = plan_chrome(pause_rect);
      plan.title_panel.valid = false;
      plan.valid = plan.pause_panel.valid;
      return plan;
    case menu_scene::Root::Playing:
      plan.valid = plan.background.visible && plan.title_safe.valid();
      return plan;
  }
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.title_safe.x) * 17u;
  hash ^= static_cast<std::uint32_t>(plan.title_safe.y) * 31u;
  hash ^= plan.title_safe.width * 47u;
  hash ^= plan.title_safe.height * 61u;
  hash ^= framekit_renderer::plan_checksum(plan.title_panel.frame);
  hash ^= framekit_renderer::plan_checksum(plan.pause_panel.frame);
  return hash;
}

}  // namespace splash_menu_layout
