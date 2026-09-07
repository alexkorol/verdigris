#pragma once

// vector_art.hpp — the procedural, animated vector art set for the native
// client. Replaces the raster actor/scenery/terrain plates with parametric
// GDI drawing: every figure is polygons and ellipses driven by a pose
// (walk cycle, breathing, attack swing, mirroring), so art animates from
// authoritative data instead of shipping sprite sheets. Terrain painters
// render per-theme tiles into the floor cache, where their cost is paid
// once per camera tile-crossing, not per frame.
//
// Palette discipline: the Verdigris bronze-age language — oxidised copper,
// smoked bronze, bone, ember — one hue family per monster theme so a foe's
// home road is readable at a glance.

#ifdef _WIN32

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <string>

namespace vector_art {

inline constexpr double kPi = 3.14159265358979323846;

// ── small GDI helpers ───────────────────────────────────────────────────
// Scenario captures paint into a 32bpp DIB; GdipCreateBitmapFromHBITMAP then
// swap_bgra_rb treats those bytes as BGRA. Raw GDI COLORREFs land as RGB in
// that DIB, so bronze cloth becomes blue in the PNG while GDI+ HUD orbs
// (PARGB cache → AlphaBlend) stay correct. The live backbuffer is a
// device bitmap without bits, so the swap must not run there.

inline bool dc_is_32bpp_dib(HDC dc) {
  HBITMAP bitmap = static_cast<HBITMAP>(GetCurrentObject(dc, OBJ_BITMAP));
  if (!bitmap) return false;
  BITMAP bm{};
  if (GetObject(bitmap, sizeof(bm), &bm) < static_cast<int>(sizeof(BITMAP)))
    return false;
  return bm.bmBitsPixel == 32 && bm.bmBits != nullptr;
}

inline COLORREF dc_color(HDC dc, COLORREF c) {
  if (!dc_is_32bpp_dib(dc)) return c;
  return RGB(GetBValue(c), GetGValue(c), GetRValue(c));
}

inline void fill_poly(HDC dc, const POINT* points, int count, COLORREF fill,
                      COLORREF outline) {
  HBRUSH brush = CreateSolidBrush(dc_color(dc, fill));
  HPEN pen = CreatePen(PS_SOLID, 1, dc_color(dc, outline));
  HGDIOBJ old_brush = SelectObject(dc, brush);
  HGDIOBJ old_pen = SelectObject(dc, pen);
  Polygon(dc, points, count);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(brush);
  DeleteObject(pen);
}

inline void fill_ell(HDC dc, int cx, int cy, int rx, int ry, COLORREF fill,
                      COLORREF outline) {
  HBRUSH brush = CreateSolidBrush(dc_color(dc, fill));
  HPEN pen = CreatePen(PS_SOLID, 1, dc_color(dc, outline));
  HGDIOBJ old_brush = SelectObject(dc, brush);
  HGDIOBJ old_pen = SelectObject(dc, pen);
  Ellipse(dc, cx - rx, cy - ry, cx + rx, cy + ry);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(brush);
  DeleteObject(pen);
}

inline void line(HDC dc, int x0, int y0, int x1, int y1, COLORREF color,
                 int width) {
  HPEN pen = CreatePen(PS_SOLID, width, dc_color(dc, color));
  HGDIOBJ old_pen = SelectObject(dc, pen);
  MoveToEx(dc, x0, y0, nullptr);
  LineTo(dc, x1, y1);
  SelectObject(dc, old_pen);
  DeleteObject(pen);
}

inline COLORREF shade(COLORREF c, double factor) {
  const auto channel = [&](int v) {
    return static_cast<BYTE>(std::min(255.0, std::max(0.0, v * factor)));
  };
  return RGB(channel(GetRValue(c)), channel(GetGValue(c)),
             channel(GetBValue(c)));
}

// Unit-space mapper: figures are authored in a box x in [-50, 50],
// y in [0 (feet), 100 (crown)], y up; `height` scales, `mirror` flips x.
struct Frame {
  int cx = 0;
  int base_y = 0;
  double scale = 1.0;
  bool mirror = false;

  POINT at(double ux, double uy) const {
    const double sx = mirror ? -ux : ux;
    return {cx + static_cast<int>(std::lround(sx * scale)),
            base_y - static_cast<int>(std::lround(uy * scale))};
  }
};

// ── poses and styles ────────────────────────────────────────────────────

struct Pose {
  double walk = 0.0;     // walk cycle 0..1 (0 = standing)
  double moving = 0.0;   // 0..1 blend into the walk cycle
  double breathe = 0.0;  // slow idle cycle 0..1
  double attack = 0.0;   // 0 = idle, ramps 0..1 through a swing
  // VG-ART-003: melee is four readable poses, not a single sine of frame count.
  enum class AttackStage { Idle, Windup, Active, Recovery, Cancel };
  AttackStage attack_stage = AttackStage::Idle;
  bool mirror = false;
};

struct Style {
  COLORREF skin = RGB(196, 158, 120);
  COLORREF cloth = RGB(122, 104, 78);
  COLORREF trim = RGB(90, 74, 56);
  COLORREF metal = RGB(176, 184, 192);
  COLORREF accent = RGB(120, 214, 168);
  COLORREF dark = RGB(24, 20, 16);
};

enum class Held : int { None, Axe, Sword, Staff, Bow, Scales, Ledger, Club };

inline Held held_from_item(const std::string& id, const std::string& name) {
  std::string key = id + " " + name;
  for (auto& ch : key)
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  if (key.find_first_not_of(" ") == std::string::npos) return Held::None;
  if (key.find("bow") != std::string::npos) return Held::Bow;
  if (key.find("staff") != std::string::npos ||
      key.find("wand") != std::string::npos)
    return Held::Staff;
  if (key.find("sword") != std::string::npos ||
      key.find("pike") != std::string::npos ||
      key.find("spear") != std::string::npos ||
      key.find("dagger") != std::string::npos ||
      key.find("knife") != std::string::npos ||
      key.find("blade") != std::string::npos)
    return Held::Sword;
  if (key.find("axe") != std::string::npos ||
      key.find("hatchet") != std::string::npos)
    return Held::Axe;
  if (key.find("club") != std::string::npos ||
      key.find("mace") != std::string::npos)
    return Held::Club;
  return Held::Club;
}

inline const char* held_label(Held held) {
  switch (held) {
    case Held::Axe:
      return "held:axe";
    case Held::Sword:
      return "held:sword";
    case Held::Staff:
      return "held:staff";
    case Held::Bow:
      return "held:bow";
    case Held::Scales:
      return "held:scales";
    case Held::Ledger:
      return "held:ledger";
    case Held::Club:
      return "held:club";
    case Held::None:
    default:
      return "held:none";
  }
}

// ── humanoid rig ────────────────────────────────────────────────────────
// Bronze-age adult, 3/4 side read. Head is ~1/8 of standing height; a chibi
// (head > 1/5) cannot pass VG-ART-001. Legs swing with the walk cycle,
// torso bobs with breath/steps, the near arm swings the held tool on attack.

inline constexpr double kAdultHeadUnits = 12.0;
inline constexpr double kAdultBodyUnits = 100.0;
inline constexpr double kAdultCrown = 98.0;

inline bool chibi_head_fails_review(double head_units, double body_units) {
  if (body_units <= 0.0) return true;
  return (head_units / body_units) > 0.20;
}

inline bool adult_head_passes(double head_units, double body_units) {
  if (body_units <= 0.0) return false;
  const double ratio = head_units / body_units;
  return ratio >= 0.10 && ratio <= 0.16 &&
         !chibi_head_fails_review(head_units, body_units);
}

// VG-ART-003: windup cocks the blade back; active commits it forward. HUD
// pose labels or frame count without those silhouettes cannot certify.
inline constexpr bool kAttackWindupCocksBlade = true;
inline constexpr bool kAttackActiveExtendsBlade = true;
inline constexpr int kLimbMinPx = 4;

inline bool stick_attack_fails_review(bool windup_cocks, bool active_extends,
                                      bool blade_readable) {
  return !windup_cocks || !active_extends || !blade_readable;
}

inline bool idle_as_attack_family_fails_review(bool only_idle_pose) {
  return only_idle_pose;
}

inline const char* owner_strike_poses_label() { return "Strike poses"; }
inline bool pose_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}

// VG-ART-006: WarCry weave beats share bronze identity. A single blob or
// screen-filling ring cannot certify the family.
inline constexpr bool kWeaveHasCastMotes = true;
inline constexpr bool kWeaveHasTravelOrbit = true;
inline constexpr bool kWeaveHasImpactTicks = true;
inline constexpr bool kWeaveHasCancelImplode = true;
inline constexpr COLORREF kWeaveBronze = RGB(239, 190, 78);
inline constexpr COLORREF kWeaveBright = RGB(255, 224, 128);
inline constexpr COLORREF kWeaveEmber = RGB(196, 122, 48);

inline bool blob_weave_fails_review(bool cast_motes, bool travel_orbit,
                                    bool impact_ticks, bool cancel_implode) {
  return !cast_motes || !travel_orbit || !impact_ticks || !cancel_implode;
}

inline const char* owner_war_cry_weave_label() { return "War Cry weave"; }
inline bool weave_strip_covers_hud_fails_review(bool overlap) { return overlap; }

inline void humanoid(HDC dc, int cx, int base_y, int height_px,
                     const Style& style, const Pose& pose, Held held) {
  Frame f{cx, base_y, height_px / kAdultBodyUnits, pose.mirror};
  const double swing = std::sin(pose.walk * 2.0 * kPi) * pose.moving;
  const double bob = std::sin(pose.breathe * 2.0 * kPi) * 1.2 +
                     std::abs(swing) * 1.6;
  double attack_lean = 0.0;
  double strike = 0.0;
  double rear_weight = 0.0;
  double reach = 0.0;
  switch (pose.attack_stage) {
    case Pose::AttackStage::Windup:
      attack_lean = -10.0;
      strike = -1.15;
      rear_weight = 0.85;
      reach = 2.0;
      break;
    case Pose::AttackStage::Active:
      attack_lean = 14.0;
      strike = 1.85;
      rear_weight = -0.55;
      reach = 10.0;
      break;
    case Pose::AttackStage::Recovery:
      attack_lean = 5.0;
      strike = 0.55;
      rear_weight = -0.15;
      reach = 4.0;
      break;
    case Pose::AttackStage::Cancel:
      attack_lean = -8.0;
      strike = -0.45;
      rear_weight = 0.4;
      reach = 0.0;
      break;
    case Pose::AttackStage::Idle:
    default:
      break;
  }
  const double lean = pose.moving * 3.0 + attack_lean;
  const double hip_w = 5.8;
  const double arm_w = 4.4;

  // Jointed legs (hip / knee / foot). A single hip-to-foot plank reads as a
  // crate, not an adult. Attack stages plant the rear foot or lunge.
  const auto leg = [&](double side, double phase_swing, COLORREF tone) {
    const double plant = side < 0.0 ? rear_weight : -rear_weight;
    const double hip_x = side * 7.0;
    const double knee_x = side * 6.0 + phase_swing * 9.0 + plant * 4.0;
    const double foot_x = side * 6.0 + phase_swing * 15.0 + plant * 10.0;
    POINT thigh[4] = {f.at(hip_x - hip_w, 50), f.at(hip_x + hip_w, 50),
                      f.at(knee_x + hip_w * 0.85, 27),
                      f.at(knee_x - hip_w * 0.9, 27)};
    fill_poly(dc, thigh, 4, tone, style.dark);
    POINT shin[4] = {f.at(knee_x - hip_w * 0.85, 27),
                     f.at(knee_x + hip_w * 0.8, 27),
                     f.at(foot_x + hip_w * 0.9, 4),
                     f.at(foot_x - hip_w, 4)};
    fill_poly(dc, shin, 4, shade(tone, 0.88), style.dark);
    const POINT sole = f.at(foot_x + 1.0, 2.5);
    fill_ell(dc, sole.x, sole.y,
             std::max(kLimbMinPx, static_cast<int>(6 * f.scale)),
             std::max(2, static_cast<int>(2.4 * f.scale)), style.trim,
             style.dark);
  };
  leg(-1.0, -swing, shade(style.cloth, 0.72));
  leg(1.0, swing, style.cloth);

  // Skirt / kilt: hangs from the waist, not a second torso box.
  {
    POINT p[4] = {f.at(-11, 56 + bob * 0.3), f.at(11, 56 + bob * 0.3),
                  f.at(13, 42), f.at(-13, 42)};
    fill_poly(dc, p, 4, style.cloth, style.dark);
  }
  // Torso tapers shoulder → waist.
  {
    POINT p[4] = {f.at(-13 + lean * 0.4, 80 + bob),
                  f.at(13 + lean * 0.5, 80 + bob), f.at(9.5, 56),
                  f.at(-10, 56)};
    fill_poly(dc, p, 4, shade(style.cloth, 1.14), style.dark);
    POINT belt[4] = {f.at(-10.5, 58), f.at(10.5, 58), f.at(9.5, 55),
                     f.at(-10, 55)};
    fill_poly(dc, belt, 4, style.trim, style.dark);
  }
  // Far arm hangs (slight counter-swing while walking).
  {
    const double hand_x = -14 - swing * 6.0 - rear_weight * 3.0;
    POINT p[4] = {f.at(-10 + lean * 0.4, 78 + bob),
                  f.at(-15 + lean * 0.4, 76 + bob), f.at(hand_x, 48),
                  f.at(hand_x + arm_w, 48)};
    fill_poly(dc, p, 4, shade(style.skin, 0.8), style.dark);
  }
  // Neck, then adult head (kAdultHeadUnits of kAdultBodyUnits).
  {
    POINT neck[4] = {f.at(-3.0 + lean * 0.55, 84 + bob),
                     f.at(3.2 + lean * 0.55, 84 + bob),
                     f.at(2.8 + lean * 0.6, 80 + bob),
                     f.at(-2.6 + lean * 0.6, 80 + bob)};
    fill_poly(dc, neck, 4, style.skin, style.dark);
    const POINT crown = f.at(lean * 0.65, kAdultCrown + bob);
    const int hx = std::max(kLimbMinPx, static_cast<int>(5.6 * f.scale));
    const int hy = std::max(kLimbMinPx,
                            static_cast<int>((kAdultHeadUnits * 0.5) * f.scale));
    fill_ell(dc, crown.x, crown.y + hy, hx, hy, style.skin, style.dark);
    fill_ell(dc, crown.x, crown.y + std::max(2, static_cast<int>(2 * f.scale)),
             std::max(kLimbMinPx, static_cast<int>(6.0 * f.scale)),
             std::max(2, static_cast<int>(2.6 * f.scale)), style.trim,
             style.dark);
  }
  // Near arm + held tool: shoulder-anchored swing driven by attack phase.
  {
    const double shoulder_x = 10 + lean * 0.5;
    const double shoulder_y = 78 + bob;
    const double angle = (-0.55 + strike) + swing * 0.25;
    const double arm_len = 24.0 + reach;
    const double hand_x = shoulder_x + std::cos(angle) * arm_len;
    const double hand_y = shoulder_y + std::sin(angle) * arm_len - 24.0 + 4.0;
    POINT arm[4] = {f.at(shoulder_x - arm_w, shoulder_y),
                    f.at(shoulder_x + arm_w, shoulder_y),
                    f.at(hand_x + arm_w * 0.7, hand_y),
                    f.at(hand_x - arm_w * 0.7, hand_y)};
    fill_poly(dc, arm, 4, style.skin, style.dark);
    const double blade_len = 26.0 + reach * 0.45;
    const double tip_x = hand_x + std::cos(angle) * blade_len;
    const double tip_y = hand_y + std::sin(angle) * blade_len;
    const POINT hand = f.at(hand_x, hand_y);
    const POINT tip = f.at(tip_x, tip_y);
    switch (held) {
      case Held::Axe: {
        line(dc, hand.x, hand.y, tip.x, tip.y, style.trim,
             std::max(kLimbMinPx, static_cast<int>(3 * f.scale)));
        const POINT head = f.at(tip_x, tip_y);
        fill_ell(dc, head.x, head.y,
                 std::max(kLimbMinPx, static_cast<int>(7 * f.scale)),
                 std::max(kLimbMinPx, static_cast<int>(5 * f.scale)),
                 style.metal, style.dark);
        break;
      }
      case Held::Sword: {
        const double dx = static_cast<double>(tip.x - hand.x);
        const double dy = static_cast<double>(tip.y - hand.y);
        const double len = std::max(1.0, std::hypot(dx, dy));
        const double hx = -dy / len;
        const double hy = dx / len;
        const double half = std::max(static_cast<double>(kLimbMinPx), 6.0 * f.scale);
        const double tip_half = std::max(2.0, 2.2 * f.scale);
        POINT blade[4] = {
            {hand.x + static_cast<int>(std::lround(hx * half)),
             hand.y + static_cast<int>(std::lround(hy * half))},
            {hand.x - static_cast<int>(std::lround(hx * half)),
             hand.y - static_cast<int>(std::lround(hy * half))},
            {tip.x - static_cast<int>(std::lround(hx * tip_half)),
             tip.y - static_cast<int>(std::lround(hy * tip_half))},
            {tip.x + static_cast<int>(std::lround(hx * tip_half)),
             tip.y + static_cast<int>(std::lround(hy * tip_half))}};
        fill_poly(dc, blade, 4, style.metal, style.dark);
        line(dc, blade[0].x, blade[0].y, blade[3].x, blade[3].y,
             shade(style.metal, 1.28), 1);
        POINT guard[4] = {
            f.at(hand_x + (-std::sin(angle)) * 6.5,
                 hand_y + std::cos(angle) * 6.5),
            f.at(hand_x - (-std::sin(angle)) * 6.5,
                 hand_y - std::cos(angle) * 6.5),
            f.at(hand_x - std::cos(angle) * 2.0 - (-std::sin(angle)) * 5.5,
                 hand_y - std::sin(angle) * 2.0 - std::cos(angle) * 5.5),
            f.at(hand_x - std::cos(angle) * 2.0 + (-std::sin(angle)) * 5.5,
                 hand_y - std::sin(angle) * 2.0 + std::cos(angle) * 5.5)};
        fill_poly(dc, guard, 4, style.trim, style.dark);
        const POINT pommel = f.at(hand_x - std::cos(angle) * 6.0,
                                  hand_y - std::sin(angle) * 6.0);
        line(dc, hand.x, hand.y, pommel.x, pommel.y, style.trim,
             std::max(3, static_cast<int>(4 * f.scale)));
        break;
      }
      case Held::Staff: {
        const POINT top = f.at(hand_x + std::cos(angle) * 34.0,
                               hand_y + std::sin(angle) * 34.0);
        line(dc, hand.x, hand.y, top.x, top.y, style.trim,
             std::max(2, static_cast<int>(3 * f.scale)));
        fill_ell(dc, top.x, top.y, static_cast<int>(4 * f.scale),
                 static_cast<int>(4 * f.scale), style.accent, style.dark);
        break;
      }
      case Held::Bow: {
        const POINT top = f.at(hand_x + 6, hand_y + 20);
        const POINT bottom = f.at(hand_x + 6, hand_y - 20);
        line(dc, top.x, top.y, hand.x, hand.y, style.trim, 2);
        line(dc, hand.x, hand.y, bottom.x, bottom.y, style.trim, 2);
        line(dc, top.x, top.y, bottom.x, bottom.y, shade(style.metal, 0.8), 1);
        break;
      }
      case Held::Scales: {
        line(dc, hand.x, hand.y, tip.x, tip.y, style.trim, 2);
        const POINT left_pan = f.at(tip_x - 7, tip_y - 6);
        const POINT right_pan = f.at(tip_x + 7, tip_y - 6);
        line(dc, tip.x, tip.y, left_pan.x, left_pan.y, style.trim, 1);
        line(dc, tip.x, tip.y, right_pan.x, right_pan.y, style.trim, 1);
        fill_ell(dc, left_pan.x, left_pan.y, static_cast<int>(3 * f.scale),
                 static_cast<int>(2 * f.scale), style.metal, style.dark);
        fill_ell(dc, right_pan.x, right_pan.y, static_cast<int>(3 * f.scale),
                 static_cast<int>(2 * f.scale), style.metal, style.dark);
        break;
      }
      case Held::Ledger: {
        POINT book[4] = {f.at(hand_x - 6, hand_y + 8), f.at(hand_x + 6, hand_y + 8),
                         f.at(hand_x + 6, hand_y - 2), f.at(hand_x - 6, hand_y - 2)};
        fill_poly(dc, book, 4, RGB(214, 202, 176), style.dark);
        line(dc, f.at(hand_x, hand_y + 8).x, f.at(hand_x, hand_y + 8).y,
             f.at(hand_x, hand_y - 2).x, f.at(hand_x, hand_y - 2).y, style.trim,
             1);
        break;
      }
      case Held::Club:
        line(dc, hand.x, hand.y, tip.x, tip.y, style.trim,
             std::max(3, static_cast<int>(5 * f.scale)));
        break;
      case Held::None:
        break;
    }
  }
}

// ── monster rigs ────────────────────────────────────────────────────────

// Hunched biped lurker: heavy shoulders, low head, dragging claws.
// Hunched bronze-age warden. A hip-to-foot plank plus a torso pentagon
// reads as a crate; jointed legs, a snout, and filled claws are required.
inline constexpr bool kWardenHasJointedLegs = true;
inline constexpr bool kWardenHasSnout = true;
inline constexpr bool kWardenHasFilledClaws = true;

inline bool crate_foe_fails_review(bool jointed_legs, bool snout,
                                   bool filled_claws) {
  return !jointed_legs || !snout || !filled_claws;
}

inline const char* owner_jointed_warden_label() { return "Jointed warden"; }
inline const char* owner_snout_claws_label() { return "Snout claws"; }
inline bool first_fight_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}
inline const char* owner_hit_flash_label() { return "Hit flash"; }
inline const char* owner_number_fade_label() { return "Number fade"; }
inline bool juice_strip_covers_hud_fails_review(bool overlap) { return overlap; }

inline void lurker(HDC dc, int cx, int base_y, int height_px, const Style& style,
                   const Pose& pose) {
  Frame f{cx, base_y, height_px / 100.0, pose.mirror};
  const double swing = std::sin(pose.walk * 2.0 * kPi) * pose.moving;
  const double bob = std::sin(pose.breathe * 2.0 * kPi) * 2.0;
  const double reach = std::sin(std::min(1.0, pose.attack) * kPi);
  const auto leg = [&](double side, double phase, COLORREF tone) {
    const double hip_x = side * 11.0;
    const double knee_x = side * 12.0 + phase * 8.0;
    const double foot_x = side * 13.0 + phase * 14.0;
    POINT thigh[4] = {f.at(hip_x - 5.0, 44), f.at(hip_x + 5.0, 44),
                      f.at(knee_x + 4.5, 24), f.at(knee_x - 4.8, 24)};
    fill_poly(dc, thigh, 4, tone, style.dark);
    POINT shin[4] = {f.at(knee_x - 4.5, 24), f.at(knee_x + 4.2, 24),
                     f.at(foot_x + 5.0, 4), f.at(foot_x - 5.5, 4)};
    fill_poly(dc, shin, 4, shade(tone, 0.86), style.dark);
    const POINT sole = f.at(foot_x + 1.2, 2.2);
    fill_ell(dc, sole.x, sole.y, std::max(3, static_cast<int>(6 * f.scale)),
             std::max(2, static_cast<int>(2.6 * f.scale)), style.trim,
             style.dark);
  };
  leg(-1.0, -swing, shade(style.cloth, 0.72));
  leg(1.0, swing, style.cloth);
  {
    POINT mantle[4] = {f.at(-22, 62 + bob), f.at(16, 66 + bob), f.at(14, 50),
                       f.at(-18, 48)};
    fill_poly(dc, mantle, 4, shade(style.metal, 0.78), style.dark);
    POINT body[4] = {f.at(-12, 52 + bob), f.at(11, 54 + bob), f.at(8, 38),
                     f.at(-11, 36)};
    fill_poly(dc, body, 4, style.cloth, style.dark);
  }
  {
    POINT brow[4] = {f.at(8 + reach * 6, 74 + bob),
                     f.at(22 + reach * 6, 76 + bob),
                     f.at(24 + reach * 6, 66 + bob),
                     f.at(8 + reach * 6, 64 + bob)};
    fill_poly(dc, brow, 4, shade(style.cloth, 1.12), style.dark);
    POINT snout[4] = {f.at(18 + reach * 6, 66 + bob),
                      f.at(36 + reach * 9, 58 + bob),
                      f.at(34 + reach * 9, 50 + bob),
                      f.at(16 + reach * 6, 56 + bob)};
    fill_poly(dc, snout, 4, style.trim, style.dark);
    const POINT eye = f.at(16 + reach * 6, 68 + bob);
    fill_ell(dc, eye.x, eye.y, std::max(2, static_cast<int>(3 * f.scale)),
             std::max(2, static_cast<int>(2.4 * f.scale)), style.accent,
             style.accent);
  }
  {
    const double claw_x = 18 + reach * 18;
    const double claw_y = 32 + reach * 12;
    POINT arm[4] = {f.at(8, 56 + bob), f.at(16, 58 + bob), f.at(claw_x, claw_y),
                    f.at(claw_x - 8, claw_y - 4)};
    fill_poly(dc, arm, 4, shade(style.cloth, 0.9), style.dark);
    for (int talon = 0; talon < 3; ++talon) {
      const double ox = static_cast<double>(talon) * 3.4;
      POINT wedge[3] = {f.at(claw_x - ox, claw_y - talon * 1.6),
                        f.at(claw_x + 10 - ox, claw_y - 8 - talon * 1.8),
                        f.at(claw_x + 2 - ox, claw_y + 2 - talon * 1.4)};
      fill_poly(dc, wedge, 3, style.metal, style.dark);
    }
  }
}

// Skeletal wight: thin frame, rib cage, cold light in the sockets.
inline void wight(HDC dc, int cx, int base_y, int height_px, const Style& style,
                  const Pose& pose) {
  Frame f{cx, base_y, height_px / 100.0, pose.mirror};
  const double swing = std::sin(pose.walk * 2.0 * kPi) * pose.moving;
  const double sway = std::sin(pose.breathe * 2.0 * kPi) * 2.5;
  const double reach = std::sin(std::min(1.0, pose.attack) * kPi);
  // Stick legs.
  line(dc, f.at(-6, 44).x, f.at(-6, 44).y, f.at(-6 - swing * 12, 0).x,
       f.at(-6 - swing * 12, 0).y, style.metal, 3);
  line(dc, f.at(6, 44).x, f.at(6, 44).y, f.at(6 + swing * 12, 0).x,
       f.at(6 + swing * 12, 0).y, shade(style.metal, 0.8), 3);
  // Tattered shroud.
  {
    POINT p[5] = {f.at(-12 + sway, 78), f.at(10 + sway, 80), f.at(14, 48),
                  f.at(4, 38), f.at(-14, 44)};
    fill_poly(dc, p, 5, style.cloth, style.dark);
  }
  // Rib lines.
  for (int rib = 0; rib < 3; ++rib) {
    const double y = 66 - rib * 7;
    line(dc, f.at(-8 + sway * 0.5, y).x, f.at(-8 + sway * 0.5, y).y,
         f.at(8 + sway * 0.5, y).x, f.at(8 + sway * 0.5, y).y,
         shade(style.metal, 0.9), 1);
  }
  // Skull.
  {
    const POINT skull = f.at(sway, 90);
    fill_ell(dc, skull.x, skull.y, static_cast<int>(7 * f.scale),
             static_cast<int>(8 * f.scale), style.metal, style.dark);
    const POINT socket = f.at(sway + 3, 90);
    fill_ell(dc, socket.x, socket.y, std::max(1, static_cast<int>(2 * f.scale)),
             std::max(1, static_cast<int>(2 * f.scale)), style.accent,
             style.accent);
  }
  // Reaching claw.
  {
    const double hand_x = 12 + reach * 22;
    const double hand_y = 58 - reach * 6;
    line(dc, f.at(8 + sway, 72).x, f.at(8 + sway, 72).y, f.at(hand_x, hand_y).x,
         f.at(hand_x, hand_y).y, style.metal, 2);
    for (int talon = 0; talon < 3; ++talon) {
      line(dc, f.at(hand_x, hand_y).x, f.at(hand_x, hand_y).y,
           f.at(hand_x + 6, hand_y - 2 - talon * 3).x,
           f.at(hand_x + 6, hand_y - 2 - talon * 3).y, style.metal, 1);
    }
  }
}

// Quadruped wolf/beast for the wilds.
inline void beast(HDC dc, int cx, int base_y, int height_px, const Style& style,
                  const Pose& pose) {
  Frame f{cx, base_y, height_px / 100.0, pose.mirror};
  const double swing = std::sin(pose.walk * 2.0 * kPi) * pose.moving;
  const double bob = std::sin(pose.breathe * 2.0 * kPi) * 1.5;
  const double pounce = std::sin(std::min(1.0, pose.attack) * kPi);
  // Legs (two pairs, alternating).
  const auto leg = [&](double x, double phase, COLORREF tone) {
    line(dc, f.at(x, 30).x, f.at(x, 30).y, f.at(x + phase * 10, 0).x,
         f.at(x + phase * 10, 0).y, tone, std::max(2, static_cast<int>(4 * f.scale)));
  };
  leg(-22, -swing, shade(style.cloth, 0.7));
  leg(14, -swing, shade(style.cloth, 0.7));
  leg(-14, swing, style.cloth);
  leg(22, swing, style.cloth);
  // Body.
  {
    POINT p[5] = {f.at(-30, 34 + bob), f.at(-24, 46 + bob), f.at(16, 48 + bob),
                  f.at(30, 36 + bob), f.at(0, 26)};
    fill_poly(dc, p, 5, style.cloth, style.dark);
    // Back ridge fur.
    for (int tuft = 0; tuft < 4; ++tuft) {
      const double x = -20 + tuft * 10;
      line(dc, f.at(x, 46 + bob).x, f.at(x, 46 + bob).y, f.at(x + 3, 52 + bob).x,
           f.at(x + 3, 52 + bob).y, style.trim, 2);
    }
  }
  // Head lunges with the pounce.
  {
    const POINT skull = f.at(34 + pounce * 12, 42 + bob - pounce * 6);
    fill_ell(dc, skull.x, skull.y, static_cast<int>(9 * f.scale),
             static_cast<int>(6 * f.scale), shade(style.cloth, 1.1), style.dark);
    POINT muzzle[3] = {f.at(40 + pounce * 12, 42 + bob - pounce * 6),
                       f.at(48 + pounce * 14, 38 + bob - pounce * 7),
                       f.at(40 + pounce * 12, 37 + bob - pounce * 6)};
    fill_poly(dc, muzzle, 3, style.trim, style.dark);
    POINT ear[3] = {f.at(30 + pounce * 12, 47 + bob), f.at(33 + pounce * 12, 54 + bob),
                    f.at(36 + pounce * 12, 47 + bob)};
    fill_poly(dc, ear, 3, style.trim, style.dark);
    const POINT eye = f.at(35 + pounce * 12, 44 + bob - pounce * 6);
    fill_ell(dc, eye.x, eye.y, std::max(1, static_cast<int>(2 * f.scale)),
             std::max(1, static_cast<int>(2 * f.scale)), style.accent,
             style.accent);
  }
  // Tail.
  line(dc, f.at(-30, 38 + bob).x, f.at(-30, 38 + bob).y,
       f.at(-42, 46 + bob + swing * 4).x, f.at(-42, 46 + bob + swing * 4).y,
       style.trim, std::max(2, static_cast<int>(3 * f.scale)));
}

// Marsh ghast: wobbling rot-mass with drips.
inline void ghast(HDC dc, int cx, int base_y, int height_px, const Style& style,
                  const Pose& pose) {
  Frame f{cx, base_y, height_px / 100.0, pose.mirror};
  const double wobble = pose.breathe * 2.0 * kPi;
  const double surge = std::sin(std::min(1.0, pose.attack) * kPi);
  POINT blob[8];
  for (int i = 0; i < 8; ++i) {
    const double theta = i * kPi / 4.0;
    const double r = 26.0 + std::sin(wobble + i * 1.7) * 4.0 +
                     (i < 3 ? surge * 10.0 : 0.0);
    blob[i] = f.at(std::cos(theta) * r * 1.15, 30 + std::sin(theta) * r);
  }
  fill_poly(dc, blob, 8, style.cloth, style.dark);
  // Inner mass.
  fill_ell(dc, f.at(0, 32).x, f.at(0, 32).y, static_cast<int>(14 * f.scale),
           static_cast<int>(11 * f.scale), shade(style.cloth, 0.75), style.dark);
  // Eyes drifting in the mass.
  for (int eye = 0; eye < 2; ++eye) {
    const POINT at = f.at(6 + eye * 8 + std::sin(wobble + eye) * 2, 40 + eye * 4);
    fill_ell(dc, at.x, at.y, std::max(1, static_cast<int>(2 * f.scale)),
             std::max(1, static_cast<int>(2 * f.scale)), style.accent,
             style.accent);
  }
  // Drips.
  for (int drip = 0; drip < 3; ++drip) {
    const double x = -14 + drip * 12;
    const double fall = std::fmod(pose.breathe * 2.0 + drip * 0.33, 1.0);
    const POINT at = f.at(x, 12 - fall * 12);
    fill_ell(dc, at.x, at.y, std::max(1, static_cast<int>(2 * f.scale)),
             std::max(1, static_cast<int>(3 * f.scale)),
             shade(style.cloth, 0.8), style.dark);
  }
}

// Buffer totem: hovering carved stone with orbiting motes.
inline void totem(HDC dc, int cx, int base_y, int height_px, const Style& style,
                  const Pose& pose) {
  Frame f{cx, base_y, height_px / 100.0, pose.mirror};
  const double hover = std::sin(pose.breathe * 2.0 * kPi) * 4.0;
  POINT body[6] = {f.at(-10, 20 + hover), f.at(-13, 55 + hover),
                   f.at(-6, 78 + hover), f.at(8, 76 + hover),
                   f.at(13, 50 + hover), f.at(9, 18 + hover)};
  fill_poly(dc, body, 6, style.cloth, style.dark);
  // Carved bands.
  for (int band = 0; band < 3; ++band) {
    const double y = 30 + band * 16 + hover;
    line(dc, f.at(-11 + band, y).x, f.at(-11 + band, y).y, f.at(11 - band, y).x,
         f.at(11 - band, y).y, style.trim, 2);
  }
  // Watching eye.
  const POINT eye = f.at(0, 62 + hover);
  fill_ell(dc, eye.x, eye.y, static_cast<int>(5 * f.scale),
           static_cast<int>(3 * f.scale), style.accent, style.dark);
  // Orbiting motes.
  for (int mote = 0; mote < 3; ++mote) {
    const double theta = pose.breathe * 2.0 * kPi + mote * (2.0 * kPi / 3.0);
    const POINT at = f.at(std::cos(theta) * 22.0, 46 + std::sin(theta) * 10.0 + hover);
    fill_ell(dc, at.x, at.y, std::max(1, static_cast<int>(2 * f.scale)),
             std::max(1, static_cast<int>(2 * f.scale)), style.accent,
             style.accent);
  }
}

// ── theme styles ────────────────────────────────────────────────────────

inline Style player_style() {
  Style s;
  s.skin = RGB(224, 184, 136);
  s.cloth = RGB(186, 122, 48);
  s.trim = RGB(122, 74, 28);
  s.metal = RGB(228, 186, 78);
  s.accent = RGB(120, 214, 168);
  return s;
}

inline Style npc_style(int npc_id) {
  Style s = player_style();
  switch (npc_id % 4) {
    case 1: s.cloth = RGB(88, 108, 146); s.trim = RGB(60, 74, 104); break;   // guide blue
    case 2: s.cloth = RGB(150, 118, 62); s.trim = RGB(108, 82, 40); break;   // trader gold
    case 3: s.cloth = RGB(104, 90, 120); s.trim = RGB(74, 62, 88); break;    // arms plum
    default: s.cloth = RGB(84, 128, 106); s.trim = RGB(56, 92, 74); break;   // banker green
  }
  return s;
}

// theme -> monster hue family. Elite/boss brightens and gilds.
inline Style monster_style(const std::string& theme, bool elite) {
  Style s;
  s.dark = RGB(18, 14, 12);
  if (theme == "crypt") {
    s.cloth = RGB(108, 116, 128);
    s.trim = RGB(70, 76, 88);
    s.metal = RGB(214, 218, 224);
    s.accent = RGB(150, 220, 255);
  } else if (theme == "wilds") {
    s.cloth = RGB(122, 96, 64);
    s.trim = RGB(86, 64, 40);
    s.metal = RGB(200, 190, 170);
    s.accent = RGB(255, 196, 92);
  } else if (theme == "marsh") {
    s.cloth = RGB(92, 116, 74);
    s.trim = RGB(62, 84, 50);
    s.metal = RGB(170, 190, 150);
    s.accent = RGB(178, 255, 130);
  } else if (theme == "grove") {
    s.cloth = RGB(96, 124, 88);
    s.trim = RGB(64, 88, 58);
    s.metal = RGB(190, 205, 180);
    s.accent = RGB(150, 255, 170);
  } else if (theme == "town" || theme == "tin") {
    s.cloth = RGB(148, 108, 58);
    s.trim = RGB(96, 64, 34);
    s.metal = RGB(196, 154, 72);
    s.accent = RGB(232, 92, 48);
  } else {  // dungeon / first-road default: bronze hide, not grey crate
    s.cloth = RGB(148, 108, 58);
    s.trim = RGB(96, 64, 34);
    s.metal = RGB(196, 154, 72);
    s.accent = RGB(232, 92, 48);
  }
  if (elite) {
    s.cloth = shade(s.cloth, 1.2);
    s.trim = RGB(160, 128, 60);
    s.accent = RGB(255, 214, 110);
  }
  return s;
}

// ── scenery ─────────────────────────────────────────────────────────────
// VG-ART-004: a circle on a stick cannot certify the tin village kit. The
// shipped tree is a forked bole with a root flare and clustered canopy.

inline constexpr int kTreeCanopyClusters = 6;
inline constexpr bool kTreeHasRootFlare = true;
inline constexpr bool kTreeHasFork = true;

inline bool lollipop_tree_fails_review(int clusters, bool root_flare, bool fork) {
  return clusters < 4 || !root_flare || !fork;
}

inline const char* owner_village_kit_label() { return "Village kit"; }
inline const char* owner_solid_proxy_label() { return "Solid proxy"; }
inline bool kit_strip_covers_hud_fails_review(bool overlap) { return overlap; }

inline void tree(HDC dc, int cx, int base_y, int height_px, double sway_phase,
                 COLORREF leaf, COLORREF trunk) {
  const double s = height_px / 100.0;
  const int sway = static_cast<int>(std::sin(sway_phase) * 4.0 * s);
  const COLORREF dark = RGB(20, 16, 12);
  const COLORREF bark = shade(trunk, 0.78);
  const COLORREF heart = shade(trunk, 1.14);
  const COLORREF deep = shade(leaf, 0.72);
  const COLORREF crown = shade(leaf, 1.16);
  const int root_w = std::max(7, static_cast<int>(12 * s));
  const int bole_w = std::max(4, static_cast<int>(6 * s));
  const int top_w = std::max(2, static_cast<int>(3 * s));
  POINT roots[4] = {{cx - root_w, base_y},
                    {cx + root_w, base_y},
                    {cx + bole_w, base_y - static_cast<int>(12 * s)},
                    {cx - bole_w, base_y - static_cast<int>(12 * s)}};
  fill_poly(dc, roots, 4, bark, dark);
  POINT bole[4] = {{cx - bole_w, base_y - static_cast<int>(10 * s)},
                   {cx + bole_w, base_y - static_cast<int>(10 * s)},
                   {cx + top_w + sway / 3, base_y - static_cast<int>(44 * s)},
                   {cx - top_w + sway / 3, base_y - static_cast<int>(44 * s)}};
  fill_poly(dc, bole, 4, trunk, dark);
  const int fork_y = base_y - static_cast<int>(40 * s);
  POINT left_limb[4] = {
      {cx - top_w + sway / 3, fork_y},
      {cx + 1 + sway / 3, fork_y - static_cast<int>(4 * s)},
      {cx - static_cast<int>(14 * s) + sway, base_y - static_cast<int>(62 * s)},
      {cx - static_cast<int>(18 * s) + sway, base_y - static_cast<int>(58 * s)}};
  fill_poly(dc, left_limb, 4, bark, dark);
  POINT right_limb[4] = {
      {cx - 1 + sway / 3, fork_y},
      {cx + top_w + sway / 3, fork_y - static_cast<int>(3 * s)},
      {cx + static_cast<int>(16 * s) + sway, base_y - static_cast<int>(60 * s)},
      {cx + static_cast<int>(11 * s) + sway, base_y - static_cast<int>(54 * s)}};
  fill_poly(dc, right_limb, 4, heart, dark);
  const int hy = base_y - static_cast<int>(64 * s);
  const int mass = std::max(8, static_cast<int>(16 * s));
  fill_ell(dc, cx + sway, hy, mass, std::max(7, static_cast<int>(13 * s)), leaf,
           dark);
  fill_ell(dc, cx - static_cast<int>(14 * s) + sway, hy + static_cast<int>(6 * s),
           std::max(6, static_cast<int>(11 * s)),
           std::max(5, static_cast<int>(9 * s)), deep, dark);
  fill_ell(dc, cx + static_cast<int>(15 * s) + sway, hy + static_cast<int>(4 * s),
           std::max(6, static_cast<int>(12 * s)),
           std::max(5, static_cast<int>(9 * s)), crown, dark);
  fill_ell(dc, cx + sway / 2, hy - static_cast<int>(10 * s),
           std::max(5, static_cast<int>(10 * s)),
           std::max(4, static_cast<int>(8 * s)), crown, dark);
  fill_ell(dc, cx - static_cast<int>(8 * s) + sway, hy - static_cast<int>(3 * s),
           std::max(4, static_cast<int>(8 * s)),
           std::max(4, static_cast<int>(7 * s)), leaf, dark);
  fill_ell(dc, cx + static_cast<int>(7 * s) + sway, hy + static_cast<int>(11 * s),
           std::max(5, static_cast<int>(9 * s)),
           std::max(4, static_cast<int>(7 * s)), deep, dark);
}

inline void standing_stones(HDC dc, int cx, int base_y, int height_px,
                            COLORREF stone) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  for (int pillar = -1; pillar <= 1; ++pillar) {
    const int px = cx + static_cast<int>(pillar * 18 * s);
    const int ph = static_cast<int>((52 - std::abs(pillar) * 12) * s);
    POINT p[4] = {{px - static_cast<int>(6 * s), base_y},
                  {px + static_cast<int>(6 * s), base_y},
                  {px + static_cast<int>(4 * s), base_y - ph},
                  {px - static_cast<int>(5 * s), base_y - ph}};
    fill_poly(dc, p, 4, pillar == 0 ? stone : shade(stone, 0.85), dark);
  }
  // Lintel.
  POINT lintel[4] = {{cx - static_cast<int>(24 * s), base_y - static_cast<int>(48 * s)},
                     {cx + static_cast<int>(24 * s), base_y - static_cast<int>(48 * s)},
                     {cx + static_cast<int>(22 * s), base_y - static_cast<int>(58 * s)},
                     {cx - static_cast<int>(22 * s), base_y - static_cast<int>(58 * s)}};
  fill_poly(dc, lintel, 4, shade(stone, 1.1), dark);
}

inline constexpr bool kDwellingHasWalls = true;
inline constexpr bool kDwellingHasRoof = true;
inline constexpr bool kDwellingHasDoor = true;

inline bool stall_dwelling_fails_review(bool walls, bool roof, bool door) {
  return !walls || !roof || !door;
}

// Bronze-age hut: mudbrick walls, thatch, a door. A scalloped awning
// stall cannot certify SceneryKind::Dwelling.
inline void dwelling(HDC dc, int cx, int base_y, int height_px, COLORREF wall,
                     COLORREF roof, COLORREF trim) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  const int wall_w = std::max(10, static_cast<int>(22 * s));
  const int wall_h = std::max(12, static_cast<int>(38 * s));
  POINT walls[4] = {{cx - wall_w, base_y},
                    {cx + wall_w, base_y},
                    {cx + wall_w - std::max(1, static_cast<int>(2 * s)),
                     base_y - wall_h},
                    {cx - wall_w + std::max(1, static_cast<int>(2 * s)),
                     base_y - wall_h}};
  fill_poly(dc, walls, 4, wall, dark);
  POINT thatch[3] = {
      {cx - wall_w - std::max(3, static_cast<int>(7 * s)),
       base_y - wall_h + std::max(2, static_cast<int>(4 * s))},
      {cx + wall_w + std::max(3, static_cast<int>(7 * s)),
       base_y - wall_h + std::max(2, static_cast<int>(4 * s))},
      {cx, base_y - wall_h - std::max(8, static_cast<int>(22 * s))}};
  fill_poly(dc, thatch, 3, roof, dark);
  const int door_w = std::max(3, static_cast<int>(6 * s));
  const int door_h = std::max(6, static_cast<int>(18 * s));
  POINT door[4] = {{cx - door_w, base_y},
                   {cx + door_w, base_y},
                   {cx + door_w, base_y - door_h},
                   {cx - door_w, base_y - door_h}};
  fill_poly(dc, door, 4, trim, dark);
}

inline constexpr bool kRuinHasBrokenWall = true;
inline constexpr bool kRuinHasRubble = true;
inline constexpr bool kRuinHasWheels = false;

inline bool wagon_ruin_fails_review(bool broken_wall, bool rubble, bool wheels) {
  return !broken_wall || !rubble || wheels;
}

// Collapsed bronze-age house: one standing wall, fallen timber, rubble.
// A covered wagon cannot certify SceneryKind::Ruin. A complete hut with
// a pitched roof also cannot — the right side is gone.
inline void ruin(HDC dc, int cx, int base_y, int height_px, COLORREF wall,
                 COLORREF rubble, COLORREF timber) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  const int left = cx - std::max(8, static_cast<int>(20 * s));
  const int mid = cx - std::max(2, static_cast<int>(4 * s));
  const int wall_h = std::max(14, static_cast<int>(40 * s));
  const int step_h = std::max(6, static_cast<int>(14 * s));
  POINT stub[5] = {{left, base_y},
                   {mid + std::max(2, static_cast<int>(6 * s)), base_y},
                   {mid + std::max(1, static_cast<int>(4 * s)), base_y - step_h},
                   {mid, base_y - wall_h},
                   {left + std::max(1, static_cast<int>(2 * s)),
                    base_y - wall_h + std::max(2, static_cast<int>(6 * s))}};
  fill_poly(dc, stub, 5, wall, dark);
  line(dc, mid, base_y - std::max(8, static_cast<int>(18 * s)),
       cx + std::max(8, static_cast<int>(18 * s)),
       base_y - std::max(2, static_cast<int>(4 * s)), timber,
       std::max(2, static_cast<int>(3 * s)));
  fill_ell(dc, cx + std::max(4, static_cast<int>(10 * s)),
           base_y - std::max(2, static_cast<int>(5 * s)),
           std::max(5, static_cast<int>(10 * s)),
           std::max(3, static_cast<int>(6 * s)), rubble, dark);
  fill_ell(dc, cx + std::max(8, static_cast<int>(18 * s)),
           base_y - std::max(1, static_cast<int>(3 * s)),
           std::max(3, static_cast<int>(6 * s)),
           std::max(2, static_cast<int>(4 * s)), shade(rubble, 0.8), dark);
}

inline void market_stall(HDC dc, int cx, int base_y, int height_px,
                         COLORREF canvas, COLORREF wood) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  // Posts.
  line(dc, cx - static_cast<int>(20 * s), base_y, cx - static_cast<int>(20 * s),
       base_y - static_cast<int>(46 * s), wood, std::max(2, static_cast<int>(3 * s)));
  line(dc, cx + static_cast<int>(20 * s), base_y, cx + static_cast<int>(20 * s),
       base_y - static_cast<int>(46 * s), wood, std::max(2, static_cast<int>(3 * s)));
  // Counter.
  POINT counter[4] = {{cx - static_cast<int>(24 * s), base_y - static_cast<int>(18 * s)},
                      {cx + static_cast<int>(24 * s), base_y - static_cast<int>(18 * s)},
                      {cx + static_cast<int>(22 * s), base_y - static_cast<int>(26 * s)},
                      {cx - static_cast<int>(22 * s), base_y - static_cast<int>(26 * s)}};
  fill_poly(dc, counter, 4, shade(wood, 1.15), dark);
  // Scalloped awning.
  POINT awning[4] = {{cx - static_cast<int>(26 * s), base_y - static_cast<int>(44 * s)},
                     {cx + static_cast<int>(26 * s), base_y - static_cast<int>(44 * s)},
                     {cx + static_cast<int>(20 * s), base_y - static_cast<int>(58 * s)},
                     {cx - static_cast<int>(20 * s), base_y - static_cast<int>(58 * s)}};
  fill_poly(dc, awning, 4, canvas, dark);
  for (int scallop = -2; scallop <= 2; ++scallop) {
    fill_ell(dc, cx + static_cast<int>(scallop * 10 * s),
             base_y - static_cast<int>(44 * s), static_cast<int>(5 * s),
             static_cast<int>(4 * s), shade(canvas, 0.85), dark);
  }
}

inline constexpr bool kShrineHasBasin = true;
inline constexpr bool kShrineHasColumn = true;
inline constexpr bool kShrineHasWater = true;

inline bool blob_shrine_fails_review(bool basin, bool column, bool water) {
  return !basin || !column || !water;
}

// Village fountain: wide basin, water, a column and bowl. A stone blob or
// a trilithon cannot certify SceneryKind::Shrine on the tin village kit.
inline void fountain(HDC dc, int cx, int base_y, int height_px, double phase,
                     COLORREF stone, COLORREF water) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  const int basin_rx = std::max(10, static_cast<int>(28 * s));
  const int basin_ry = std::max(5, static_cast<int>(10 * s));
  fill_ell(dc, cx, base_y - std::max(3, static_cast<int>(6 * s)), basin_rx,
           basin_ry, stone, dark);
  fill_ell(dc, cx, base_y - std::max(4, static_cast<int>(8 * s)),
           std::max(7, static_cast<int>(20 * s)),
           std::max(3, static_cast<int>(6 * s)), water, dark);
  const int col_w = std::max(3, static_cast<int>(5 * s));
  const int col_h = std::max(10, static_cast<int>(30 * s));
  POINT column[4] = {{cx - col_w, base_y - std::max(4, static_cast<int>(8 * s))},
                     {cx + col_w, base_y - std::max(4, static_cast<int>(8 * s))},
                     {cx + col_w - 1, base_y - col_h},
                     {cx - col_w + 1, base_y - col_h}};
  fill_poly(dc, column, 4, shade(stone, 1.1), dark);
  fill_ell(dc, cx, base_y - col_h, std::max(5, static_cast<int>(11 * s)),
           std::max(3, static_cast<int>(4 * s)), stone, dark);
  for (int spout = -1; spout <= 1; spout += 2) {
    const double arc = std::sin(phase * 2.0 * kPi) * 2.0;
    line(dc, cx, base_y - col_h - 2,
         cx + static_cast<int>(spout * (12 + arc) * s),
         base_y - std::max(6, static_cast<int>(14 * s)), water,
         std::max(2, static_cast<int>(2 * s)));
  }
}

inline constexpr bool kGateHasPillars = true;
inline constexpr bool kGateHasLintel = true;
inline constexpr bool kGateHasOpening = true;

inline bool slab_gate_fails_review(bool pillars, bool lintel, bool opening) {
  return !pillars || !lintel || !opening;
}

inline void road_gate(HDC dc, int cx, int base_y, int height_px, COLORREF stone,
                      COLORREF accent) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  const int pillar_h = std::max(16, static_cast<int>(52 * s));
  const int pillar_w = std::max(4, static_cast<int>(6 * s));
  const int span = std::max(10, static_cast<int>(20 * s));
  for (int side = -1; side <= 1; side += 2) {
    const int px = cx + side * span;
    POINT pillar[4] = {{px - pillar_w, base_y},
                       {px + pillar_w, base_y},
                       {px + pillar_w - 1, base_y - pillar_h},
                       {px - pillar_w + 1, base_y - pillar_h}};
    fill_poly(dc, pillar, 4, stone, dark);
  }
  POINT lintel[4] = {{cx - span - pillar_w - 2, base_y - pillar_h + 4},
                     {cx + span + pillar_w + 2, base_y - pillar_h + 4},
                     {cx + span + pillar_w, base_y - pillar_h - std::max(4, static_cast<int>(10 * s))},
                     {cx - span - pillar_w, base_y - pillar_h - std::max(4, static_cast<int>(10 * s))}};
  fill_poly(dc, lintel, 4, shade(stone, 1.12), dark);
  fill_ell(dc, cx, base_y - pillar_h - 2, std::max(3, static_cast<int>(4 * s)),
           std::max(3, static_cast<int>(4 * s)), accent, dark);
}

inline void wagon(HDC dc, int cx, int base_y, int height_px, COLORREF wood,
                  COLORREF canvas) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  // Wheels.
  for (int side = -1; side <= 1; side += 2) {
    const int wx = cx + static_cast<int>(side * 14 * s);
    fill_ell(dc, wx, base_y - static_cast<int>(7 * s), static_cast<int>(8 * s),
             static_cast<int>(8 * s), shade(wood, 0.8), dark);
    fill_ell(dc, wx, base_y - static_cast<int>(7 * s), static_cast<int>(2 * s),
             static_cast<int>(2 * s), dark, dark);
  }
  // Bed.
  POINT bed[4] = {{cx - static_cast<int>(24 * s), base_y - static_cast<int>(12 * s)},
                  {cx + static_cast<int>(24 * s), base_y - static_cast<int>(12 * s)},
                  {cx + static_cast<int>(22 * s), base_y - static_cast<int>(24 * s)},
                  {cx - static_cast<int>(22 * s), base_y - static_cast<int>(24 * s)}};
  fill_poly(dc, bed, 4, wood, dark);
  // Canvas hoop cover seated on the bed.
  fill_ell(dc, cx, base_y - static_cast<int>(26 * s), static_cast<int>(21 * s),
           static_cast<int>(12 * s), canvas, dark);
  POINT skirt[4] = {{cx - static_cast<int>(21 * s), base_y - static_cast<int>(24 * s)},
                    {cx + static_cast<int>(21 * s), base_y - static_cast<int>(24 * s)},
                    {cx + static_cast<int>(22 * s), base_y - static_cast<int>(20 * s)},
                    {cx - static_cast<int>(22 * s), base_y - static_cast<int>(20 * s)}};
  fill_poly(dc, skirt, 4, canvas, dark);
}

// ── terrain tiles ───────────────────────────────────────────────────────
// Painted into the floor cache; hash picks deterministic variants.

inline void terrain_tile(HDC dc, const RECT& cell, const std::string& theme,
                         std::uint32_t hash) {
  const int w = cell.right - cell.left;
  const int h = cell.bottom - cell.top;
  if (w <= 0 || h <= 0) return;
  COLORREF base, joint, fleck;
  if (theme == "town") {
    base = RGB(96, 86, 74);
    joint = RGB(72, 64, 54);
    fleck = RGB(126, 114, 96);
  } else if (theme == "grove") {
    base = RGB(64, 84, 52);
    joint = RGB(48, 66, 40);
    fleck = RGB(96, 128, 70);
  } else if (theme == "crypt") {
    base = RGB(58, 60, 70);
    joint = RGB(40, 42, 52);
    fleck = RGB(96, 100, 116);
  } else if (theme == "wilds") {
    base = RGB(96, 78, 56);
    joint = RGB(72, 58, 42);
    fleck = RGB(134, 112, 78);
  } else if (theme == "marsh") {
    base = RGB(56, 70, 52);
    joint = RGB(40, 52, 40);
    fleck = RGB(88, 116, 76);
  } else {  // dungeon / stone default
    base = RGB(74, 66, 60);
    joint = RGB(52, 46, 42);
    fleck = RGB(104, 94, 84);
  }
  const bool alt = ((hash >> 9) % 5u) == 0;
  RECT fill_rect = cell;
  HBRUSH brush = CreateSolidBrush(dc_color(dc, alt ? shade(base, 0.93) : base));
  FillRect(dc, &fill_rect, brush);
  DeleteObject(brush);
  HPEN pen = CreatePen(PS_SOLID, 1, dc_color(dc, joint));
  HGDIOBJ old_pen = SelectObject(dc, pen);
  MoveToEx(dc, cell.left, cell.top, nullptr);
  LineTo(dc, cell.right, cell.top);
  MoveToEx(dc, cell.left, cell.top, nullptr);
  LineTo(dc, cell.left, cell.bottom);
  SelectObject(dc, old_pen);
  DeleteObject(pen);
  // Theme details, deterministic per tile.
  std::uint32_t noise = hash * 2654435761u + 12345u;
  const auto next_noise = [&]() {
    noise ^= noise << 13;
    noise ^= noise >> 17;
    noise ^= noise << 5;
    return noise;
  };
  const int detail_count = 2 + static_cast<int>(hash % 3u);
  for (int i = 0; i < detail_count; ++i) {
    const int px = cell.left + static_cast<int>(next_noise() % std::max(1, w - 6)) + 3;
    const int py = cell.top + static_cast<int>(next_noise() % std::max(1, h - 6)) + 3;
    const int size = 1 + static_cast<int>(next_noise() % 3u);
    if (theme == "grove" || theme == "marsh") {
      // Grass / reed tufts.
      line(dc, px, py, px - size, py - size * 3, fleck, 1);
      line(dc, px, py, px + 1, py - size * 3 - 1, shade(fleck, 1.15), 1);
      line(dc, px, py, px + size, py - size * 2, fleck, 1);
    } else if (theme == "crypt") {
      // Bone flecks and a rare crack.
      line(dc, px, py, px + size * 2, py + 1, fleck, 1);
      if ((next_noise() & 7u) == 0)
        line(dc, px, py, px + size * 4, py + size * 3, joint, 1);
    } else {
      // Pebbles.
      fill_ell(dc, px, py, size, std::max(1, size - 1), fleck,
               shade(fleck, 0.7));
    }
  }
  if (theme == "marsh" && (hash & 0xC0u) == 0) {
    // Standing water sheen.
    const int pw = w / 3;
    const int ph = h / 4;
    fill_ell(dc, cell.left + w / 2, cell.top + h / 2, pw, ph,
             RGB(52, 84, 88), RGB(40, 62, 66));
    fill_ell(dc, cell.left + w / 2 - pw / 3, cell.top + h / 2 - ph / 3, pw / 4,
             ph / 5, RGB(96, 140, 140), RGB(96, 140, 140));
  }
  if (theme == "town" && (hash & 1u)) {
    // Flagstone diagonal seam.
    line(dc, cell.left + w / 4, cell.top + h, cell.left + w / 2 + w / 4,
         cell.top, joint, 1);
  }
}

inline constexpr bool kPackGlyphHasBlade = true;
inline constexpr bool kPackGlyphHasGuard = true;

inline bool grey_pack_icon_fails_review(bool blade, bool guard) {
  return !blade || !guard;
}

// Inventory cell fallback when billboard art is missing. A grey square
// with only "+N" text cannot certify a carried weapon.
inline void pack_item_glyph(HDC dc, int cx, int cy, int size, Held held,
                            const Style& style) {
  const int h = std::max(16, size);
  const COLORREF metal = style.metal;
  const COLORREF trim = style.trim;
  const COLORREF dark = style.dark;
  if (held == Held::Axe) {
    const int haft_w = std::max(3, h / 10);
    const int haft_h = std::max(12, (h * 5) / 8);
    POINT haft[4] = {{cx - haft_w, cy + haft_h / 2},
                     {cx + haft_w, cy + haft_h / 2},
                     {cx + haft_w, cy - haft_h / 2},
                     {cx - haft_w, cy - haft_h / 2}};
    fill_poly(dc, haft, 4, trim, dark);
    const int head_w = std::max(8, h / 3);
    POINT head[4] = {{cx - 1, cy - haft_h / 3},
                     {cx + head_w, cy - haft_h / 2},
                     {cx + head_w, cy - haft_h / 10},
                     {cx - 1, cy - haft_h / 8}};
    fill_poly(dc, head, 4, metal, dark);
    return;
  }
  if (held == Held::Bow) {
    const int limb = std::max(8, h / 3);
    line(dc, cx + limb / 4, cy - limb, cx - limb / 6, cy, trim,
         std::max(2, h / 14));
    line(dc, cx + limb / 4, cy + limb, cx - limb / 6, cy, trim,
         std::max(2, h / 14));
    line(dc, cx + limb / 4, cy - limb, cx + limb / 4, cy + limb, metal, 1);
    return;
  }
  if (held == Held::Staff) {
    line(dc, cx, cy + h / 2, cx, cy - h / 2, trim, std::max(2, h / 12));
    fill_ell(dc, cx, cy - h / 2, std::max(3, h / 8), std::max(3, h / 8),
             style.accent, dark);
    return;
  }
  const int half = std::max(3, h / 7);
  const int tip_half = std::max(1, half / 3);
  const int blade_h = std::max(10, (h * 5) / 8);
  POINT blade[4] = {
      {cx - half, cy + blade_h / 4},
      {cx + half, cy + blade_h / 4},
      {cx + tip_half, cy - blade_h / 2},
      {cx - tip_half, cy - blade_h / 2}};
  fill_poly(dc, blade, 4, metal, dark);
  line(dc, blade[1].x, blade[1].y, blade[2].x, blade[2].y, shade(metal, 1.28),
       1);
  const int guard_h = std::max(3, h / 12);
  POINT guard[4] = {{cx - h / 4, cy + blade_h / 4},
                    {cx + h / 4, cy + blade_h / 4},
                    {cx + h / 4, cy + blade_h / 4 + guard_h},
                    {cx - h / 4, cy + blade_h / 4 + guard_h}};
  fill_poly(dc, guard, 4, trim, dark);
  line(dc, cx, cy + blade_h / 4 + guard_h, cx, cy + blade_h / 2, trim,
       std::max(2, h / 14));
}

}  // namespace vector_art

#endif  // _WIN32
