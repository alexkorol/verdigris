// gate_overlay_layout.hpp — TASK-0188 prep: zone gate overlay layout planner.
//
// Plans highlight rings and destination label placement from
// gate_interaction hover state. No main.cpp, networking, or GDI in this packet.
#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include "gate_interaction.hpp"

namespace gate_overlay_layout {

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

enum class HighlightKind : std::uint8_t {
  None = 0,
  OutOfRange,
  Highlighted,
  Inaccessible,
};

struct GateOverlay {
  std::uint32_t gate_id = 0;
  std::uint32_t destination_zone = 0;
  HighlightKind highlight = HighlightKind::None;
  PixelRect ring{};
  std::uint16_t radius = 0;
  PixelRect label{};
  bool label_visible = false;
  bool visible = false;

  [[nodiscard]] constexpr bool operator==(const GateOverlay&) const = default;
};

struct LayoutPlan {
  std::array<GateOverlay, gate_interaction::kMaxGates> gates{};
  std::uint8_t count = 0;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kHighlightPad = 6;
inline constexpr std::uint16_t kLabelCharWidth = 8;
inline constexpr std::uint16_t kLabelHeight = 14;
inline constexpr std::uint16_t kLabelGap = 8;

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

[[nodiscard]] constexpr std::uint8_t label_len(const char* label) {
  std::uint8_t n = 0;
  while (n < gate_interaction::kLabelCapacity && label[n] != '\0') {
    ++n;
  }
  return n;
}

[[nodiscard]] constexpr PixelRect circle_bounds(gate_interaction::Point center,
                                                std::uint16_t radius,
                                                std::uint16_t pad) {
  const std::int32_t r =
      static_cast<std::int32_t>(radius) + static_cast<std::int32_t>(pad);
  PixelRect out;
  out.x = clamp_i16(static_cast<std::int32_t>(center.x) - r);
  out.y = clamp_i16(static_cast<std::int32_t>(center.y) - r);
  out.width = clamp_u16(r * 2);
  out.height = out.width;
  return out;
}

[[nodiscard]] constexpr PixelRect label_bounds(gate_interaction::Point center,
                                               std::uint16_t radius,
                                               const char* label) {
  const std::uint8_t len = label_len(label);
  if (len == 0) return {};
  const std::uint16_t w =
      static_cast<std::uint16_t>(len) * kLabelCharWidth;
  PixelRect out;
  out.width = w;
  out.height = kLabelHeight;
  out.x = clamp_i16(static_cast<std::int32_t>(center.x) -
                    static_cast<std::int32_t>(w) / 2);
  out.y = clamp_i16(static_cast<std::int32_t>(center.y) -
                    static_cast<std::int32_t>(radius) -
                    static_cast<std::int32_t>(kLabelGap) -
                    static_cast<std::int32_t>(kLabelHeight));
  return out;
}

[[nodiscard]] constexpr HighlightKind map_hover(gate_interaction::Hover hover) {
  switch (hover) {
    case gate_interaction::Hover::Highlighted:
      return HighlightKind::Highlighted;
    case gate_interaction::Hover::OutOfRange:
      return HighlightKind::OutOfRange;
    case gate_interaction::Hover::Inaccessible:
      return HighlightKind::Inaccessible;
    default:
      return HighlightKind::None;
  }
}

[[nodiscard]] constexpr LayoutPlan plan_gate_overlay(
    const gate_interaction::World& world, const gate_interaction::State& state,
    const Viewport& vp) {
  LayoutPlan plan;
  if (!vp.valid() || world.count == 0) return plan;

  for (std::uint8_t i = 0; i < world.count; ++i) {
    const gate_interaction::GateDef& gate =
        world.gates[static_cast<std::size_t>(i)];
    if (!gate.valid()) continue;

    GateOverlay overlay;
    overlay.gate_id = gate.id;
    overlay.destination_zone = gate.destination_zone;
    overlay.radius = gate.radius;

    const bool hovered =
        state.hovered_index == static_cast<std::int8_t>(i) &&
        state.hover != gate_interaction::Hover::None;
    if (hovered) {
      overlay.visible = true;
      overlay.highlight = map_hover(state.hover);
      overlay.ring = circle_bounds(gate.center, gate.radius, kHighlightPad);
      overlay.label = label_bounds(gate.center, gate.radius, gate.label.data());
      overlay.label_visible = overlay.label.valid();
    }

    plan.gates[static_cast<std::size_t>(plan.count)] = overlay;
    ++plan.count;
  }

  plan.valid = plan.count > 0;
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.count) * 17u;
  for (std::uint8_t i = 0; i < plan.count; ++i) {
    const GateOverlay& overlay =
        plan.gates[static_cast<std::size_t>(i)];
    hash ^= overlay.gate_id * 31u;
    hash ^= static_cast<std::uint32_t>(overlay.highlight) * 7u;
    hash ^= overlay.visible ? 3u : 0u;
    hash ^= static_cast<std::uint32_t>(overlay.ring.x) * 13u;
    hash ^= static_cast<std::uint32_t>(overlay.label.y) * 19u;
  }
  return hash;
}

}  // namespace gate_overlay_layout
