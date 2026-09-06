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

#include <cmath>
#include <cstdint>
#include <cctype>
#include <string>

namespace vector_art {

inline constexpr double kPi = 3.14159265358979323846;

// ── small GDI helpers ───────────────────────────────────────────────────

inline void fill_poly(HDC dc, const POINT* points, int count, COLORREF fill,
                      COLORREF outline) {
  HBRUSH brush = CreateSolidBrush(fill);
  HPEN pen = CreatePen(PS_SOLID, 1, outline);
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
  HBRUSH brush = CreateSolidBrush(fill);
  HPEN pen = CreatePen(PS_SOLID, 1, outline);
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
  HPEN pen = CreatePen(PS_SOLID, width, color);
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
// Bronze-age figure, 3/4 side read. Legs swing with the walk cycle, torso
// bobs with breath/steps, the near arm swings the held tool on attack.

inline void humanoid(HDC dc, int cx, int base_y, int height_px,
                     const Style& style, const Pose& pose, Held held) {
  Frame f{cx, base_y, height_px / 100.0, pose.mirror};
  const double swing = std::sin(pose.walk * 2.0 * kPi) * pose.moving;
  const double bob = std::sin(pose.breathe * 2.0 * kPi) * 1.5 +
                     std::abs(swing) * 2.0;
  double attack_lean = 0.0;
  double strike = 0.0;
  switch (pose.attack_stage) {
    case Pose::AttackStage::Windup:
      attack_lean = -4.0;
      strike = -0.75 - pose.attack * 0.45;
      break;
    case Pose::AttackStage::Active:
      attack_lean = pose.attack * 6.0;
      strike = std::sin(std::min(1.0, pose.attack) * kPi) * 1.9;
      break;
    case Pose::AttackStage::Recovery:
      attack_lean = 2.0;
      strike = 0.45 * (1.0 - std::min(1.0, pose.attack));
      break;
    case Pose::AttackStage::Cancel:
      attack_lean = -1.5;
      strike = -0.2;
      break;
    case Pose::AttackStage::Idle:
    default:
      break;
  }
  const double lean = pose.moving * 3.0 + attack_lean;

  // Far leg, then near leg (draw order back-to-front).
  const double leg_spread = 8.0;
  const auto leg = [&](double side, double phase_swing, COLORREF tone) {
    const double knee_x = side * leg_spread + phase_swing * 10.0;
    const double foot_x = side * leg_spread + phase_swing * 16.0;
    POINT p[4] = {f.at(side * leg_spread - 4, 45), f.at(side * leg_spread + 4, 45),
                  f.at(foot_x + 4, 0), f.at(foot_x - 5, 0)};
    fill_poly(dc, p, 4, tone, style.dark);
    (void)knee_x;
  };
  leg(-1.0, -swing, shade(style.cloth, 0.72));
  leg(1.0, swing, style.cloth);

  // Tunic skirt.
  {
    POINT p[4] = {f.at(-13, 58 + bob * 0.4), f.at(13, 58 + bob * 0.4),
                  f.at(16, 38), f.at(-16, 38)};
    fill_poly(dc, p, 4, style.cloth, style.dark);
  }
  // Torso with breathing bob and forward lean.
  {
    POINT p[4] = {f.at(-11 + lean * 0.4, 82 + bob), f.at(12 + lean * 0.5, 82 + bob),
                  f.at(14, 55), f.at(-14, 55)};
    fill_poly(dc, p, 4, shade(style.cloth, 1.18), style.dark);
    // Chest wrap / belt trim.
    POINT belt[4] = {f.at(-13, 60), f.at(13, 60), f.at(14, 55), f.at(-14, 55)};
    fill_poly(dc, belt, 4, style.trim, style.dark);
  }
  // Far arm hangs (slight counter-swing while walking).
  {
    const double hand_x = -14 - swing * 6.0;
    POINT p[4] = {f.at(-9 + lean * 0.4, 78 + bob), f.at(-14 + lean * 0.4, 76 + bob),
                  f.at(hand_x, 52), f.at(hand_x + 5, 52)};
    fill_poly(dc, p, 4, shade(style.skin, 0.8), style.dark);
  }
  // Head + simple helm.
  {
    const POINT crown = f.at(lean * 0.6, 95 + bob);
    fill_ell(dc, crown.x, crown.y + static_cast<int>(6 * f.scale),
             static_cast<int>(8 * f.scale), static_cast<int>(9 * f.scale),
             style.skin, style.dark);
    fill_ell(dc, crown.x, crown.y + static_cast<int>(2 * f.scale),
             static_cast<int>(9 * f.scale), static_cast<int>(5 * f.scale),
             style.trim, style.dark);
  }
  // Near arm + held tool: shoulder-anchored swing driven by attack phase.
  {
    const double shoulder_x = 9 + lean * 0.5;
    const double shoulder_y = 79 + bob;
    // Rest angle points the tool down-forward; the attack sweeps it over
    // the shoulder and through: -140deg .. +30deg of unit-space rotation.
    const double angle = (-0.55 + strike) + swing * 0.25;
    const double hand_x = shoulder_x + std::cos(angle) * 24.0;
    const double hand_y = shoulder_y + std::sin(angle) * 24.0 - 24.0 + 4.0;
    POINT arm[4] = {f.at(shoulder_x - 3, shoulder_y), f.at(shoulder_x + 4, shoulder_y),
                    f.at(hand_x + 3, hand_y), f.at(hand_x - 3, hand_y)};
    fill_poly(dc, arm, 4, style.skin, style.dark);
    // Tool from the hand, continuing the arm angle.
    const double tip_x = hand_x + std::cos(angle) * 26.0;
    const double tip_y = hand_y + std::sin(angle) * 26.0;
    const POINT hand = f.at(hand_x, hand_y);
    const POINT tip = f.at(tip_x, tip_y);
    switch (held) {
      case Held::Axe: {
        line(dc, hand.x, hand.y, tip.x, tip.y, style.trim,
             std::max(2, static_cast<int>(3 * f.scale)));
        const POINT head = f.at(tip_x, tip_y);
        fill_ell(dc, head.x, head.y, static_cast<int>(7 * f.scale),
                 static_cast<int>(5 * f.scale), style.metal, style.dark);
        break;
      }
      case Held::Sword:
        line(dc, hand.x, hand.y, tip.x, tip.y, style.metal,
             std::max(2, static_cast<int>(3 * f.scale)));
        break;
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
        // Strung arc perpendicular-ish to the arm.
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
             f.at(hand_x, hand_y - 2).x, f.at(hand_x, hand_y - 2).y,
             style.trim, 1);
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
inline void lurker(HDC dc, int cx, int base_y, int height_px, const Style& style,
                   const Pose& pose) {
  Frame f{cx, base_y, height_px / 100.0, pose.mirror};
  const double swing = std::sin(pose.walk * 2.0 * kPi) * pose.moving;
  const double bob = std::sin(pose.breathe * 2.0 * kPi) * 2.0;
  const double reach = std::sin(std::min(1.0, pose.attack) * kPi);
  // Legs.
  const auto leg = [&](double side, double phase, COLORREF tone) {
    POINT p[4] = {f.at(side * 10 - 5, 40), f.at(side * 10 + 5, 40),
                  f.at(side * 10 + phase * 14 + 5, 0),
                  f.at(side * 10 + phase * 14 - 6, 0)};
    fill_poly(dc, p, 4, tone, style.dark);
  };
  leg(-1.0, -swing, shade(style.cloth, 0.7));
  leg(1.0, swing, style.cloth);
  // Hunched mass: big shoulder hump over a narrow waist.
  {
    POINT p[5] = {f.at(-20, 46), f.at(-8, 74 + bob), f.at(16, 66 + bob),
                  f.at(22, 44), f.at(0, 36)};
    fill_poly(dc, p, 5, style.cloth, style.dark);
    // Spine ridge.
    line(dc, f.at(-12, 72 + bob).x, f.at(-12, 72 + bob).y, f.at(12, 62 + bob).x,
         f.at(12, 62 + bob).y, style.trim, 2);
  }
  // Low-slung head with jaw.
  {
    const POINT skull = f.at(24 + reach * 8, 56 + bob);
    fill_ell(dc, skull.x, skull.y, static_cast<int>(9 * f.scale),
             static_cast<int>(7 * f.scale), shade(style.cloth, 1.15), style.dark);
    POINT jaw[3] = {f.at(28 + reach * 8, 52 + bob), f.at(36 + reach * 10, 48 + bob),
                    f.at(27 + reach * 8, 47 + bob)};
    fill_poly(dc, jaw, 3, style.trim, style.dark);
    // Eye ember.
    const POINT eye = f.at(26 + reach * 8, 58 + bob);
    fill_ell(dc, eye.x, eye.y, std::max(1, static_cast<int>(2 * f.scale)),
             std::max(1, static_cast<int>(2 * f.scale)), style.accent,
             style.accent);
  }
  // Claw arm rakes forward with the attack.
  {
    const double claw_x = 20 + reach * 20;
    const double claw_y = 30 + reach * 14;
    POINT arm[4] = {f.at(8, 60 + bob), f.at(16, 60 + bob), f.at(claw_x, claw_y),
                    f.at(claw_x - 7, claw_y - 3)};
    fill_poly(dc, arm, 4, shade(style.cloth, 0.9), style.dark);
    for (int talon = 0; talon < 3; ++talon) {
      const POINT root = f.at(claw_x - talon * 3, claw_y - talon * 1.5);
      const POINT tip = f.at(claw_x + 7 - talon * 3, claw_y - 6 - talon * 1.5);
      line(dc, root.x, root.y, tip.x, tip.y, style.metal, 2);
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
  s.skin = RGB(198, 160, 122);
  s.cloth = RGB(126, 104, 74);
  s.trim = RGB(84, 66, 46);
  s.metal = RGB(188, 196, 204);
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
  } else {  // dungeon/stone default
    s.cloth = RGB(128, 100, 88);
    s.trim = RGB(88, 66, 58);
    s.metal = RGB(198, 190, 186);
    s.accent = RGB(255, 120, 90);
  }
  if (elite) {
    s.cloth = shade(s.cloth, 1.2);
    s.trim = RGB(160, 128, 60);
    s.accent = RGB(255, 214, 110);
  }
  return s;
}

// ── scenery ─────────────────────────────────────────────────────────────

inline void tree(HDC dc, int cx, int base_y, int height_px, double sway_phase,
                 COLORREF leaf, COLORREF trunk) {
  const double s = height_px / 100.0;
  const int sway = static_cast<int>(std::sin(sway_phase) * 3.0 * s);
  const COLORREF dark = RGB(20, 16, 12);
  POINT bole[4] = {{cx - static_cast<int>(4 * s), base_y},
                   {cx + static_cast<int>(4 * s), base_y},
                   {cx + static_cast<int>(2 * s) + sway / 2,
                    base_y - static_cast<int>(45 * s)},
                   {cx - static_cast<int>(2 * s) + sway / 2,
                    base_y - static_cast<int>(45 * s)}};
  fill_poly(dc, bole, 4, trunk, dark);
  fill_ell(dc, cx + sway, base_y - static_cast<int>(62 * s),
           static_cast<int>(22 * s), static_cast<int>(18 * s), leaf, dark);
  fill_ell(dc, cx - static_cast<int>(12 * s) + sway,
           base_y - static_cast<int>(52 * s), static_cast<int>(14 * s),
           static_cast<int>(12 * s), shade(leaf, 0.85), dark);
  fill_ell(dc, cx + static_cast<int>(13 * s) + sway,
           base_y - static_cast<int>(50 * s), static_cast<int>(13 * s),
           static_cast<int>(11 * s), shade(leaf, 1.12), dark);
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

inline void fountain(HDC dc, int cx, int base_y, int height_px, double phase,
                     COLORREF stone, COLORREF water) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  // Basin.
  fill_ell(dc, cx, base_y - static_cast<int>(6 * s), static_cast<int>(30 * s),
           static_cast<int>(10 * s), stone, dark);
  fill_ell(dc, cx, base_y - static_cast<int>(8 * s), static_cast<int>(24 * s),
           static_cast<int>(7 * s), water, dark);
  // Column and bowl.
  POINT column[4] = {{cx - static_cast<int>(4 * s), base_y - static_cast<int>(8 * s)},
                     {cx + static_cast<int>(4 * s), base_y - static_cast<int>(8 * s)},
                     {cx + static_cast<int>(3 * s), base_y - static_cast<int>(34 * s)},
                     {cx - static_cast<int>(3 * s), base_y - static_cast<int>(34 * s)}};
  fill_poly(dc, column, 4, shade(stone, 1.1), dark);
  fill_ell(dc, cx, base_y - static_cast<int>(36 * s), static_cast<int>(12 * s),
           static_cast<int>(4 * s), stone, dark);
  // Animated spouts.
  for (int spout = -1; spout <= 1; spout += 2) {
    const double arc = std::sin(phase * 2.0 * kPi) * 2.0;
    line(dc, cx, base_y - static_cast<int>(38 * s),
         cx + static_cast<int>(spout * (12 + arc) * s),
         base_y - static_cast<int>(14 * s), water, 2);
  }
}

inline void road_gate(HDC dc, int cx, int base_y, int height_px, COLORREF stone,
                      COLORREF accent) {
  const double s = height_px / 100.0;
  const COLORREF dark = RGB(20, 16, 12);
  for (int side = -1; side <= 1; side += 2) {
    POINT pillar[4] = {{cx + static_cast<int>(side * 20 * s) - static_cast<int>(5 * s), base_y},
                       {cx + static_cast<int>(side * 20 * s) + static_cast<int>(5 * s), base_y},
                       {cx + static_cast<int>(side * 18 * s) + static_cast<int>(4 * s),
                        base_y - static_cast<int>(52 * s)},
                       {cx + static_cast<int>(side * 18 * s) - static_cast<int>(4 * s),
                        base_y - static_cast<int>(52 * s)}};
    fill_poly(dc, pillar, 4, stone, dark);
  }
  POINT lintel[4] = {{cx - static_cast<int>(26 * s), base_y - static_cast<int>(50 * s)},
                     {cx + static_cast<int>(26 * s), base_y - static_cast<int>(50 * s)},
                     {cx + static_cast<int>(24 * s), base_y - static_cast<int>(60 * s)},
                     {cx - static_cast<int>(24 * s), base_y - static_cast<int>(60 * s)}};
  fill_poly(dc, lintel, 4, shade(stone, 1.12), dark);
  // Waymark glyph.
  fill_ell(dc, cx, base_y - static_cast<int>(55 * s), static_cast<int>(4 * s),
           static_cast<int>(4 * s), accent, dark);
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
  HBRUSH brush = CreateSolidBrush(alt ? shade(base, 0.93) : base);
  FillRect(dc, &fill_rect, brush);
  DeleteObject(brush);
  // Slab joints.
  HPEN pen = CreatePen(PS_SOLID, 1, joint);
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

}  // namespace vector_art

#endif  // _WIN32
