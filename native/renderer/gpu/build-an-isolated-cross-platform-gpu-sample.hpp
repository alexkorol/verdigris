#pragma once

// VG-GPU-001: isolated GPU sample outside the simulation core.
// The portable proof is Backend::Software (CPU textured quad + shutdown).
// A Win32/D3D-only window cannot settle the cross-platform backend decision;
// macOS uses this same software path until a Metal presenter exists.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "packets.hpp"
#include "../../client/assets/production/bronze_stone.hpp"

namespace verdigris::gpu {

enum class Backend { Software };

struct Sample {
  static constexpr int kWidth = 64;
  static constexpr int kHeight = 64;
  static constexpr const char* kBackendName = "software";

  bool alive = false;
  int width = 0;
  int height = 0;
  std::vector<std::uint32_t> pixels;  // 0x00RRGGBB

  bool init(Backend backend, int w = kWidth, int h = kHeight) {
    shutdown();
    if (backend != Backend::Software) return false;
    if (w <= 0 || h <= 0) return false;
    width = w;
    height = h;
    pixels.assign(static_cast<std::size_t>(width * height), 0x00182028);
    alive = true;
    return true;
  }

  void shutdown() {
    pixels.clear();
    width = 0;
    height = 0;
    alive = false;
  }

  // 8x8 bronze/stone checker. Not a gameplay texture handle.
  static std::uint32_t texel(int u, int v) {
    return verdigris::art::bronze_stone::sample_albedo(u, v);
  }

  bool draw_textured_quad() {
    if (!alive || pixels.size() != static_cast<std::size_t>(width * height))
      return false;
    const int x0 = 16;
    const int y0 = 16;
    const int x1 = 48;
    const int y1 = 48;
    for (int y = y0; y < y1; ++y) {
      for (int x = x0; x < x1; ++x) {
        const int u = (x - x0) * 8 / (x1 - x0);
        const int v = (y - y0) * 8 / (y1 - y0);
        pixels[static_cast<std::size_t>(y * width + x)] = texel(u, v);
      }
    }
    return true;
  }

  std::uint32_t pixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return 0;
    return pixels[static_cast<std::size_t>(y * width + x)];
  }

  bool write_bmp(const std::string& path) const {
    if (!alive || path.empty()) return false;
    const int row_stride = ((width * 3 + 3) / 4) * 4;
    const std::uint32_t pixel_bytes =
        static_cast<std::uint32_t>(row_stride * height);
    FILE* file = nullptr;
#if defined(_MSC_VER)
    if (fopen_s(&file, path.c_str(), "wb") != 0 || !file) return false;
#else
    file = std::fopen(path.c_str(), "wb");
    if (!file) return false;
#endif
    const std::uint32_t off = 54;
    const std::uint32_t size = off + pixel_bytes;
    const std::uint8_t hdr[54] = {
        'B', 'M',
        static_cast<std::uint8_t>(size), static_cast<std::uint8_t>(size >> 8),
        static_cast<std::uint8_t>(size >> 16), static_cast<std::uint8_t>(size >> 24),
        0, 0, 0, 0,
        54, 0, 0, 0,
        40, 0, 0, 0,
        static_cast<std::uint8_t>(width), static_cast<std::uint8_t>(width >> 8),
        static_cast<std::uint8_t>(width >> 16), static_cast<std::uint8_t>(width >> 24),
        static_cast<std::uint8_t>(height), static_cast<std::uint8_t>(height >> 8),
        static_cast<std::uint8_t>(height >> 16), static_cast<std::uint8_t>(height >> 24),
        1, 0, 24, 0};
    if (std::fwrite(hdr, 1, 54, file) != 54) {
      std::fclose(file);
      return false;
    }
    std::vector<std::uint8_t> row(static_cast<std::size_t>(row_stride), 0);
    for (int y = height - 1; y >= 0; --y) {
      for (int x = 0; x < width; ++x) {
        const std::uint32_t p = pixel(x, y);
        row[static_cast<std::size_t>(x * 3 + 0)] =
            static_cast<std::uint8_t>(p & 0xFF);
        row[static_cast<std::size_t>(x * 3 + 1)] =
            static_cast<std::uint8_t>((p >> 8) & 0xFF);
        row[static_cast<std::size_t>(x * 3 + 2)] =
            static_cast<std::uint8_t>((p >> 16) & 0xFF);
      }
      if (std::fwrite(row.data(), 1, static_cast<std::size_t>(row_stride),
                      file) != static_cast<std::size_t>(row_stride)) {
        std::fclose(file);
        return false;
      }
    }
    std::fclose(file);
    return true;
  }
};

inline const char* owner_software_quad_label() { return "Software quad"; }
inline const char* owner_no_d3d_label() { return "No D3D"; }

}  // namespace verdigris::gpu
