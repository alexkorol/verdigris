// chronicles_owner_pane_layout.hpp — TASK-0197 prep: Chronicles pane layout.
//
// Plans Framekit panel chrome and scroll entry rows for journey pane content.
// No main.cpp in this packet.
#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

#include "chronicles_owner_pane.hpp"
#include "framekit_renderer.hpp"

namespace chronicles_owner_pane_layout {

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

struct EntryRow {
  chronicles_owner_pane::EntryKind kind =
      chronicles_owner_pane::EntryKind::HouseFounded;
  PixelRect row{};
  PixelRect headline{};
  PixelRect detail{};

  [[nodiscard]] constexpr bool operator==(const EntryRow&) const = default;
};

struct LayoutPlan {
  PixelRect panel_outer{};
  framekit_renderer::NineSlicePlan panel_frame{};
  PixelRect scroll_area{};
  std::array<EntryRow, chronicles_owner_pane::kMaxEntries> rows{};
  std::uint8_t row_count = 0;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kPanelWidthPercent = 28;
inline constexpr std::uint16_t kPanelPad = 16;
inline constexpr std::uint16_t kEntryRowHeight = 48;
inline constexpr std::uint16_t kEntryGap = 4;
inline constexpr std::uint16_t kHeadlineHeight = 20;

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

[[nodiscard]] constexpr PixelRect panel_rect(const Viewport& vp,
                                             std::uint8_t entry_count) {
  PixelRect out;
  if (!vp.valid() || entry_count == 0) return out;
  const std::int32_t width =
      static_cast<std::int32_t>(vp.width) * kPanelWidthPercent / 100;
  const std::int32_t content_h =
      static_cast<std::int32_t>(entry_count) *
          static_cast<std::int32_t>(kEntryRowHeight + kEntryGap) +
      static_cast<std::int32_t>(kPanelPad) * 2;
  const std::int32_t max_h = static_cast<std::int32_t>(vp.height) * 45 / 100;
  const std::int32_t height =
      content_h > max_h ? max_h : content_h;
  out.width = clamp_u16(width);
  out.height = clamp_u16(height);
  out.x = clamp_i16((static_cast<std::int32_t>(vp.width) - width) / 2);
  out.y = clamp_i16((static_cast<std::int32_t>(vp.height) - height) / 2);
  return out;
}

[[nodiscard]] constexpr LayoutPlan plan_chronicles_pane(
    const Viewport& vp, const chronicles_owner_pane::PaneState& pane) {
  LayoutPlan plan;
  if (!vp.valid() || pane.count == 0) return plan;

  plan.panel_outer = panel_rect(vp, pane.count);
  if (!plan.panel_outer.valid()) return plan;

  plan.panel_frame =
      framekit_renderer::plan_nine_slice(
          framekit_renderer::Rect{plan.panel_outer.x, plan.panel_outer.y,
                                  plan.panel_outer.width,
                                  plan.panel_outer.height},
          framekit_renderer::default_panel_asset());

  plan.scroll_area.x =
      static_cast<std::int16_t>(plan.panel_outer.x + kPanelPad);
  plan.scroll_area.y =
      static_cast<std::int16_t>(plan.panel_outer.y + kPanelPad);
  plan.scroll_area.width =
      static_cast<std::uint16_t>(plan.panel_outer.width - kPanelPad * 2);
  plan.scroll_area.height =
      static_cast<std::uint16_t>(plan.panel_outer.height - kPanelPad * 2);

  for (std::uint8_t i = 0; i < pane.count; ++i) {
    EntryRow row;
    row.kind = pane.entries[i].kind;
    row.row.x = plan.scroll_area.x;
    row.row.y = clamp_i16(static_cast<std::int32_t>(plan.scroll_area.y) +
                          static_cast<std::int32_t>(i) *
                              static_cast<std::int32_t>(kEntryRowHeight +
                                                        kEntryGap));
    row.row.width = plan.scroll_area.width;
    row.row.height = kEntryRowHeight;
    row.headline = row.row;
    row.headline.height = kHeadlineHeight;
    row.detail = row.row;
    row.detail.y =
        static_cast<std::int16_t>(row.row.y + kHeadlineHeight);
    row.detail.height =
        static_cast<std::uint16_t>(kEntryRowHeight - kHeadlineHeight);
    plan.rows[static_cast<std::size_t>(plan.row_count)] = row;
    ++plan.row_count;
  }

  plan.valid = plan.panel_frame.valid && plan.row_count == pane.count;
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.row_count) * 17u;
  hash ^= static_cast<std::uint32_t>(plan.panel_outer.x) * 31u;
  hash ^= framekit_renderer::plan_checksum(plan.panel_frame);
  return hash;
}

}  // namespace chronicles_owner_pane_layout
