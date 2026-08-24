// inventory_pane_layout.hpp — TASK-0184 prep: Framekit gear pane layout planner.
//
// Plans WIZARD Framekit panel chrome, paper-doll slot grid, 12x6 backpack
// cells, and item-art blits from inventory_grid + paper_doll models. No
// main.cpp, asset loading, or GDI in this packet.
#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

#include "framekit_renderer.hpp"
#include "inventory_grid.hpp"
#include "item_art_renderer.hpp"
#include "paper_doll.hpp"

namespace inventory_pane_layout {

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

struct PaneChrome {
  PixelRect outer{};
  framekit_renderer::NineSlicePlan frame{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const PaneChrome&) const = default;
};

struct GridPlan {
  PixelRect area{};
  std::uint8_t columns = 0;
  std::uint8_t rows = 0;
  std::uint8_t cell_w = 0;
  std::uint8_t cell_h = 0;
  std::uint8_t gap = 2;

  [[nodiscard]] constexpr bool valid() const {
    return area.valid() && columns > 0 && rows > 0 && cell_w > 0 && cell_h > 0;
  }

  [[nodiscard]] constexpr PixelRect cell_rect(std::uint8_t col,
                                              std::uint8_t row) const {
    PixelRect out;
    if (!valid() || col >= columns || row >= rows) return out;
    out.x = static_cast<std::int16_t>(
        area.x + static_cast<std::int16_t>(col) *
                     static_cast<std::int16_t>(cell_w + gap));
    out.y = static_cast<std::int16_t>(
        area.y + static_cast<std::int16_t>(row) *
                     static_cast<std::int16_t>(cell_h + gap));
    out.width = cell_w;
    out.height = cell_h;
    return out;
  }

  [[nodiscard]] constexpr PixelRect footprint_rect(std::uint8_t col,
                                                   std::uint8_t row,
                                                   std::uint8_t span_w,
                                                   std::uint8_t span_h) const {
    PixelRect out;
    if (!valid() || span_w == 0 || span_h == 0 || col + span_w > columns ||
        row + span_h > rows) {
      return out;
    }
    out.x = static_cast<std::int16_t>(
        area.x + static_cast<std::int16_t>(col) *
                     static_cast<std::int16_t>(cell_w + gap));
    out.y = static_cast<std::int16_t>(
        area.y + static_cast<std::int16_t>(row) *
                     static_cast<std::int16_t>(cell_h + gap));
    const std::uint16_t w =
        static_cast<std::uint16_t>(span_w * cell_w + (span_w - 1) * gap);
    const std::uint16_t h =
        static_cast<std::uint16_t>(span_h * cell_h + (span_h - 1) * gap);
    out.width = w;
    out.height = h;
    return out;
  }
};

struct DollSlotPlan {
  paper_doll::Slot slot = paper_doll::Slot::Head;
  PixelRect rect{};
  framekit_renderer::NineSlicePlan slot_frame{};
  item_art_renderer::SpriteBlit equipped_blit{};
  bool has_equipped = false;

  [[nodiscard]] constexpr bool operator==(const DollSlotPlan&) const = default;
};

struct ItemBlitPlan {
  std::uint32_t item_id = 0;
  PixelRect footprint{};
  item_art_renderer::SpriteBlit blit{};

  [[nodiscard]] constexpr bool operator==(const ItemBlitPlan&) const = default;
};

struct LayoutPlan {
  PaneChrome pane{};
  PixelRect doll_column{};
  PixelRect grid_column{};
  GridPlan grid{};
  std::array<DollSlotPlan, paper_doll::kSlotCount> doll_slots{};
  std::array<ItemBlitPlan, inventory_grid::kMaxItems> item_blits{};
  std::uint8_t item_blit_count = 0;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const LayoutPlan&) const = default;
};

inline constexpr std::uint16_t kPaneWidth = 380;
inline constexpr std::uint16_t kPaneTop = 64;
inline constexpr std::uint16_t kPaneHeight = 430;
inline constexpr std::uint16_t kPaneMargin = 24;
inline constexpr std::uint16_t kPaneBottomMargin = 28;
inline constexpr std::uint16_t kInnerPad = 14;
inline constexpr std::uint16_t kDollColumnWidth = 130;
inline constexpr std::uint16_t kColumnGap = 8;
inline constexpr std::uint8_t kDollSlotW = 58;
inline constexpr std::uint8_t kDollSlotH = 32;
inline constexpr std::uint8_t kDollSlotGap = 4;

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

[[nodiscard]] constexpr std::uint16_t min_u16(std::uint16_t a, std::uint16_t b) {
  return a < b ? a : b;
}

[[nodiscard]] constexpr std::uint16_t max_u16(std::uint16_t a, std::uint16_t b) {
  return a > b ? a : b;
}

[[nodiscard]] constexpr PixelRect gear_pane_rect(const Viewport& vp) {
  PixelRect out;
  if (!vp.valid()) return out;
  const std::int32_t width = static_cast<std::int32_t>(vp.width);
  const std::int32_t height = static_cast<std::int32_t>(vp.height);
  const std::int32_t margin = static_cast<std::int32_t>(kPaneMargin);
  const std::int32_t pane_w = static_cast<std::int32_t>(kPaneWidth);
  const std::int32_t raw_x = width - pane_w - margin;
  const std::int32_t pane_x = raw_x < margin ? margin : raw_x;
  const std::int32_t bottom =
      std::min(height - static_cast<std::int32_t>(kPaneBottomMargin),
               static_cast<std::int32_t>(kPaneTop + kPaneHeight));
  const std::int32_t w = std::min(pane_w, std::max(0, width - pane_x));
  const std::int32_t h = bottom - static_cast<std::int32_t>(kPaneTop);
  if (w <= 0 || h <= 0) return out;
  out.x = clamp_i16(pane_x);
  out.y = clamp_i16(static_cast<std::int32_t>(kPaneTop));
  out.width = clamp_u16(w);
  out.height = clamp_u16(h);
  return out;
}

[[nodiscard]] constexpr PaneChrome plan_pane_chrome(const PixelRect& outer) {
  PaneChrome chrome;
  if (!outer.valid()) return chrome;
  chrome.outer = outer;
  framekit_renderer::Rect dest{outer.x, outer.y, outer.width, outer.height};
  chrome.frame =
      framekit_renderer::plan_nine_slice(dest, framekit_renderer::default_panel_asset());
  chrome.valid = chrome.frame.valid;
  return chrome;
}

[[nodiscard]] constexpr framekit_renderer::NineSlicePlan plan_slot_frame(
    const PixelRect& rect) {
  framekit_renderer::Rect dest{rect.x, rect.y, rect.width, rect.height};
  return framekit_renderer::plan_nine_slice(
      dest, framekit_renderer::default_slot_asset());
}

[[nodiscard]] constexpr PixelRect doll_slot_rect(const PixelRect& doll_column,
                                                 std::uint8_t index) {
  PixelRect out;
  if (!doll_column.valid() || index >= paper_doll::kSlotCount) return out;
  const std::uint8_t col = index % 2;
  const std::uint8_t row = index / 2;
  out.x = static_cast<std::int16_t>(
      doll_column.x + static_cast<std::int16_t>(col) *
                          static_cast<std::int16_t>(kDollSlotW + kDollSlotGap));
  out.y = static_cast<std::int16_t>(
      doll_column.y + static_cast<std::int16_t>(row) *
                          static_cast<std::int16_t>(kDollSlotH + kDollSlotGap));
  out.width = kDollSlotW;
  out.height = kDollSlotH;
  return out;
}

[[nodiscard]] constexpr GridPlan plan_grid(const PixelRect& grid_column,
                                           std::uint8_t columns,
                                           std::uint8_t rows) {
  GridPlan plan;
  if (!grid_column.valid() || columns == 0 || rows == 0) return plan;
  plan.area = grid_column;
  plan.columns = columns;
  plan.rows = rows;
  const std::int32_t inner_w = static_cast<std::int32_t>(grid_column.width);
  const std::int32_t inner_h = static_cast<std::int32_t>(grid_column.height);
  const std::int32_t gaps_w =
      static_cast<std::int32_t>(columns - 1) * static_cast<std::int32_t>(plan.gap);
  const std::int32_t gaps_h =
      static_cast<std::int32_t>(rows - 1) * static_cast<std::int32_t>(plan.gap);
  if (inner_w <= gaps_w || inner_h <= gaps_h) return plan;
  const std::uint16_t cw =
      clamp_u16((inner_w - gaps_w) / static_cast<std::int32_t>(columns));
  const std::uint16_t ch =
      clamp_u16((inner_h - gaps_h) / static_cast<std::int32_t>(rows));
  plan.cell_w = static_cast<std::uint8_t>(cw);
  plan.cell_h = static_cast<std::uint8_t>(ch);
  return plan;
}

[[nodiscard]] constexpr item_art_renderer::CellRect to_cell_rect(
    const PixelRect& rect) {
  item_art_renderer::CellRect cell;
  cell.x = rect.x;
  cell.y = rect.y;
  cell.width = rect.width;
  cell.height = rect.height;
  return cell;
}

[[nodiscard]] constexpr LayoutPlan plan_gear_overlay(
    const Viewport& vp, const inventory_grid::State& grid,
    const paper_doll::State& doll) {
  LayoutPlan plan;
  if (!vp.valid() || !grid.valid() || !doll.valid()) return plan;

  plan.pane = plan_pane_chrome(gear_pane_rect(vp));
  if (!plan.pane.valid) return plan;

  const PixelRect& outer = plan.pane.outer;
  const std::int32_t inner_left =
      static_cast<std::int32_t>(outer.x) + static_cast<std::int32_t>(kInnerPad);
  const std::int32_t inner_top =
      static_cast<std::int32_t>(outer.y) + static_cast<std::int32_t>(kInnerPad + 48);
  const std::int32_t inner_right =
      static_cast<std::int32_t>(outer.x) + static_cast<std::int32_t>(outer.width) -
      static_cast<std::int32_t>(kInnerPad);
  const std::int32_t inner_bottom =
      static_cast<std::int32_t>(outer.y) + static_cast<std::int32_t>(outer.height) -
      static_cast<std::int32_t>(kInnerPad);

  plan.doll_column.x = clamp_i16(inner_left);
  plan.doll_column.y = clamp_i16(inner_top);
  plan.doll_column.width = kDollColumnWidth;
  plan.doll_column.height =
      clamp_u16(max_u16(0, static_cast<std::uint16_t>(inner_bottom - inner_top)));

  plan.grid_column.x =
      clamp_i16(inner_left + static_cast<std::int32_t>(kDollColumnWidth) +
                static_cast<std::int32_t>(kColumnGap));
  plan.grid_column.y = plan.doll_column.y;
  plan.grid_column.width =
      clamp_u16(max_u16(0, static_cast<std::uint16_t>(inner_right -
                                                      plan.grid_column.x)));
  plan.grid_column.height = plan.doll_column.height;

  plan.grid = plan_grid(plan.grid_column, grid.width, grid.height);
  if (!plan.grid.valid()) return plan;

  for (std::uint8_t i = 0; i < paper_doll::kSlotCount; ++i) {
    DollSlotPlan& slot_plan = plan.doll_slots[static_cast<std::size_t>(i)];
    slot_plan.slot = static_cast<paper_doll::Slot>(i);
    slot_plan.rect = doll_slot_rect(plan.doll_column, i);
    slot_plan.slot_frame = plan_slot_frame(slot_plan.rect);
    const paper_doll::Equipped& equipped =
        doll.slots[static_cast<std::size_t>(i)];
    if (!equipped.empty()) {
      const auto resolved =
          item_art_renderer::resolve(equipped.id, to_cell_rect(slot_plan.rect));
      if (resolved.status == item_art_renderer::Status::Ok) {
        slot_plan.equipped_blit = resolved.blit;
        slot_plan.has_equipped = true;
      }
    }
  }

  plan.item_blit_count = 0;
  for (std::uint8_t i = 0; i < grid.count; ++i) {
    const inventory_grid::Item& item = grid.items[static_cast<std::size_t>(i)];
    const PixelRect footprint =
        plan.grid.footprint_rect(item.x, item.y, item.width, item.height);
    if (!footprint.valid()) continue;
    const auto resolved =
        item_art_renderer::resolve(item.id, to_cell_rect(footprint));
    if (resolved.status != item_art_renderer::Status::Ok) continue;
    ItemBlitPlan blit_plan;
    blit_plan.item_id = item.id;
    blit_plan.footprint = footprint;
    blit_plan.blit = resolved.blit;
    plan.item_blits[static_cast<std::size_t>(plan.item_blit_count)] = blit_plan;
    ++plan.item_blit_count;
  }

  plan.valid = true;
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const LayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.pane.outer.x) * 17u;
  hash ^= plan.pane.outer.width * 31u;
  hash ^= plan.grid.cell_w * 47u;
  hash ^= plan.grid.cell_h * 61u;
  hash ^= static_cast<std::uint32_t>(plan.item_blit_count) * 73u;
  hash ^= framekit_renderer::plan_checksum(plan.pane.frame);
  return hash;
}

}  // namespace inventory_pane_layout
