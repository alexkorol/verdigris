// pane_model.hpp — TASK-0158 pure pane model/layout foundation.
//
// Header-only, dependency-free planner for future Gear, Character, and
// Passive tabs. Given a viewport, content minimums, the active tab, and a
// row count, plan() returns bounded panel/header/tab/content/footer
// rectangles plus scroll/clipping metadata. Plans never overlap the reserved
// top HUD or bottom orbs/quickbar bands and degrade explicitly on small or
// invalid viewports instead of emitting off-screen geometry. No Win32/GDI,
// no I/O, no clocks: identical requests always yield identical plans.
// Painting and main.cpp integration stay outside this packet.
#pragma once

#include <algorithm>
#include <cstdint>

namespace pane {

enum class Tab : std::uint8_t { Gear, Character, Passive };

enum class Status : std::uint8_t {
  Ok,
  InvalidViewport,   // width/height not strictly positive
  InvalidMinimums,   // contradictory/negative minimums or negative rows
  ViewportTooSmall,  // safe band cannot hold the minimum pane chrome
};

// Reserved HUD geometry mirrors the shipped client's hud_safe_zones
// (native/client/main.cpp): a full-width top band covering the 132x132
// minimap plus the four status-chip rows (through y=148), and the bottom
// 96px band covering the vital orbs and quickbar strip.
inline constexpr int kTopHudReserveHeight = 148;
inline constexpr int kBottomHudReserveHeight = 96;
inline constexpr int kSideMargin = 16;

struct Viewport {
  int width = 0;
  int height = 0;
};

struct Rect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  [[nodiscard]] constexpr int right() const { return x + w; }
  [[nodiscard]] constexpr int bottom() const { return y + h; }
  [[nodiscard]] constexpr bool empty() const { return w <= 0 || h <= 0; }
  // Half-open [x, x+w) x [y, y+h) overlap; zero-area rects never overlap.
  [[nodiscard]] constexpr bool overlaps(const Rect& other) const {
    if (empty() || other.empty()) return false;
    return x < other.right() && other.x < right() && y < other.bottom() &&
           other.y < bottom();
  }
  [[nodiscard]] constexpr bool contains(const Rect& other) const {
    return other.x >= x && other.y >= y && other.right() <= right() &&
           other.bottom() <= bottom();
  }
};

struct ContentMinimums {
  int preferred_width = 420;
  int min_width = 340;
  int preferred_height = 460;
  int min_height = 280;
  int header_height = 34;
  int tab_bar_height = 38;
  int footer_height = 30;
  int row_height = 26;
};

struct Request {
  Viewport viewport{};
  Tab tab = Tab::Gear;
  ContentMinimums minimums{};
  int rows = 0;
};

struct Scroll {
  int row_height = 0;
  int rows_total = 0;
  int rows_visible = 0;
  int content_height_px = 0;
  int total_height_px = 0;
  int max_offset_px = 0;
  bool clipped = false;
};

struct Plan {
  Status status = Status::ViewportTooSmall;
  Tab tab = Tab::Gear;
  Rect reserved_top{};
  Rect reserved_bottom{};
  Rect panel{};
  Rect header{};
  Rect tab_bar{};
  Rect content{};
  Rect footer{};
  Scroll scroll{};

  [[nodiscard]] bool ok() const { return status == Status::Ok; }
  [[nodiscard]] const char* status_text() const {
    switch (status) {
      case Status::Ok:
        return "ok";
      case Status::InvalidViewport:
        return "invalid viewport";
      case Status::InvalidMinimums:
        return "invalid content minimums";
      case Status::ViewportTooSmall:
        return "viewport too small for minimum pane";
    }
    return "unknown status";
  }
  // A failed plan carries only its status; every rectangle stays zeroed so
  // callers can never paint off-screen geometry from a degraded result.
  [[nodiscard]] static Plan with_status(Status status) {
    Plan failed{};
    failed.status = status;
    return failed;
  }
};

[[nodiscard]] inline Plan plan(const Request& request) {
  Plan out;
  const ContentMinimums& m = request.minimums;
  const long long chrome = static_cast<long long>(m.header_height) +
                           static_cast<long long>(m.tab_bar_height) +
                           static_cast<long long>(m.footer_height);

  if (request.viewport.width <= 0 || request.viewport.height <= 0)
    return out.with_status(Status::InvalidViewport);

  const bool bad_minimums = m.min_width <= 0 || m.min_height <= 0 ||
                            m.preferred_width < m.min_width ||
                            m.preferred_height < m.min_height ||
                            m.header_height < 0 || m.tab_bar_height < 0 ||
                            m.footer_height < 0 || m.row_height <= 0 ||
                            request.rows < 0 ||
                            chrome > static_cast<long long>(m.min_height);
  if (bad_minimums) return out.with_status(Status::InvalidMinimums);

  const int avail_top = kTopHudReserveHeight;
  const int avail_bottom = request.viewport.height - kBottomHudReserveHeight;
  const int avail_h = avail_bottom - avail_top;
  const int max_w = request.viewport.width - 2 * kSideMargin;
  if (max_w < m.min_width || avail_h < m.min_height)
    return out.with_status(Status::ViewportTooSmall);

  const int w = std::min(m.preferred_width, max_w);
  const int h = std::min(m.preferred_height, avail_h);

  out.reserved_top =
      Rect{0, 0, request.viewport.width, kTopHudReserveHeight};
  out.reserved_bottom =
      Rect{0, request.viewport.height - kBottomHudReserveHeight,
           request.viewport.width, kBottomHudReserveHeight};
  out.panel = Rect{(request.viewport.width - w) / 2, avail_top + (avail_h - h) / 2,
                   w, h};
  out.header = Rect{out.panel.x, out.panel.y, w, m.header_height};
  out.tab_bar = Rect{out.panel.x, out.header.bottom(), w, m.tab_bar_height};
  out.footer =
      Rect{out.panel.x, out.panel.bottom() - m.footer_height, w, m.footer_height};
  out.content =
      Rect{out.panel.x, out.tab_bar.bottom(), w, out.footer.y - out.tab_bar.bottom()};
  out.tab = request.tab;

  Scroll s;
  s.row_height = m.row_height;
  s.rows_total = request.rows;
  s.content_height_px = out.content.h;
  s.rows_visible = s.content_height_px / s.row_height;
  const long long total =
      static_cast<long long>(s.rows_total) * static_cast<long long>(s.row_height);
  constexpr long long kIntMax = 2147483647LL;
  s.total_height_px = static_cast<int>(std::min(total, kIntMax));
  const long long overflow = std::max<long long>(0, total - s.content_height_px);
  s.max_offset_px = static_cast<int>(std::min(overflow, kIntMax));
  s.clipped = total > static_cast<long long>(s.content_height_px);
  out.scroll = s;

  out.status = Status::Ok;
  return out;
}

// Clamps a requested content scroll offset into the plan's valid range.
[[nodiscard]] inline int clamped_scroll_offset(const Scroll& scroll,
                                               int requested) {
  return std::clamp(requested, 0, std::max(0, scroll.max_offset_px));
}

// Index of the first content row visible at a scroll offset.
[[nodiscard]] inline int first_visible_row(const Scroll& scroll, int offset_px) {
  if (scroll.row_height <= 0) return 0;
  return clamped_scroll_offset(scroll, offset_px) / scroll.row_height;
}

}  // namespace pane
