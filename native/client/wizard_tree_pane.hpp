// wizard_tree_pane.hpp — Owner Demo integration of the geometric passive
// tree (TASK-0193 model through the TASK-0194 layout contract): the
// first-level tree pane. Presentation + bounded client-side allocation via
// the model's own rules; when an authoritative progression payload is
// present its unspent-point count seeds the pane and is labeled as the
// authority. Full server-authoritative allocation remains TASK-0194 work.
#pragma once

#include "geometric_skill_tree.hpp"
#include "skill_tree_layout.hpp"
#include "wizard_orb_art.hpp"
#include "wizard_splash_art.hpp"

#include <windows.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace wizard_tree_pane {

inline constexpr double kHexPi = 3.14159265358979323846;

using geometric_skill_tree::State;
using skill_tree_layout::LayoutPlan;
using skill_tree_layout::SeatLayout;
using wizard_orb_art::OrbBitmap;
using wizard_splash_art::draw_nine_slice_panel;
using wizard_splash_art::fill_rect_alpha;
using wizard_splash_art::AlphaBlendProc;

struct TreePane {
  State model{};
  LayoutPlan plan{};
  LayoutPlan display{};  // pane-local geometry at presentation radius
  bool open = false;
  int hover_seat = -1;
  bool seeded = false;
};

inline constexpr std::uint16_t kPaneSeatRadius = 42;
inline constexpr std::uint16_t kPaneHitPad = 8;

// Rebuilds the pane-local display plan: same axial seats and clickability,
// presentation-sized centers and hit rects.
inline void refresh_display(TreePane& pane, int width, int height) {
  pane.display = pane.plan;
  if (!pane.plan.valid) return;
  const skill_tree_layout::PixelPoint center{
      static_cast<std::int16_t>(width / 2),
      static_cast<std::int16_t>(height / 2 + 14)};
  pane.display.panel_center = center;
  for (std::uint8_t i = 0; i < pane.plan.seat_count; ++i) {
    const SeatLayout& seat = pane.plan.seats[i];
    SeatLayout scaled = seat;
    scaled.center = skill_tree_layout::axial_to_pixel(seat.pos, center,
                                                      kPaneSeatRadius);
    const std::int32_t half =
        static_cast<std::int32_t>(kPaneSeatRadius + kPaneHitPad);
    scaled.hit.x = static_cast<std::int16_t>(scaled.center.x - half);
    scaled.hit.y = static_cast<std::int16_t>(scaled.center.y - half);
    scaled.hit.width = static_cast<std::uint16_t>(half * 2);
    scaled.hit.height = scaled.hit.width;
    pane.display.seats[i] = scaled;
  }
}

// Seeds the pane once: the owner-demo first-level hex slice, unlocked from
// level 2, with authoritative unspent points when the payload is present.
inline void ensure_seeded(TreePane& pane, int player_level,
                          bool authoritative_present,
                          std::uint16_t authoritative_unspent) {
  if (pane.seeded) return;
  pane.seeded = true;
  pane.model = geometric_skill_tree::make_owner_demo_first_level_slice();
  pane.model.tree_unlocked =
      player_level >= geometric_skill_tree::kOwnerDemoFirstLevel ||
      authoritative_present;
  pane.model.player_level = static_cast<std::uint8_t>(
      player_level > 255 ? 255 : (player_level < 0 ? 0 : player_level));
  pane.model.skill_points =
      authoritative_present ? authoritative_unspent
                            : geometric_skill_tree::kOwnerDemoFirstLevelPoints;
}

inline void refresh_plan(TreePane& pane, int width, int height) {
  skill_tree_layout::Viewport vp{static_cast<std::uint16_t>(width),
                                 static_cast<std::uint16_t>(height)};
  pane.plan = skill_tree_layout::plan_level_up_panel(vp, pane.model);
  refresh_display(pane, width, height);
}

inline int seat_at(const TreePane& pane, int x, int y) {
  if (!pane.display.valid) return -1;
  for (std::uint8_t i = 0; i < pane.display.seat_count; ++i) {
    const SeatLayout& seat = pane.display.seats[i];
    const int left = seat.hit.x;
    const int top = seat.hit.y;
    if (x >= left && x < left + seat.hit.width && y >= top &&
        y < top + seat.hit.height) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

// Returns true when the click allocated a seat (the caller then re-plans).
inline bool click(TreePane& pane, int x, int y) {
  const int index = seat_at(pane, x, y);
  if (index < 0) return false;
  const geometric_skill_tree::Status status = geometric_skill_tree::allocate_at(
      pane.model, pane.model.seats[static_cast<std::size_t>(index)].pos);
  return status == geometric_skill_tree::Status::Ok;
}

inline COLORREF seat_color(const SeatLayout& seat) {
  using ST = geometric_skill_tree::SeatType;
  switch (seat.type) {
    case ST::Sign:
      return RGB(239, 208, 116);
    case ST::Socket:
      return RGB(116, 204, 208);
    case ST::Keystone:
      return RGB(214, 92, 72);
    case ST::Gateway:
      return RGB(170, 140, 220);
    case ST::Class:
      return RGB(120, 214, 168);
    case ST::Passive:
    default:
      return RGB(150, 170, 158);
  }
}

inline void draw_hex_seat(HDC dc, const SeatLayout& seat, bool hovered) {
  const int r = kPaneSeatRadius;
  const int cx = seat.center.x;
  const int cy = seat.center.y;
  const COLORREF base = seat_color(seat);

  POINT hex[6];
  for (int i = 0; i < 6; ++i) {
    const double angle = kHexPi * i / 3.0;
    hex[i].x = cx + static_cast<int>(std::cos(angle) * (r - 4));
    hex[i].y = cy + static_cast<int>(std::sin(angle) * (r - 4));
  }
  HBRUSH fill = CreateSolidBrush(seat.active ? RGB(52, 60, 44) : RGB(24, 30, 30));
  HGDIOBJ old_brush = SelectObject(dc, fill);
  HPEN pen = CreatePen(PS_SOLID, hovered ? 3 : 2,
                       seat.active ? RGB(255, 232, 150)
                                   : (hovered ? RGB(255, 244, 200) : base));
  HGDIOBJ old_pen = SelectObject(dc, pen);
  Polygon(dc, hex, 6);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(fill);
  DeleteObject(pen);

  // Inner pip: filled when allocated, hollow when clickable, dim otherwise.
  const int pip = std::max(2, r / 4);
  if (seat.active) {
    HBRUSH pip_fill = CreateSolidBrush(base);
    HGDIOBJ old = SelectObject(dc, pip_fill);
    Ellipse(dc, cx - pip, cy - pip, cx + pip, cy + pip);
    SelectObject(dc, old);
    DeleteObject(pip_fill);
  } else if (seat.clickable) {
    HPEN pip_pen = CreatePen(PS_SOLID, 2, RGB(255, 232, 150));
    HGDIOBJ old = SelectObject(dc, pip_pen);
    Ellipse(dc, cx - pip, cy - pip, cx + pip, cy + pip);
    SelectObject(dc, old);
    DeleteObject(pip_pen);
  }
}

// Full pane paint. Returns the pane frame rect via out-frame when non-null.
inline void draw_pane(HDC dc, AlphaBlendProc alpha_blend,
                      const wizard_orb_art::OrbBitmap* panel_texture,
                      TreePane& pane, int width, int height,
                      bool authoritative_present) {
  if (!pane.open || !pane.plan.valid) return;
  // The display geometry always matches the surface actually being drawn.
  refresh_display(pane, width, height);
  fill_rect_alpha(dc, alpha_blend, 0, 0, width, height, 150);

  const int frame_w = 560;
  const int frame_h = 460;
  const int fx = (width - frame_w) / 2;
  const int fy = (height - frame_h) / 2;
  if (panel_texture && panel_texture->ok()) {
    draw_nine_slice_panel(dc, alpha_blend, *panel_texture, fx, fy, frame_w,
                          frame_h, 255);
  } else {
    RECT backdrop{fx, fy, fx + frame_w, fy + frame_h};
    HBRUSH brush = CreateSolidBrush(RGB(25, 33, 37));
    FillRect(dc, &backdrop, brush);
    DeleteObject(brush);
  }

  SetBkMode(dc, TRANSPARENT);
  // Title.
  HFONT title_font = CreateFontA(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, "Georgia");
  HGDIOBJ old_font = SelectObject(dc, title_font);
  SetTextColor(dc, RGB(120, 214, 168));
  const char* title = "PASSIVE TREE";
  TextOutA(dc, fx + 22, fy + 16, title, static_cast<int>(strlen(title)));
  SelectObject(dc, old_font);
  DeleteObject(title_font);

  char points_line[128];
  std::snprintf(points_line, sizeof(points_line), "unspent points: %u   %s",
                static_cast<unsigned>(pane.model.skill_points),
                authoritative_present ? "(authoritative)" : "(local preview)");
  SetTextColor(dc, RGB(239, 208, 116));
  TextOutA(dc, fx + 22, fy + 48, points_line,
           static_cast<int>(strlen(points_line)));

  // Links between adjacent seats: bright when both ends are active.
  for (std::uint8_t i = 0; i < pane.display.seat_count; ++i) {
    const SeatLayout& a = pane.display.seats[i];
    for (std::uint8_t j = static_cast<std::uint8_t>(i + 1);
         j < pane.display.seat_count; ++j) {
      const SeatLayout& b = pane.display.seats[j];
      if (geometric_skill_tree::hex_distance(a.pos, b.pos) != 1) {
        continue;
      }
      HPEN link = CreatePen(PS_SOLID, a.active && b.active ? 3 : 1,
                            a.active && b.active ? RGB(239, 208, 116)
                                                 : RGB(70, 84, 80));
      HGDIOBJ old = SelectObject(dc, link);
      MoveToEx(dc, a.center.x, a.center.y, nullptr);
      LineTo(dc, b.center.x, b.center.y);
      SelectObject(dc, old);
      DeleteObject(link);
    }
  }

  // Seats (hover state is baked into plan by the caller via hover_seat).
  for (std::uint8_t i = 0; i < pane.display.seat_count; ++i) {
    draw_hex_seat(dc, pane.display.seats[i],
                  static_cast<int>(i) == pane.hover_seat);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(140, 158, 148));
    const char* type_label = geometric_skill_tree::seat_type_name(
        pane.display.seats[i].type);
    const int label_x = pane.display.seats[i].center.x -
                        static_cast<int>(strlen(type_label)) * 3;
    TextOutA(dc, label_x, pane.display.seats[i].center.y + kPaneSeatRadius - 6,
             type_label, static_cast<int>(strlen(type_label)));
  }

  SetTextColor(dc, RGB(150, 165, 152));
  const char* footer =
      pane.model.tree_unlocked
          ? "click a highlighted seat to allocate | T or Escape closes"
          : "the tree opens at level 2 | T or Escape closes";
  TextOutA(dc, fx + 22, fy + frame_h - 30, footer,
           static_cast<int>(strlen(footer)));
}

}  // namespace wizard_tree_pane
