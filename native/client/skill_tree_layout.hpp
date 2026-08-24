// skill_tree_layout.hpp — TASK-0194 prep: geometric skill tree panel layout.
//
// Maps axial hex seats to pixel anchors for integrator paint and hit-testing.
// No main.cpp or GPU in this packet.
#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include "geometric_skill_tree.hpp"

namespace skill_tree_layout {

struct Viewport {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct PixelPoint {
  std::int16_t x = 0;
  std::int16_t y = 0;

  [[nodiscard]] constexpr bool operator==(const PixelPoint&) const = default;
};

struct PixelRect {
  std::int16_t x = 0;
  std::int16_t y = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct SeatLayout {
  geometric_skill_tree::Axial pos{};
  geometric_skill_tree::SeatType type =
      geometric_skill_tree::SeatType::Passive;
  PixelPoint center{};
  PixelRect hit{};
  bool active = false;
  bool clickable = false;

  [[nodiscard]] constexpr bool operator==(const SeatLayout&) const = default;
};

struct LayoutPlan {
  PixelPoint panel_center{};
  std::array<SeatLayout, geometric_skill_tree::kMaxSeats> seats{};
  std::uint8_t seat_count = 0;
  std::uint16_t spendable_points = 0;
  bool tree_unlocked = false;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kCellRadius = 22;
inline constexpr std::uint16_t kHitPad = 6;
inline constexpr std::uint16_t kSqrt3Millis = 1732;

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

[[nodiscard]] constexpr PixelPoint axial_to_pixel(
    geometric_skill_tree::Axial axial, PixelPoint origin,
    std::uint16_t cell_radius = kCellRadius) {
  const std::int32_t x =
      static_cast<std::int32_t>(origin.x) +
      (static_cast<std::int32_t>(cell_radius) * kSqrt3Millis *
       (2 * static_cast<std::int32_t>(axial.q) +
        static_cast<std::int32_t>(axial.r))) / 2000;
  const std::int32_t y =
      static_cast<std::int32_t>(origin.y) +
      (static_cast<std::int32_t>(cell_radius) * 1500 *
       static_cast<std::int32_t>(axial.r)) / 1000;
  PixelPoint out;
  out.x = clamp_i16(x);
  out.y = clamp_i16(y);
  return out;
}

[[nodiscard]] constexpr PixelRect seat_hit_rect(PixelPoint center) {
  const std::int32_t half =
      static_cast<std::int32_t>(kCellRadius) +
      static_cast<std::int32_t>(kHitPad);
  PixelRect out;
  out.x = clamp_i16(static_cast<std::int32_t>(center.x) - half);
  out.y = clamp_i16(static_cast<std::int32_t>(center.y) - half);
  out.width = clamp_u16(half * 2);
  out.height = out.width;
  return out;
}

[[nodiscard]] constexpr bool seat_clickable(
    const geometric_skill_tree::State& state, std::uint8_t seat_index) {
  if (!state.tree_unlocked || seat_index >= state.seat_count) {
    return false;
  }
  const geometric_skill_tree::Seat& seat =
      state.seats[static_cast<std::size_t>(seat_index)];
  if (seat.active) return false;
  if (seat.point_cost > state.skill_points) return false;
  if (seat.type == geometric_skill_tree::SeatType::Sign &&
      state.sign_allocated) {
    return false;
  }
  return geometric_skill_tree::is_adjacent_to_active(state, seat_index);
}

[[nodiscard]] constexpr LayoutPlan plan_level_up_panel(
    const Viewport& vp, const geometric_skill_tree::State& state) {
  LayoutPlan plan;
  if (!vp.valid() || state.seat_count == 0) return plan;

  plan.panel_center.x =
      clamp_i16(static_cast<std::int32_t>(vp.width) / 2);
  plan.panel_center.y =
      clamp_i16(static_cast<std::int32_t>(vp.height) / 2);
  plan.seat_count = state.seat_count;
  plan.spendable_points = state.skill_points;
  plan.tree_unlocked = state.tree_unlocked;

  for (std::uint8_t i = 0; i < state.seat_count; ++i) {
    const geometric_skill_tree::Seat& seat =
        state.seats[static_cast<std::size_t>(i)];
    SeatLayout layout;
    layout.pos = seat.pos;
    layout.type = seat.type;
    layout.active = seat.active;
    layout.center = axial_to_pixel(seat.pos, plan.panel_center);
    layout.hit = seat_hit_rect(layout.center);
    layout.clickable = seat_clickable(state, i);
    plan.seats[static_cast<std::size_t>(i)] = layout;
  }

  plan.valid = true;
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.seat_count) * 17u;
  hash ^= plan.spendable_points * 31u;
  hash ^= plan.tree_unlocked ? 3u : 0u;
  for (std::uint8_t i = 0; i < plan.seat_count; ++i) {
    const SeatLayout& seat = plan.seats[static_cast<std::size_t>(i)];
    hash ^= static_cast<std::uint32_t>(seat.center.x) * 7u;
    hash ^= static_cast<std::uint32_t>(seat.center.y) * 13u;
    hash ^= seat.clickable ? 5u : 0u;
  }
  return hash;
}

}  // namespace skill_tree_layout
