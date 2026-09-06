#pragma once

// VG-GPU-006: one moving light on the bronze/stone family.
// Intensity is capped so the material never washes to white and cannot
// conceal a damage-zone chroma.

#include "cook-shaders-and-resource-bindings.hpp"

namespace verdigris::gpu {

inline constexpr std::uint32_t kDamageZone = 0x00E04840u;
inline constexpr int kLitChannelCap = 220;

struct Light {
  int x = 2;
  int y = 2;
  int intensity = 180;
};

inline Light light_from_tick(int tick) {
  const int phase = ((tick % 40) + 40) % 40;
  Light light;
  light.x = 1 + (phase * 5) / 40;
  light.y = 2 + ((phase / 8) % 3);
  light.intensity = 160 + (phase % 5) * 4;
  return light;
}

inline std::uint32_t shade_texel_lit(const Bindings& bindings, int u, int v,
                                     Light light) {
  const std::uint32_t base = shade_texel(bindings, u, v);
  if (!bindings.albedo || bindings.map_size <= 0) return base;
  const int size = bindings.map_size;
  const int x = ((u % size) + size) % size;
  const int y = ((v % size) + size) % size;
  const int dx = x - light.x;
  const int dy = y - light.y;
  const int dist2 = dx * dx + dy * dy;
  const int boost = (light.intensity * 8) / (8 + dist2);
  int r = static_cast<int>((base >> 16) & 0xFF) + (boost * 48) / 256;
  int g = static_cast<int>((base >> 8) & 0xFF) + (boost * 36) / 256;
  int b = static_cast<int>(base & 0xFF) + (boost * 20) / 256;
  if (r > kLitChannelCap) r = kLitChannelCap;
  if (g > kLitChannelCap) g = kLitChannelCap;
  if (b > kLitChannelCap) b = kLitChannelCap;
  return (static_cast<std::uint32_t>(r) << 16) |
         (static_cast<std::uint32_t>(g) << 8) |
         static_cast<std::uint32_t>(b);
}

inline std::uint32_t composite_damage_zone(std::uint32_t lit, bool in_zone) {
  if (!in_zone) return lit;
  const int lr = static_cast<int>((lit >> 16) & 0xFF);
  const int lg = static_cast<int>((lit >> 8) & 0xFF);
  const int lb = static_cast<int>(lit & 0xFF);
  const int zr = static_cast<int>((kDamageZone >> 16) & 0xFF);
  const int zg = static_cast<int>((kDamageZone >> 8) & 0xFF);
  const int zb = static_cast<int>(kDamageZone & 0xFF);
  int r = (lr + zr * 2) / 3;
  int g = (lg + zg * 2) / 3;
  int b = (lb + zb * 2) / 3;
  if (r > 230 && g > 230 && b > 230) return kDamageZone;
  return (static_cast<std::uint32_t>(r) << 16) |
         (static_cast<std::uint32_t>(g) << 8) |
         static_cast<std::uint32_t>(b);
}

inline bool washed_light_fails_review(std::uint32_t pixel) {
  const int r = static_cast<int>((pixel >> 16) & 0xFF);
  const int g = static_cast<int>((pixel >> 8) & 0xFF);
  const int b = static_cast<int>(pixel & 0xFF);
  return r > kLitChannelCap || (r > 240 && g > 240 && b > 240);
}

inline bool hud_label_alone_fails_light_review(bool pool_painted) {
  return !pool_painted;
}

inline bool damage_zone_concealed(std::uint32_t pixel) {
  const int r = static_cast<int>((pixel >> 16) & 0xFF);
  const int g = static_cast<int>((pixel >> 8) & 0xFF);
  const int b = static_cast<int>(pixel & 0xFF);
  const bool white = r > 240 && g > 240 && b > 240;
  const bool background = r < 40 && g < 48 && b < 56;
  const int zr = static_cast<int>((kDamageZone >> 16) & 0xFF);
  const int dr = r - zr;
  const int dg = g - static_cast<int>((kDamageZone >> 8) & 0xFF);
  const int db = b - static_cast<int>(kDamageZone & 0xFF);
  const int dist = dr * dr + dg * dg + db * db;
  return white || background || dist > 18000;
}

inline bool draw_lit_quad_moving(Sample& sample, const Bindings& bindings,
                                 Light light, bool paint_damage_zone) {
  if (!draw_lit_quad(sample, bindings)) return false;
  const int x0 = 16;
  const int y0 = 16;
  const int x1 = 48;
  const int y1 = 48;
  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      const int u = (x - x0) * bindings.map_size / (x1 - x0);
      const int v = (y - y0) * bindings.map_size / (y1 - y0);
      std::uint32_t lit = shade_texel_lit(bindings, u, v, light);
      const bool zone = paint_damage_zone && x >= 24 && x < 40 && y >= 24 && y < 40;
      lit = composite_damage_zone(lit, zone);
      sample.pixels[static_cast<std::size_t>(y * sample.width + x)] = lit;
    }
  }
  return true;
}

}  // namespace verdigris::gpu
