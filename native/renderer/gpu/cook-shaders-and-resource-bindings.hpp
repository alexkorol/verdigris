#pragma once

// VG-GPU-003: versioned software program + resource bindings.
// Loaded from compiled tables (no runtime .hlsl/.metal path). Wrong backend
// or stale layout returns false — the sample must not draw a silent fallback.

#include <cstdint>
#include <cstring>

#include "build-an-isolated-cross-platform-gpu-sample.hpp"
#include "../../client/assets/production/bronze_stone.hpp"

namespace verdigris::gpu {

inline constexpr std::uint32_t kBindingLayoutVersion = 1;
inline constexpr const char* kSoftwareShaderId = "software-albedo-rim-v1";

struct Bindings {
  const char* backend = nullptr;
  std::uint32_t layout_version = 0;
  const char* shader_id = nullptr;
  const std::uint32_t* albedo = nullptr;
  const std::uint32_t* rim = nullptr;
  int map_size = 0;
};

inline bool load_bindings(Backend backend, std::uint32_t layout_version,
                          Bindings* out) {
  if (out == nullptr) return false;
  *out = Bindings{};
  if (backend != Backend::Software) return false;
  if (layout_version != kBindingLayoutVersion) return false;
  out->backend = Sample::kBackendName;
  out->layout_version = kBindingLayoutVersion;
  out->shader_id = kSoftwareShaderId;
  out->albedo = verdigris::art::bronze_stone::kAlbedo;
  out->rim = verdigris::art::bronze_stone::kRim;
  out->map_size = verdigris::art::bronze_stone::kMapSize;
  return out->albedo != nullptr && out->rim != nullptr &&
         !verdigris::art::bronze_stone::is_placeholder(out->albedo[0]);
}

inline std::uint32_t shade_texel(const Bindings& bindings, int u, int v) {
  if (!bindings.albedo || bindings.map_size <= 0)
    return verdigris::art::bronze_stone::kPlaceholder;
  const int size = bindings.map_size;
  const int x = ((u % size) + size) % size;
  const int y = ((v % size) + size) % size;
  const int i = y * size + x;
  const std::uint32_t albedo = bindings.albedo[i];
  const std::uint32_t rim =
      bindings.rim ? bindings.rim[i] : albedo;
  const int ar = static_cast<int>((albedo >> 16) & 0xFF);
  const int ag = static_cast<int>((albedo >> 8) & 0xFF);
  const int ab = static_cast<int>(albedo & 0xFF);
  const int rr = static_cast<int>((rim >> 16) & 0xFF);
  const int rg = static_cast<int>((rim >> 8) & 0xFF);
  const int rb = static_cast<int>(rim & 0xFF);
  const int r = (ar * 3 + rr) / 4;
  const int g = (ag * 3 + rg) / 4;
  const int b = (ab * 3 + rb) / 4;
  return (static_cast<std::uint32_t>(r) << 16) |
         (static_cast<std::uint32_t>(g) << 8) |
         static_cast<std::uint32_t>(b);
}

inline bool draw_lit_quad(Sample& sample, const Bindings& bindings) {
  if (!sample.alive || !bindings.albedo || bindings.map_size <= 0) return false;
  if (verdigris::art::bronze_stone::is_placeholder(bindings.albedo[0]))
    return false;
  const int x0 = 16;
  const int y0 = 16;
  const int x1 = 48;
  const int y1 = 48;
  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      const int u = (x - x0) * bindings.map_size / (x1 - x0);
      const int v = (y - y0) * bindings.map_size / (y1 - y0);
      sample.pixels[static_cast<std::size_t>(y * sample.width + x)] =
          shade_texel(bindings, u, v);
    }
  }
  return true;
}

}  // namespace verdigris::gpu
