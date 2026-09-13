// spell_lattice_layout.hpp — TASK-0196 prep: spell lattice panel layout.
//
// Maps spell_lattice Plane-tier nodes to pixel anchors and click targets for
// integrator paint. No main.cpp or GPU in this packet.
#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include "spell_lattice.hpp"

namespace spell_lattice_layout {

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

struct NodeLayout {
  spell_lattice::Node node = spell_lattice::Node::Source;
  PixelPoint center{};
  PixelRect hit{};
  bool in_path = false;
  bool clickable = false;

  [[nodiscard]] constexpr bool operator==(const NodeLayout&) const = default;
};

struct LayoutPlan {
  PixelPoint panel_center{};
  std::array<NodeLayout, static_cast<std::size_t>(
                            spell_lattice::Node::Count)> nodes{};
  std::uint8_t node_count = 0;
  bool elemental_choice_complete = false;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kColSpacing = 56;
inline constexpr std::uint16_t kRowSpacing = 48;
inline constexpr std::uint16_t kNodeRadius = 18;

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

[[nodiscard]] constexpr std::uint8_t row_width(std::uint8_t row) {
  switch (row) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 3;
    case 3:
      return 4;
    case 4:
      return 3;
    case 5:
      return 2;
    case 6:
      return 1;
    default:
      return 0;
  }
}

[[nodiscard]] constexpr PixelRect node_hit_rect(PixelPoint center) {
  PixelRect out;
  out.x = clamp_i16(static_cast<std::int32_t>(center.x) -
                    static_cast<std::int32_t>(kNodeRadius));
  out.y = clamp_i16(static_cast<std::int32_t>(center.y) -
                    static_cast<std::int32_t>(kNodeRadius));
  out.width = static_cast<std::uint16_t>(kNodeRadius * 2);
  out.height = out.width;
  return out;
}

[[nodiscard]] constexpr PixelPoint node_center(PixelPoint panel_center,
                                               std::uint8_t row,
                                               std::uint8_t idx) {
  const std::uint8_t width = row_width(row);
  if (width == 0) return panel_center;
  const std::int32_t row_top =
      static_cast<std::int32_t>(panel_center.y) -
      static_cast<std::int32_t>(kRowSpacing) * 3 +
      static_cast<std::int32_t>(row) * static_cast<std::int32_t>(kRowSpacing);
  const std::int32_t span =
      static_cast<std::int32_t>(width - 1) *
      static_cast<std::int32_t>(kColSpacing);
  const std::int32_t x =
      static_cast<std::int32_t>(panel_center.x) - span / 2 +
      static_cast<std::int32_t>(idx) * static_cast<std::int32_t>(kColSpacing);
  PixelPoint out;
  out.x = clamp_i16(x);
  out.y = clamp_i16(row_top);
  return out;
}

[[nodiscard]] constexpr bool node_in_path(const spell_lattice::State& state,
                                        spell_lattice::Node node) {
  for (std::uint8_t i = 0; i < state.weave_length; ++i) {
    if (state.path[i] == node) {
      return true;
    }
  }
  return false;
}

[[nodiscard]] constexpr bool node_clickable(const spell_lattice::State& state,
                                            spell_lattice::Node node) {
  if (state.weave_length == 0 || node >= spell_lattice::Node::Count) {
    return false;
  }
  if (state.elemental_choice_made &&
      spell_lattice::native_row(node) <= 3) {
    return false;
  }
  if (state.weave_length >= spell_lattice::kStratumCount) {
    return false;
  }
  const spell_lattice::SlotRef prev =
      spell_lattice::slot_for(state.path[state.weave_length - 1]);
  const spell_lattice::SlotRef next = spell_lattice::slot_for(node);
  if (next.row != prev.row + 1) {
    return false;
  }
  return spell_lattice::plane_can_descend(prev.row, prev.idx, next.idx);
}

[[nodiscard]] constexpr LayoutPlan plan_lattice_panel(
    const Viewport& vp, const spell_lattice::State& state) {
  LayoutPlan plan;
  if (!vp.valid()) return plan;

  plan.panel_center.x =
      clamp_i16(static_cast<std::int32_t>(vp.width) / 2);
  plan.panel_center.y =
      clamp_i16(static_cast<std::int32_t>(vp.height) / 2);
  plan.elemental_choice_complete =
      spell_lattice::owner_demo_elemental_choice_complete(state);

  for (spell_lattice::Node node = spell_lattice::Node::Source;
       node < spell_lattice::Node::Count;
       node = static_cast<spell_lattice::Node>(
           static_cast<std::uint8_t>(node) + 1)) {
    const spell_lattice::SlotRef slot = spell_lattice::slot_for(node);
    if (slot.row == 255) continue;
    NodeLayout layout;
    layout.node = node;
    layout.center = node_center(plan.panel_center, slot.row, slot.idx);
    layout.hit = node_hit_rect(layout.center);
    layout.in_path = node_in_path(state, node);
    layout.clickable = node_clickable(state, node);
    plan.nodes[static_cast<std::size_t>(plan.node_count)] = layout;
    ++plan.node_count;
  }

  plan.valid = plan.node_count > 0;
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.node_count) * 17u;
  hash ^= plan.elemental_choice_complete ? 3u : 0u;
  for (std::uint8_t i = 0; i < plan.node_count; ++i) {
    const NodeLayout& node = plan.nodes[static_cast<std::size_t>(i)];
    hash ^= static_cast<std::uint32_t>(node.center.x) * 7u;
    hash ^= static_cast<std::uint32_t>(node.center.y) * 13u;
    hash ^= node.clickable ? 5u : 0u;
  }
  return hash;
}

}  // namespace spell_lattice_layout
