// framekit_renderer.hpp — TASK-0180 Framekit nine-slice render adapter.
//
// Deterministic layout planner for WIZARD Framekit raster slices (panel, slot,
// circular orb sprites). Computes source/destination blit regions without
// primitive placeholder chrome. No main.cpp integration in this packet.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace framekit_renderer {

inline constexpr std::size_t kNineSlicePieces = 9;

enum class TextureId : std::uint8_t {
  Panel = 0,
  Slot,
  OrbVitality,
  OrbMana,
  OrbEssence,
};

enum class Piece : std::uint8_t {
  TopLeft = 0,
  Top,
  TopRight,
  Left,
  Center,
  Right,
  BottomLeft,
  Bottom,
  BottomRight,
};

struct SliceInsets {
  std::uint16_t top = 0;
  std::uint16_t right = 0;
  std::uint16_t bottom = 0;
  std::uint16_t left = 0;

  [[nodiscard]] constexpr bool valid() const {
    return top > 0 || right > 0 || bottom > 0 || left > 0;
  }
};

struct TextureFootprint {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const {
    return width > 0 && height > 0;
  }
};

struct NineSliceAsset {
  TextureId id = TextureId::Panel;
  SliceInsets insets{};
  TextureFootprint source{};

  [[nodiscard]] constexpr bool valid() const {
    if (!insets.valid() || !source.valid()) return false;
    if (insets.left + insets.right >= source.width ||
        insets.top + insets.bottom >= source.height) {
      return false;
    }
    return true;
  }
};

struct Rect {
  std::int16_t x = 0;
  std::int16_t y = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct BlitRegion {
  std::int16_t dst_x = 0;
  std::int16_t dst_y = 0;
  std::uint16_t dst_w = 0;
  std::uint16_t dst_h = 0;
  std::uint16_t src_x = 0;
  std::uint16_t src_y = 0;
  std::uint16_t src_w = 0;
  std::uint16_t src_h = 0;
  TextureId texture = TextureId::Panel;
  Piece piece = Piece::TopLeft;

  [[nodiscard]] constexpr bool operator==(const BlitRegion&) const = default;
};

struct NineSlicePlan {
  std::array<BlitRegion, kNineSlicePieces> regions{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const NineSlicePlan&) const = default;
};

[[nodiscard]] constexpr NineSliceAsset default_panel_asset() {
  NineSliceAsset asset;
  asset.id = TextureId::Panel;
  asset.insets = {12, 12, 12, 12};
  asset.source = {48, 48};
  return asset;
}

[[nodiscard]] constexpr NineSliceAsset default_slot_asset() {
  NineSliceAsset asset;
  asset.id = TextureId::Slot;
  asset.insets = {12, 12, 12, 12};
  asset.source = {32, 32};
  return asset;
}

[[nodiscard]] constexpr NineSliceAsset default_orb_asset(TextureId id) {
  NineSliceAsset asset;
  asset.id = id;
  asset.insets = {2, 2, 2, 2};
  asset.source = {16, 16};
  return asset;
}

[[nodiscard]] constexpr BlitRegion make_region(TextureId texture, Piece piece,
                                               std::int16_t dx, std::int16_t dy,
                                               std::uint16_t dw, std::uint16_t dh,
                                               std::uint16_t sx, std::uint16_t sy,
                                               std::uint16_t sw, std::uint16_t sh) {
  BlitRegion region;
  region.texture = texture;
  region.piece = piece;
  region.dst_x = dx;
  region.dst_y = dy;
  region.dst_w = dw;
  region.dst_h = dh;
  region.src_x = sx;
  region.src_y = sy;
  region.src_w = sw;
  region.src_h = sh;
  return region;
}

[[nodiscard]] constexpr NineSlicePlan plan_nine_slice(const Rect& dest,
                                                    const NineSliceAsset& asset) {
  NineSlicePlan plan;
  if (!dest.valid() || !asset.valid()) return plan;

  const std::uint16_t min_w =
      static_cast<std::uint16_t>(asset.insets.left + asset.insets.right);
  const std::uint16_t min_h =
      static_cast<std::uint16_t>(asset.insets.top + asset.insets.bottom);
  if (dest.width < min_w || dest.height < min_h) return plan;

  const TextureId tex = asset.id;
  const std::uint16_t l = asset.insets.left;
  const std::uint16_t r = asset.insets.right;
  const std::uint16_t t = asset.insets.top;
  const std::uint16_t b = asset.insets.bottom;
  const std::uint16_t sw = asset.source.width;
  const std::uint16_t sh = asset.source.height;

  const std::uint16_t center_w = dest.width - l - r;
  const std::uint16_t center_h = dest.height - t - b;
  const std::uint16_t src_center_w = sw - l - r;
  const std::uint16_t src_center_h = sh - t - b;

  const std::int16_t x = dest.x;
  const std::int16_t y = dest.y;

  plan.regions[static_cast<std::size_t>(Piece::TopLeft)] =
      make_region(tex, Piece::TopLeft, x, y, l, t, 0, 0, l, t);
  plan.regions[static_cast<std::size_t>(Piece::Top)] =
      make_region(tex, Piece::Top, x + static_cast<std::int16_t>(l), y, center_w,
                  t, l, 0, src_center_w, t);
  plan.regions[static_cast<std::size_t>(Piece::TopRight)] =
      make_region(tex, Piece::TopRight, x + static_cast<std::int16_t>(l + center_w),
                  y, r, t, sw - r, 0, r, t);
  plan.regions[static_cast<std::size_t>(Piece::Left)] =
      make_region(tex, Piece::Left, x, y + static_cast<std::int16_t>(t), l, center_h,
                  0, t, l, src_center_h);
  plan.regions[static_cast<std::size_t>(Piece::Center)] =
      make_region(tex, Piece::Center, x + static_cast<std::int16_t>(l),
                  y + static_cast<std::int16_t>(t), center_w, center_h, l, t,
                  src_center_w, src_center_h);
  plan.regions[static_cast<std::size_t>(Piece::Right)] =
      make_region(tex, Piece::Right, x + static_cast<std::int16_t>(l + center_w),
                  y + static_cast<std::int16_t>(t), r, center_h, sw - r, t, r,
                  src_center_h);
  plan.regions[static_cast<std::size_t>(Piece::BottomLeft)] =
      make_region(tex, Piece::BottomLeft, x, y + static_cast<std::int16_t>(t + center_h),
                  l, b, 0, sh - b, l, b);
  plan.regions[static_cast<std::size_t>(Piece::Bottom)] =
      make_region(tex, Piece::Bottom, x + static_cast<std::int16_t>(l),
                  y + static_cast<std::int16_t>(t + center_h), center_w, b, l,
                  sh - b, src_center_w, b);
  plan.regions[static_cast<std::size_t>(Piece::BottomRight)] =
      make_region(tex, Piece::BottomRight, x + static_cast<std::int16_t>(l + center_w),
                  y + static_cast<std::int16_t>(t + center_h), r, b, sw - r, sh - b,
                  r, b);

  plan.valid = true;
  return plan;
}

[[nodiscard]] constexpr BlitRegion plan_sprite(const Rect& dest, TextureId texture,
                                               const TextureFootprint& source) {
  BlitRegion region;
  if (!dest.valid() || !source.valid()) return region;
  region.texture = texture;
  region.piece = Piece::Center;
  region.dst_x = dest.x;
  region.dst_y = dest.y;
  region.dst_w = dest.width;
  region.dst_h = dest.height;
  region.src_x = 0;
  region.src_y = 0;
  region.src_w = source.width;
  region.src_h = source.height;
  return region;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const NineSlicePlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  for (const BlitRegion& region : plan.regions) {
    hash ^= static_cast<std::uint32_t>(region.dst_x) * 131u;
    hash ^= static_cast<std::uint32_t>(region.dst_y) * 257u;
    hash ^= region.dst_w * 389u;
    hash ^= region.dst_h * 521u;
    hash ^= region.src_x * 613u;
    hash ^= region.src_y * 719u;
    hash ^= region.src_w * 823u;
    hash ^= region.src_h * 929u;
    hash ^= static_cast<std::uint32_t>(region.texture) * 1031u;
  }
  return hash;
}

}  // namespace framekit_renderer
