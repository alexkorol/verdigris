// combat_vfx_layout.hpp — TASK-0187 prep: attack VFX stroke layout planner.
//
// Maps attack_vfx primitives to fade-weighted stroke plans for integrator
// paint. Uses stable_render_order; no main.cpp or GPU in this packet.
#pragma once

#include <array>
#include <cstdint>

#include "attack_vfx.hpp"

namespace combat_vfx_layout {

struct Viewport {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct StrokePlan {
  attack_vfx::PrimitiveKind kind = attack_vfx::PrimitiveKind::SwingArc;
  attack_vfx::Point from{};
  attack_vfx::Point to{};
  std::uint16_t radius = 0;
  std::uint16_t alpha = 0;  // 0..1000 fade weight from ticks_left
  std::uint32_t attacker_id = 0;
  std::uint32_t target_id = 0;
  bool clipped = false;

  [[nodiscard]] constexpr bool operator==(const StrokePlan&) const = default;
};

struct LayoutPlan {
  std::array<StrokePlan, attack_vfx::kMaxPrimitives> strokes{};
  std::uint8_t count = 0;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kAlphaScale = 1000;

[[nodiscard]] constexpr std::uint16_t fade_alpha(std::uint16_t ticks_left,
                                                   std::uint16_t ticks_total) {
  if (ticks_total == 0) return 0;
  const std::uint32_t scaled =
      (static_cast<std::uint32_t>(ticks_left) * kAlphaScale) /
      static_cast<std::uint32_t>(ticks_total);
  if (scaled > kAlphaScale) return kAlphaScale;
  return static_cast<std::uint16_t>(scaled);
}

[[nodiscard]] constexpr StrokePlan stroke_from_primitive(
    const attack_vfx::Primitive& prim) {
  StrokePlan stroke;
  stroke.kind = prim.kind;
  stroke.from = prim.origin;
  stroke.to = prim.end;
  stroke.radius = prim.radius;
  stroke.alpha = fade_alpha(prim.ticks_left, prim.ticks_total);
  stroke.attacker_id = prim.attacker_id;
  stroke.target_id = prim.target_id;
  stroke.clipped = prim.clipped;
  return stroke;
}

[[nodiscard]] constexpr LayoutPlan plan_combat_vfx(
    const attack_vfx::Planner& planner, const Viewport& vp) {
  LayoutPlan plan;
  if (!vp.valid()) return plan;

  const std::array<std::uint8_t, attack_vfx::kMaxPrimitives> order =
      attack_vfx::stable_render_order(planner);
  for (std::uint8_t i = 0; i < planner.count; ++i) {
    const attack_vfx::Primitive& prim =
        planner.items[static_cast<std::size_t>(order[i])];
    plan.strokes[static_cast<std::size_t>(plan.count)] =
        stroke_from_primitive(prim);
    ++plan.count;
  }

  plan.valid = plan.count > 0;
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.count) * 17u;
  for (std::uint8_t i = 0; i < plan.count; ++i) {
    const StrokePlan& stroke = plan.strokes[static_cast<std::size_t>(i)];
    hash ^= static_cast<std::uint32_t>(stroke.kind) * 31u;
    hash ^= stroke.alpha * 7u;
    hash ^= static_cast<std::uint32_t>(stroke.from.x) * 13u;
    hash ^= static_cast<std::uint32_t>(stroke.to.y) * 19u;
    hash ^= stroke.attacker_id;
    hash ^= stroke.target_id * 3u;
  }
  return hash;
}

}  // namespace combat_vfx_layout
