#pragma once

#ifdef _WIN32

#include "raster_art.hpp"

// Attach separate equipment to a measured hand in the actual rendered pose.
// Coordinates use source-canvas edges: (50.5, 66.5) is the center of pixel
// (50, 66). Actor and weapon share one native-pixel scale. No source is cropped,
// warped, or rotated, and this helper adds no image storage to raster_art's
// bounded cache. Call after drawing the opaque actor with draw_sprite.
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
};

struct WeaponMetadata {
  const char* name;
  raster_art::Dimensions canvas;
  PixelPoint grip;
  bool flip_when_facing_right;
};

namespace detail {

// Anatomical left hand throughout: screen-right in front views and screen-left
// in rear views. These SE anchors were measured on the accepted 80x96 idle and
// four common-canvas walk frames, then inspected with all five weapons at 1x/3x.
inline constexpr PoseMetadata kPoses[] = {
    {"hero_se", {80, 96}, {50.5, 66.5}, {49, 65, 52, 69}, false, false, false},
    {"hero_walk0_se", {80, 96}, {50.5, 66.5}, {49, 65, 53, 69}, false, false, false},
    {"hero_walk1_se", {80, 96}, {50.5, 64.5}, {49, 63, 52, 67}, false, false, false},
    {"hero_walk2_se", {80, 96}, {50.5, 65.5}, {49, 64, 52, 68}, false, false, false},
    {"hero_walk3_se", {80, 96}, {49.5, 62.5}, {48, 61, 51, 64}, false, false, false},
    {"hero_sw", {80, 96}, {52.5, 64.5}, {51, 63, 54, 67}, true, true, false},
    {"hero_ne", {80, 96}, {29.5, 62.5}, {28, 61, 31, 65}, false, true, false},
    {"hero_nw", {80, 96}, {29.5, 67.5}, {28, 65, 31, 69}, true, false, false},
    // Legacy attack art is transitional: these are measured anatomical-left
    // fists, but its body scale/style and striking arm differ across directions.
    // Attachment continuity does not make these weapon-specific attack poses.
    {"hero_attack_se", {80, 96}, {66.5, 70.5}, {65, 68, 69, 72}, false, false, true},
    {"hero_attack_sw", {80, 96}, {58.5, 61.5}, {57, 60, 60, 64}, true, true, true},
    {"hero_attack_ne", {80, 96}, {20.5, 70.5}, {19, 69, 22, 72}, false, true, true},
    {"hero_attack_nw", {80, 96}, {20.5, 51.5}, {19, 50, 23, 54}, true, false, true},
};

// Axe and bow artwork faces left; mirror the image AND its source grip for a
// right-facing actor. The staff has no distinct wrapped grip: its measured
// mid-shaft contact leaves its bottom near the idle actor's ground line.
inline constexpr WeaponMetadata kWeapons[] = {
    {"weapon_axe", {32, 64}, {22.5, 57.5}, true},
    {"weapon_sword", {32, 64}, {15.5, 55.5}, false},
    {"weapon_staff", {32, 64}, {15.5, 35.5}, false},
    {"weapon_bow", {32, 64}, {11.5, 38.5}, true},
    {"weapon_club", {32, 64}, {15.5, 54.5}, false},
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
  const int weapon_width = detail::scaled_width(weapon_height, weapon_size);
  const int actor_left = center_x - actor_width / 2;
  const int actor_top = feet_y - actor_height;
  const PixelPoint hand{
      actor_left + pose->hand.x * actor_width / actor_size.width,
      actor_top + pose->hand.y * actor_height / actor_size.height};
  const bool flip = weapon->flip_when_facing_right != pose->faces_left;
  const double grip_x = flip ? weapon_size.width - weapon->grip.x : weapon->grip.x;
  const int weapon_left = static_cast<int>(std::lround(
      hand.x - grip_x * weapon_width / weapon_size.width));
  const int weapon_top = static_cast<int>(std::lround(
      hand.y - weapon->grip.y * weapon_height / weapon_size.height));

  result.pose = pose;
  result.weapon = weapon;
  result.actor_bounds = {actor_left, actor_top, actor_left + actor_width, feet_y};
  result.weapon_bounds = {weapon_left, weapon_top, weapon_left + weapon_width,
                          weapon_top + weapon_height};
  result.hand_screen = hand;
  result.actor_center_x = center_x;
  result.actor_feet_y = feet_y;
  result.actor_height = actor_height;
  result.weapon_center_x = weapon_left + weapon_width / 2;
  result.weapon_feet_y = weapon_top + weapon_height;
  result.weapon_height = weapon_height;
  result.flip = flip;

  if (pose->behind_actor) {
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
  if (!raster_art::draw_sprite(dc, placement.weapon->name,
                               placement.weapon_center_x, placement.weapon_feet_y,
                               placement.weapon_height, placement.flip))
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
