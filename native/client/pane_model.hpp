// pane_model.hpp — TASK-0158 native pane model and layout foundation.
//
// Pure, header-only, dependency-free layout planner for the Gear / Character /
// Passives tabbed panel. Given a viewport, content minimums, the active tab,
// and per-tab row counts, it returns bounded panel/header/tab/content/footer
// rectangles plus scroll and clipping metadata. It never paints: presentation
// asks for a plan and draws it. No Win32/GDI, no I/O, no time, no globals —
// the same inputs always produce the same plan.
//
// The planner must never overlap the reserved HUD bands and must degrade
// explicitly (compact metrics, or an InsufficientArea failure with empty
// rectangles) rather than ever returning rectangles that draw off-screen.

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

namespace pane_model {

// Reserved bands, matching the live HUD planners:
//  - top HUD rows (TASK-0153): row0 y=12, 4 rows x 34 step, chip height + gap
//  - bottom orbs/quickbar reserve (TASK-0068/0076): 96 px strip
constexpr int kReservedTopHudHeight = 148;
constexpr int kReservedBottomHudHeight = 96;

constexpr int kPanelGutter = 12;        // clearance between panel and bands
constexpr int kCompactPanelGutter = 8;  // degraded viewports
constexpr int kHeaderHeight = 30;
constexpr int kTabBarHeight = 34;
constexpr int kFooterHeight = 24;
constexpr int kContentPad = 14;
constexpr int kCompactContentPad = 8;
constexpr int kDefaultRowHeight = 24;

// Below this even the compact plan cannot fit between the reserved bands.
constexpr int kMinViewportWidth = 640;
constexpr int kMinViewportHeight = 400;

// Full-size metrics apply at/above this viewport class; below it the plan is
// explicitly flagged degraded (this is the 960x600 class).
constexpr int kFullViewportWidth = 1366;
constexpr int kFullViewportHeight = 768;

constexpr int kPreferredPanelWidthFull = 640;
constexpr int kPreferredPanelWidthCompact = 560;

constexpr int kTabCount = 3;

enum class Tab { Gear = 0, Character = 1, Passives = 2 };

enum class Failure {
  None = 0,
  InvalidViewport,    // non-positive or below the hard floor
  InvalidRequest,     // non-positive row height or negative row counts
  InsufficientArea,   // explicit degradation: nothing fits, nothing drawn
};

struct Rect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
};

inline bool intersects(const Rect& a, const Rect& b) {
  return a.x < b.x + b.w && b.x < a.x + a.w &&
         a.y < b.y + b.h && b.y < a.y + a.h;
}

inline bool contains(const Rect& outer, const Rect& inner) {
  return inner.x >= outer.x && inner.y >= outer.y &&
         inner.x + inner.w <= outer.x + outer.w &&
         inner.y + inner.h <= outer.y + outer.h;
}

struct ContentMinimums {
  int min_panel_width = 420;
  int min_content_width = 320;
  int min_content_height = 96;
};

struct Request {
  int viewport_width = 0;
  int viewport_height = 0;
  Tab active_tab = Tab::Gear;
  ContentMinimums minimums{};
  std::array<int, kTabCount> row_counts{}; // list rows per tab
  int row_height = kDefaultRowHeight;
  int scroll_offset_px = 0;                // requested offset, clamped in plan
};

struct TabButton {
  Rect rect;
  bool active = false;
};

struct ScrollInfo {
  bool clipped = false;
  int visible_rows = 0;
  int first_visible_row = 0;
  int scroll_offset_px = 0;
  int max_scroll_offset_px = 0;
  int total_height_px = 0;
};

struct Plan {
  bool ok = false;
  Failure failure = Failure::None;
  bool degraded = false;
  Rect panel{};
  Rect header{};
  Rect tab_bar{};
  Rect content{};
  Rect footer{};
  std::array<TabButton, kTabCount> tabs{};
  ScrollInfo scroll{};
  Rect reserved_top{};
  Rect reserved_bottom{};
};

inline Rect reserved_top_rect(int viewport_width) {
  return Rect{0, 0, std::max(0, viewport_width), kReservedTopHudHeight};
}

inline Rect reserved_bottom_rect(int viewport_width, int viewport_height) {
  const int h = std::min(kReservedBottomHudHeight, std::max(0, viewport_height));
  return Rect{0, std::max(0, viewport_height) - h, std::max(0, viewport_width), h};
}

inline Plan plan(const Request& request) {
  Plan plan;
  const int w = request.viewport_width;
  const int h = request.viewport_height;

  plan.reserved_top = reserved_top_rect(w);
  plan.reserved_bottom = reserved_bottom_rect(w, h);

  if (w < kMinViewportWidth || h < kMinViewportHeight) {
    plan.failure = Failure::InvalidViewport;
    plan.reserved_top = Rect{};
    plan.reserved_bottom = Rect{};
    return plan;
  }
  if (request.row_height <= 0) {
    plan.failure = Failure::InvalidRequest;
    plan.reserved_top = Rect{};
    plan.reserved_bottom = Rect{};
    return plan;
  }
  for (int i = 0; i < kTabCount; ++i) {
    if (request.row_counts[static_cast<std::size_t>(i)] < 0) {
      plan.failure = Failure::InvalidRequest;
      plan.reserved_top = Rect{};
      plan.reserved_bottom = Rect{};
      return plan;
    }
  }

  plan.degraded = w < kFullViewportWidth || h < kFullViewportHeight;
  const int gutter = plan.degraded ? kCompactPanelGutter : kPanelGutter;
  const int pad = plan.degraded ? kCompactContentPad : kContentPad;

  const int avail_y0 = kReservedTopHudHeight + gutter;
  const int avail_y1 = h - kReservedBottomHudHeight - gutter;
  const int avail_h = avail_y1 - avail_y0;
  const int avail_w = w - 2 * gutter;

  const int preferred_w = plan.degraded ? kPreferredPanelWidthCompact
                                        : kPreferredPanelWidthFull;
  const int wanted_w = std::max(preferred_w, std::max(
      request.minimums.min_panel_width,
      request.minimums.min_content_width + 2 * pad));
  const int panel_w = std::min(avail_w, wanted_w);

  const int inner_h = avail_h - kHeaderHeight - kTabBarHeight -
                      kFooterHeight - 2 * pad;
  const bool fits = panel_w >= request.minimums.min_panel_width &&
                    panel_w - 2 * pad >= request.minimums.min_content_width &&
                    inner_h >= request.minimums.min_content_height;
  if (!fits) {
    plan.failure = Failure::InsufficientArea;
    plan.reserved_top = Rect{};
    plan.reserved_bottom = Rect{};
    return plan;
  }

  plan.panel = Rect{(w - panel_w) / 2, avail_y0, panel_w, avail_h};
  plan.header = Rect{plan.panel.x, plan.panel.y, plan.panel.w, kHeaderHeight};
  plan.tab_bar = Rect{plan.panel.x, plan.panel.y + kHeaderHeight,
                      plan.panel.w, kTabBarHeight};
  plan.footer = Rect{plan.panel.x, plan.panel.y + plan.panel.h - kFooterHeight,
                     plan.panel.w, kFooterHeight};
  plan.content = Rect{plan.panel.x + pad, plan.tab_bar.y + kTabBarHeight,
                      plan.panel.w - 2 * pad,
                      plan.footer.y - plan.tab_bar.y - kTabBarHeight - pad};

  const int tab_w = plan.tab_bar.w / kTabCount;
  for (int i = 0; i < kTabCount; ++i) {
    TabButton& button = plan.tabs[static_cast<std::size_t>(i)];
    button.rect = Rect{plan.tab_bar.x + i * tab_w, plan.tab_bar.y,
                       (i == kTabCount - 1) ? plan.tab_bar.w - (kTabCount - 1) * tab_w
                                            : tab_w,
                       plan.tab_bar.h};
    button.active = (i == static_cast<int>(request.active_tab));
  }

  const int active = static_cast<int>(request.active_tab);
  const int rows = request.row_counts[static_cast<std::size_t>(active)];
  ScrollInfo& scroll = plan.scroll;
  scroll.total_height_px = rows * request.row_height;
  scroll.clipped = scroll.total_height_px > plan.content.h;
  scroll.max_scroll_offset_px = std::max(0, scroll.total_height_px - plan.content.h);
  scroll.scroll_offset_px = std::clamp(request.scroll_offset_px, 0,
                                       scroll.max_scroll_offset_px);
  scroll.first_visible_row = std::min(rows, scroll.scroll_offset_px / request.row_height);
  const int row_capacity = plan.content.h >= request.row_height
                               ? plan.content.h / request.row_height
                               : 0;
  scroll.visible_rows = std::min(row_capacity,
                                 std::max(0, rows - scroll.first_visible_row));

  plan.ok = true;
  plan.failure = Failure::None;
  return plan;
}

} // namespace pane_model
