#pragma once

#ifdef _WIN32

#include "raster_art.hpp"

// Attach separate equipment to a measured hand in the actual rendered pose.
// Coordinates use source-canvas edges: (50.5, 66.5) is the center of pixel
// (50, 66). Actor and weapon share one native-pixel scale. Authored melee angles
// rotate the weapon about its grip, using raster_art's existing bounded cache.
// Actor pixels are never warped. Call after drawing the opaque actor.
namespace raster_equipment {

struct PixelPoint {
  double x = 0.0;
  double y = 0.0;
};

struct PoseMetadata {
  const char* name;
  raster_art::Dimensions canvas;
  PixelPoint hand;
  RECT fingers;  // Source-pixel rectangle; right and bottom are exclusive.
  bool faces_left;
  bool behind_actor;
  bool transitional;
  int melee_clockwise_degrees = 0;
  bool melee_behind_actor = false;
};

struct WeaponMetadata {
  const char* name;
  raster_art::Dimensions canvas;
  PixelPoint grip;
  bool flip_when_facing_right;
  bool authored_melee = false;
};

namespace detail {

// Anatomical left hand throughout: screen-right in front views and screen-left
// in rear views. These SE anchors were measured on the accepted 80x96 idle and
// four common-canvas walk frames, then inspected with all five weapons at 1x/3x.
inline constexpr PoseMetadata kPoses[] = {
#include "lineage_equipment.inc"
    {"hero_se", {80, 96}, {50.5, 66.5}, {49, 65, 52, 69}, false, false, false},
    {"hero_walk0_se", {80, 96}, {50.5, 66.5}, {49, 65, 53, 69}, false, false, false},
    {"hero_walk1_se", {80, 96}, {50.5, 64.5}, {49, 63, 52, 67}, false, false, false},
    {"hero_walk2_se", {80, 96}, {50.5, 65.5}, {49, 64, 52, 68}, false, false, false},
    {"hero_walk3_se", {80, 96}, {49.5, 62.5}, {48, 61, 51, 64}, false, false, false},
    {"hero_sw", {80, 96}, {52.5, 64.5}, {51, 63, 54, 67}, true, true, false},
    {"hero_ne", {80, 96}, {29.5, 62.5}, {28, 61, 31, 65}, false, true, false},
    {"hero_nw", {80, 96}, {29.5, 67.5}, {28, 65, 31, 69}, true, false, false},
    // SW/NW eight-frame walks: measured after Pixel Respecter reconstruction.
    // The SW left palm is the screen-right far hand; its carried weapon passes
    // behind opaque torso pixels. NW's screen-left palm is the near hand, so
    // only the measured fingers replay over the held weapon. Preserve each
    // frame's source position rather than borrowing the differently posed idle.
    {"hero_walk0_sw", {80, 96}, {45.5, 66.5}, {43, 65, 47, 69}, true, true, false},
    {"hero_walk1_sw", {80, 96}, {46.5, 66.5}, {44, 64, 49, 69}, true, true, false},
    {"hero_walk2_sw", {80, 96}, {47.5, 65.5}, {45, 63, 50, 68}, true, true, false},
    {"hero_walk3_sw", {80, 96}, {51.5, 65.5}, {49, 63, 54, 67}, true, true, false},
    {"hero_walk4_sw", {80, 96}, {47.5, 66.5}, {45, 64, 50, 68}, true, true, false},
    {"hero_walk5_sw", {80, 96}, {48.5, 66.5}, {46, 64, 50, 68}, true, true, false},
    {"hero_walk6_sw", {80, 96}, {44.5, 66.5}, {42, 64, 47, 69}, true, true, false},
    {"hero_walk7_sw", {80, 96}, {43.5, 65.5}, {41, 63, 46, 68}, true, true, false},
    // NW registration correction preserves every texel: first sheet row +1y,
    // second row +5y. These hand/replay coordinates follow the same pixels.
    {"hero_walk0_nw", {80, 96}, {26.5, 61.5}, {25, 60, 28, 63}, true, false, false},
    {"hero_walk1_nw", {80, 96}, {25.5, 61.5}, {23, 59, 26, 63}, true, false, false},
    {"hero_walk2_nw", {80, 96}, {24.5, 63.5}, {23, 62, 27, 65}, true, false, false},
    {"hero_walk3_nw", {80, 96}, {23.5, 61.5}, {22, 60, 25, 63}, true, false, false},
    {"hero_walk4_nw", {80, 96}, {25.5, 63.5}, {24, 62, 27, 65}, true, false, false},
    {"hero_walk5_nw", {80, 96}, {28.5, 64.5}, {27, 62, 29, 66}, true, false, false},
    {"hero_walk6_nw", {80, 96}, {25.5, 63.5}, {24, 62, 27, 65}, true, false, false},
    {"hero_walk7_nw", {80, 96}, {25.5, 63.5}, {25, 62, 27, 65}, true, false, false},
    // NE uses the screen-left far hand. Frames 6/7 expose only its narrow
    // skin edge below the bracer: anchor to that observed edge and let the
    // opaque body hide the rest of the grip, without inventing a visible fist.
    {"hero_walk0_ne", {80, 96}, {33.5, 60.5}, {32, 59, 34, 62}, false, true, false},
    {"hero_walk1_ne", {80, 96}, {31.5, 61.5}, {31, 59, 34, 63}, false, true, false},
    {"hero_walk2_ne", {80, 96}, {30.5, 61.5}, {29, 58, 33, 63}, false, true, false},
    {"hero_walk3_ne", {80, 96}, {31.5, 60.5}, {30, 59, 34, 63}, false, true, false},
    {"hero_walk4_ne", {80, 96}, {31.5, 61.5}, {31, 60, 33, 63}, false, true, false},
    {"hero_walk5_ne", {80, 96}, {30.5, 61.5}, {30, 59, 34, 63}, false, true, false},
    {"hero_walk6_ne", {80, 96}, {33.5, 60.5}, {32, 59, 34, 62}, false, true, false},
    {"hero_walk7_ne", {80, 96}, {34.5, 60.5}, {33, 59, 35, 62}, false, true, false},
    // Legacy attack art is transitional: these are measured anatomical-left
    // fists, but its body scale/style and striking arm differ across directions.
    // Attachment continuity does not make these weapon-specific attack poses.
    {"hero_attack_se", {80, 96}, {66.5, 70.5}, {65, 68, 69, 72}, false, false, true},
    {"hero_attack_sw", {80, 96}, {58.5, 61.5}, {57, 60, 60, 64}, true, true, true},
    {"hero_attack_ne", {80, 96}, {20.5, 70.5}, {19, 69, 22, 72}, false, true, true},
    {"hero_attack_nw", {80, 96}, {20.5, 51.5}, {19, 50, 23, 54}, true, false, true},
    // V3 SE strike body frames, physical-left hand. Palm centers are unchanged
    // after remeasurement; replay bounds include the new thumb/knuckle edges,
    // especially recovery5's upper row at y49. Angles describe an
    // authored blade/haft arc, not the forearm direction: gather, backswing,
    // approach, rightward contact, follow-through, raised recovery. The first
    // two melee silhouettes pass behind the head/body; the extension is near.
    // Bow and staff retain carry orientation; these are not bespoke attacks.
    {"hero_strike0_se", {80, 96}, {47.5, 52.5}, {45, 50, 50, 54}, false, false, false, -35, true},
    {"hero_strike1_se", {80, 96}, {49.5, 56.5}, {47, 55, 52, 59}, false, false, false, -65, true},
    {"hero_strike2_se", {80, 96}, {59.5, 49.5}, {57, 47, 62, 52}, false, false, false, 35},
    {"hero_strike3_se", {80, 96}, {68.5, 49.5}, {66, 47, 71, 52}, false, false, false, 90},
    {"hero_strike4_se", {80, 96}, {54.5, 54.5}, {52, 52, 57, 58}, false, false, false, 130},
    {"hero_strike5_se", {80, 96}, {45.5, 51.5}, {43, 49, 47, 54}, false, false, false, 15},
    // NW physical-left fist: gather, backswing across the far side, approach,
    // upper-left contact, follow-through, recovery. The frame1 blade passes
    // behind the actor; extension and recovery retain the near-hand fingers.
    {"hero_strike0_nw", {80, 96}, {26.5, 51.5}, {25, 50, 28, 53}, true, false, false, -5},
    {"hero_strike1_nw", {80, 96}, {22.5, 46.5}, {20, 45, 24, 49}, true, false, false, 45, true},
    {"hero_strike2_nw", {80, 96}, {15.5, 43.5}, {13, 42, 17, 46}, true, false, false, -25},
    {"hero_strike3_nw", {80, 96}, {13.5, 39.5}, {12, 37, 16, 42}, true, false, false, -55},
    {"hero_strike4_nw", {80, 96}, {25.5, 49.5}, {23, 48, 27, 52}, true, false, false, -95},
    {"hero_strike5_nw", {80, 96}, {26.5, 50.5}, {25, 49, 28, 54}, true, false, false, -15},
    // SW's bare left fist starts on the far side, crosses the body during
    // the downswing, then returns. The contact blade points lower-left; the
    // three crossing/extended poses retain only fingers over the weapon.
    {"hero_strike0_sw", {80, 96}, {57.5, 61.5}, {55, 60, 59, 64}, true, true, false},
    {"hero_strike1_sw", {80, 96}, {64.5, 48.5}, {62, 47, 67, 51}, true, true, false, 55},
    {"hero_strike2_sw", {80, 96}, {29.5, 71.5}, {27, 69, 33, 73}, true, false, false, -45},
    {"hero_strike3_sw", {80, 96}, {21.5, 71.5}, {19, 69, 25, 74}, true, false, false, -110},
    {"hero_strike4_sw", {80, 96}, {35.5, 62.5}, {33, 60, 39, 65}, true, false, false, -140},
    {"hero_strike5_sw", {80, 96}, {53.5, 60.5}, {51, 59, 56, 63}, true, true, false, -15},
    // NE uses the physical-left far hand: ready, windup, overhead commit,
    // upper-right contact, downward follow-through, low recovery. Measured
    // after original-palette normalization; geometry and pivots are fixed.
    // Body replay preserves far-side occlusion. Staff/bow keep carry angles.
    {"hero_strike0_ne", {80, 96}, {29.5, 62.5}, {28, 61, 31, 65}, false, true, false, 0},
    {"hero_strike1_ne", {80, 96}, {29.5, 41.5}, {28, 40, 31, 44}, false, true, false, -15},
    {"hero_strike2_ne", {80, 96}, {30.5, 21.5}, {29, 19, 33, 24}, false, true, false, -45},
    {"hero_strike3_ne", {80, 96}, {46.5, 22.5}, {44, 21, 48, 25}, false, true, false, 65},
    {"hero_strike4_ne", {80, 96}, {42.5, 22.5}, {41, 21, 44, 25}, false, true, false, 100},
    {"hero_strike5_ne", {80, 96}, {25.5, 64.5}, {24, 62, 28, 66}, false, true, false, -15},
};

// Axe and bow artwork faces left; mirror the image AND its source grip for a
// right-facing actor. The staff has no distinct wrapped grip: its measured
// mid-shaft contact leaves its bottom near the idle actor's ground line.
inline constexpr WeaponMetadata kWeapons[] = {
    {"weapon_axe", {32, 64}, {22.5, 57.5}, true, true},
    {"weapon_sword", {32, 64}, {15.5, 55.5}, false, true},
    {"weapon_staff", {32, 64}, {15.5, 35.5}, false},
    {"weapon_bow", {32, 64}, {11.5, 38.5}, true},
    {"weapon_club", {32, 64}, {15.5, 54.5}, false, true},
};

inline int scaled_width(int height, const raster_art::Dimensions& size) {
  return std::max(1, static_cast<int>(std::lround(
                         static_cast<double>(height) * size.width / size.height)));
}

}  // namespace detail

inline const PoseMetadata* pose_metadata(const char* resolved_actor_name) {
  if (!resolved_actor_name) return nullptr;
  for (const auto& pose : detail::kPoses)
    if (std::string_view(resolved_actor_name) == pose.name) return &pose;
  return nullptr;
}

inline const WeaponMetadata* weapon_metadata(const char* weapon_name) {
  if (!weapon_name) return nullptr;
  for (const auto& weapon : detail::kWeapons)
    if (std::string_view(weapon_name) == weapon.name) return &weapon;
  return nullptr;
}

struct Placement {
  const PoseMetadata* pose = nullptr;
  const WeaponMetadata* weapon = nullptr;
  RECT actor_bounds{};
  RECT weapon_bounds{};
  RECT occlusion_bounds{};
  PixelPoint hand_screen;
  int actor_center_x = 0;
  int actor_feet_y = 0;
  int actor_height = 0;
  int weapon_center_x = 0;
  int weapon_feet_y = 0;
  int weapon_height = 0;
  raster_art::SpriteTransform weapon_transform;
  int clockwise_degrees = 0;
  bool behind_actor = false;
  bool flip = false;
  [[nodiscard]] bool valid() const { return pose && weapon && weapon_height > 0; }
};

// resolved_actor_name must be the sprite actually drawn, including an idle
// fallback when a requested directional animation frame is absent. Unknown or
// resized source assets have no guessed attachment and return an invalid plan.
inline Placement compute(const char* resolved_actor_name, const char* weapon_name,
                         int center_x, int feet_y, int actor_height) {
  Placement result;
  const auto* pose = pose_metadata(resolved_actor_name);
  const auto* weapon = weapon_metadata(weapon_name);
  if (!pose || !weapon || actor_height <= 0 ||
      actor_height > raster_art::detail::kMaxDimension)
    return result;
  const auto actor_size = raster_art::dimensions(resolved_actor_name);
  const auto weapon_size = raster_art::dimensions(weapon_name);
  if (actor_size.width != pose->canvas.width ||
      actor_size.height != pose->canvas.height ||
      weapon_size.width != weapon->canvas.width ||
      weapon_size.height != weapon->canvas.height)
    return result;

  const int actor_width = detail::scaled_width(actor_height, actor_size);
  const double scale = static_cast<double>(actor_height) / actor_size.height;
  const int weapon_height = std::max(
      1, static_cast<int>(std::lround(weapon_size.height * scale)));
  const long long left = static_cast<long long>(center_x) - actor_width / 2;
  const long long top = static_cast<long long>(feet_y) - actor_height;
  if (left < std::numeric_limits<int>::min() || top < std::numeric_limits<int>::min() ||
      left + actor_width > std::numeric_limits<int>::max())
    return result;
  const int actor_left = static_cast<int>(left);
  const int actor_top = static_cast<int>(top);
  const PixelPoint hand{
      actor_left + pose->hand.x * actor_width / actor_size.width,
      actor_top + pose->hand.y * actor_height / actor_size.height};
  const bool flip = weapon->flip_when_facing_right != pose->faces_left;
  const int angle = weapon->authored_melee ? pose->melee_clockwise_degrees : 0;
  const auto transform = raster_art::sprite_transform(
      weapon_name, weapon_height, weapon->grip.x, weapon->grip.y, angle, flip);
  RECT weapon_bounds{};
  if (!raster_art::anchored_bounds(transform, hand.x, hand.y, weapon_bounds))
    return result;

  result.pose = pose;
  result.weapon = weapon;
  result.actor_bounds = {actor_left, actor_top, actor_left + actor_width, feet_y};
  result.weapon_bounds = weapon_bounds;
  result.hand_screen = hand;
  result.actor_center_x = center_x;
  result.actor_feet_y = feet_y;
  result.actor_height = actor_height;
  result.weapon_center_x = weapon_bounds.left + transform.canvas.width / 2;
  result.weapon_feet_y = weapon_bounds.bottom;
  result.weapon_height = weapon_height;
  result.flip = flip;
  result.weapon_transform = transform;
  result.clockwise_degrees = angle;
  result.behind_actor = pose->behind_actor || (angle != 0 && pose->melee_behind_actor);

  if (result.behind_actor) {
    // Replaying the existing actor through this rectangle restores its body
    // occlusion over a far-hand weapon. Transparent actor pixels retain weapon.
    IntersectRect(&result.occlusion_bounds, &result.actor_bounds,
                  &result.weapon_bounds);
  } else {
    // Match raster_art's nearest-neighbor / PixelOffsetModeHalf sampling: a
    // destination pixel belongs to this source rectangle when its center lies
    // inside the scaled source edges. This also preserves edge fingers at
    // fractional camera zoom without restoring an adjacent source pixel.
    result.occlusion_bounds = {
        actor_left + static_cast<int>(std::ceil(
                         pose->fingers.left * actor_width / double(actor_size.width) - 0.5)),
        actor_top + static_cast<int>(std::ceil(
                        pose->fingers.top * actor_height / double(actor_size.height) - 0.5)),
        actor_left + static_cast<int>(std::ceil(
                         pose->fingers.right * actor_width / double(actor_size.width) - 0.5)),
        actor_top + static_cast<int>(std::ceil(
                        pose->fingers.bottom * actor_height / double(actor_size.height) - 0.5))};
  }
  return result;
}

// Draw the equipment after its actor, then restore only the required original
// actor pixels over the grip (or body overlap for a far-hand weapon). This uses
// the same cached sprite and only a saved clip region; it creates no bitmaps.
inline bool draw_front(HDC dc, const Placement& placement) {
  if (!dc || !placement.valid()) return false;
  if (!raster_art::draw_sprite_at_anchor(dc, placement.weapon->name,
          placement.hand_screen.x, placement.hand_screen.y, placement.weapon_height,
          placement.weapon->grip.x, placement.weapon->grip.y,
          placement.clockwise_degrees, placement.flip))
    return false;
  if (IsRectEmpty(&placement.occlusion_bounds)) return true;
  const int saved = SaveDC(dc);
  if (saved == 0) return false;
  const RECT& clip = placement.occlusion_bounds;
  const int region = IntersectClipRect(dc, clip.left, clip.top, clip.right, clip.bottom);
  bool drawn = region != ERROR;
  if (drawn && region != NULLREGION)
    drawn = raster_art::draw_sprite(dc, placement.pose->name,
                                    placement.actor_center_x, placement.actor_feet_y,
                                    placement.actor_height);
  RestoreDC(dc, saved);
  return drawn;
}

inline bool draw_front(HDC dc, const char* resolved_actor_name,
                       const char* weapon_name, int center_x, int feet_y,
                       int actor_height) {
  return draw_front(dc, compute(resolved_actor_name, weapon_name,
                               center_x, feet_y, actor_height));
}

}  // namespace raster_equipment

#endif  // _WIN32
