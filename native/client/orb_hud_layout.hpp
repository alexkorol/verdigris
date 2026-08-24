// orb_hud_layout.hpp — TASK-0185 prep: WIZARD vital orb HUD layout planner.
//
// Matches production vital-orb geometry from main.cpp and composes
// orb_renderer draw plans from authoritative life/resource ratios. No
// main.cpp, asset loading, or GDI in this packet.
#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

#include "orb_renderer.hpp"

namespace orb_hud_layout {

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

struct VitalStats {
  std::uint16_t life = 0;
  std::uint16_t life_max = 0;
  std::uint16_t resource = 0;
  std::uint16_t resource_max = 0;
  std::uint16_t life_reserved = 0;
  std::uint16_t resource_reserved = 0;

  [[nodiscard]] constexpr bool valid() const {
    return life_max > 0 && resource_max > 0 && life <= life_max &&
           resource <= resource_max && life_reserved <= life_max &&
           resource_reserved <= resource_max;
  }
};

struct OrbAnchor {
  std::int16_t cx = 0;
  std::int16_t cy = 0;
  std::uint16_t radius = 0;
  PixelRect trace_bounds{};

  [[nodiscard]] constexpr bool operator==(const OrbAnchor&) const = default;
};

struct OrbEntry {
  orb_renderer::OrbKind kind = orb_renderer::OrbKind::Life;
  OrbAnchor anchor{};
  orb_renderer::OrbDrawPlan draw{};

  [[nodiscard]] constexpr bool operator==(const OrbEntry&) const = default;
};

struct LayoutPlan {
  OrbEntry life{};
  OrbEntry resource{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kOrbRadius = 34;
inline constexpr std::uint16_t kOrbMargin = 18;
inline constexpr std::uint16_t kPulsePad = 3;

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

[[nodiscard]] constexpr OrbAnchor vital_orb_anchor(const Viewport& vp,
                                                   bool resource_orb) {
  OrbAnchor anchor;
  if (!vp.valid()) return anchor;
  const std::int32_t width = static_cast<std::int32_t>(vp.width);
  const std::int32_t height = static_cast<std::int32_t>(vp.height);
  const std::int32_t radius = static_cast<std::int32_t>(kOrbRadius);
  const std::int32_t cx =
      resource_orb ? width - static_cast<std::int32_t>(kOrbMargin) - radius
                   : static_cast<std::int32_t>(kOrbMargin) + radius;
  const std::int32_t cy =
      height - static_cast<std::int32_t>(kOrbMargin) - radius;
  anchor.cx = clamp_i16(cx);
  anchor.cy = clamp_i16(cy);
  anchor.radius = kOrbRadius;
  const std::int32_t trace_r = radius + static_cast<std::int32_t>(kPulsePad);
  anchor.trace_bounds.x = clamp_i16(cx - trace_r);
  anchor.trace_bounds.y = clamp_i16(cy - trace_r);
  anchor.trace_bounds.width =
      clamp_u16(static_cast<std::int32_t>(trace_r) * 2);
  anchor.trace_bounds.height = anchor.trace_bounds.width;
  return anchor;
}

[[nodiscard]] constexpr PixelRect vital_orb_trace(const Viewport& vp,
                                                bool resource_orb) {
  return vital_orb_anchor(vp, resource_orb).trace_bounds;
}

[[nodiscard]] constexpr orb_renderer::OrbState make_orb_state(
    orb_renderer::OrbKind kind, std::uint16_t current, std::uint16_t max,
    std::uint16_t reserved) {
  orb_renderer::OrbState state;
  state.kind = kind;
  state.current = current;
  state.max = max;
  state.reserved = reserved;
  return state;
}

[[nodiscard]] constexpr LayoutPlan plan_vital_orbs(const Viewport& vp,
                                                   const VitalStats& stats) {
  LayoutPlan plan;
  if (!vp.valid() || !stats.valid()) return plan;

  plan.life.kind = orb_renderer::OrbKind::Life;
  plan.life.anchor = vital_orb_anchor(vp, false);
  plan.life.draw = orb_renderer::plan_orb(
      make_orb_state(orb_renderer::OrbKind::Life, stats.life, stats.life_max,
                     stats.life_reserved));

  plan.resource.kind = orb_renderer::OrbKind::Mana;
  plan.resource.anchor = vital_orb_anchor(vp, true);
  plan.resource.draw = orb_renderer::plan_orb(
      make_orb_state(orb_renderer::OrbKind::Mana, stats.resource,
                     stats.resource_max, stats.resource_reserved));

  plan.valid =
      plan.life.draw.status == orb_renderer::Status::Ok &&
      plan.resource.draw.status == orb_renderer::Status::Ok &&
      plan.life.anchor.trace_bounds.valid() &&
      plan.resource.anchor.trace_bounds.valid();
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.life.anchor.cx) * 17u;
  hash ^= static_cast<std::uint32_t>(plan.resource.anchor.cx) * 31u;
  hash ^= orb_renderer::plan_checksum(plan.life.draw);
  hash ^= orb_renderer::plan_checksum(plan.resource.draw);
  return hash;
}

}  // namespace orb_hud_layout
