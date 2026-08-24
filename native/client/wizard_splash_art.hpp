// wizard_splash_art.hpp — Owner Demo integration of the WIZARD splash pack
// (TASK-0179 assets) and the Framekit nine-slice panel (TASK-0167 art,
// TASK-0180 contract) for the Chronicles front door.
//
// Presentation only: loads derived splash plates and the Framekit panel
// texture, provides cover/fit drawing and a nine-slice frame painter. The
// front door keeps its text layout, render-list ops, and input contract;
// only the backdrop and panel presentation change.
#pragma once

#include "wizard_orb_art.hpp"

#include <string>
#include <vector>

namespace wizard_splash_art {

using wizard_orb_art::OrbBitmap;
using wizard_orb_art::GdiPlusProcs;
using wizard_orb_art::load_orb_plate;
using wizard_orb_art::release_orb_bitmap;
using wizard_orb_art::AlphaBlendProc;

struct SplashArtSet {
  OrbBitmap planet;      // background_primary_1600.png (RGB disc on black)
  OrbBitmap milkyway;    // atmosphere_milkyway.png (RGBA band)
  OrbBitmap panel;       // framekit textures/panel.png (48x48 nine-slice)

  [[nodiscard]] bool ok() const { return planet.ok(); }
  [[nodiscard]] bool panel_ok() const { return panel.ok(); }
};

inline std::vector<std::string> splash_art_roots() {
  std::vector<std::string> roots;
  char exe_buffer[MAX_PATH];
  const DWORD exe_len = GetModuleFileNameA(nullptr, exe_buffer, MAX_PATH);
  std::string executable;
  if (exe_len > 0 && exe_len < MAX_PATH) {
    std::string path(exe_buffer, exe_len);
    const std::size_t slash = path.find_last_of("\\/");
    executable = slash == std::string::npos ? "." : path.substr(0, slash);
  }
  const std::string relative = "native\\client\\assets\\wizard\\splash";
  const std::string framekit_relative =
      "native\\client\\assets\\wizard\\framekit\\textures";
  std::vector<std::string> bases;
  if (!executable.empty()) {
    bases.push_back(executable);
    std::string prefix = executable;
    for (int depth = 1; depth <= 6; ++depth) {
      prefix += "\\..";
      bases.push_back(prefix);
    }
    roots.push_back(executable + "\\assets\\wizard\\splash");
  }
  bases.push_back(".");
  std::string walk = ".";
  for (int depth = 0; depth <= 4; ++depth) {
    bases.push_back(walk);
    walk += "\\..";
  }
  std::vector<std::string> framekit_roots;
  for (const auto& base : bases) {
    roots.push_back(base + "\\" + relative);
    framekit_roots.push_back(base + "\\" + framekit_relative);
  }
  // Panel resolution is tracked separately by the loader via the same walk.
  for (const auto& base : bases) {
    roots.push_back(base + "|" + framekit_relative);
  }
  return roots;
}

// Loads planet + milkyway + framekit panel. Returns true when the planet
// plate (the identity-carrying minimum) is present; panel is optional.
inline bool load_splash_art_set(const GdiPlusProcs& procs, SplashArtSet* set) {
  char exe_buffer[MAX_PATH];
  const DWORD exe_len = GetModuleFileNameA(nullptr, exe_buffer, MAX_PATH);
  std::string executable;
  if (exe_len > 0 && exe_len < MAX_PATH) {
    std::string path(exe_buffer, exe_len);
    const std::size_t slash = path.find_last_of("\\/");
    executable = slash == std::string::npos ? "." : path.substr(0, slash);
  }
  std::vector<std::string> bases;
  if (!executable.empty()) {
    bases.push_back(executable);
    std::string prefix = executable;
    for (int depth = 1; depth <= 6; ++depth) {
      prefix += "\\..";
      bases.push_back(prefix);
    }
  }
  bases.push_back(".");
  std::string walk = ".";
  for (int depth = 0; depth <= 4; ++depth) {
    bases.push_back(walk);
    walk += "\\..";
  }

  auto directory_exists = [](const std::string& path) {
    DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY);
  };

  for (const auto& base : bases) {
    const std::string splash_root =
        base + "\\native\\client\\assets\\wizard\\splash";
    if (!directory_exists(splash_root)) continue;
    const bool planet =
        load_orb_plate(procs, splash_root + "\\background_primary_1600.png",
                       &set->planet);
    if (!planet) continue;
    load_orb_plate(procs, splash_root + "\\atmosphere_milkyway.png",
                   &set->milkyway);
    for (const auto& candidate : bases) {
      const std::string panel_path =
          candidate +
          "\\native\\client\\assets\\wizard\\framekit\\textures\\panel.png";
      if (load_orb_plate(procs, panel_path, &set->panel)) break;
    }
    return true;
  }
  return false;
}

inline void release_splash_art_set(SplashArtSet* set) {
  release_orb_bitmap(&set->planet);
  release_orb_bitmap(&set->milkyway);
  release_orb_bitmap(&set->panel);
}

struct BlendProcs {
  AlphaBlendProc alpha_blend = nullptr;
};

inline void stretch_alpha(HDC dst, AlphaBlendProc alpha_blend,
                          const OrbBitmap& plate, int dest_x, int dest_y,
                          int dest_w, int dest_h, std::uint8_t src_alpha) {
  if (!alpha_blend || !plate.ok() || dest_w <= 0 || dest_h <= 0 ||
      src_alpha == 0) {
    return;
  }
  BLENDFUNCTION blend{};
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = src_alpha;
  blend.AlphaFormat = AC_SRC_ALPHA;
  alpha_blend(dst, dest_x, dest_y, dest_w, dest_h, plate.dc, 0, 0, plate.width,
              plate.height, blend);
}

// Fit-height disc: the planet keeps its aspect, fills the window height, and
// centers horizontally. Space around the disc stays the caller's backdrop.
inline void draw_planet_fit_height(HDC dst, AlphaBlendProc alpha_blend,
                                   const OrbBitmap& planet, int window_w,
                                   int window_h) {
  if (!planet.ok()) return;
  const int dest_h = window_h;
  const int dest_w = planet.width * window_h / planet.height;
  const int dest_x = (window_w - dest_w) / 2;
  stretch_alpha(dst, alpha_blend, planet, dest_x, 0, dest_w, dest_h, 255);
}

// Milky way band stretched across the window behind the disc.
inline void draw_milkyway_band(HDC dst, AlphaBlendProc alpha_blend,
                               const OrbBitmap& milkyway, int window_w,
                               int band_y, int band_h) {
  if (!milkyway.ok()) return;
  stretch_alpha(dst, alpha_blend, milkyway, 0, band_y, window_w, band_h, 80);
}

inline void fill_rect_alpha(HDC dst, AlphaBlendProc alpha_blend, int x, int y,
                            int w, int h, std::uint8_t alpha) {
  if (!alpha_blend || w <= 0 || h <= 0) return;
  HDC screen = GetDC(nullptr);
  HDC memory = CreateCompatibleDC(screen);
  ReleaseDC(nullptr, screen);
  // Solid-color alpha veil via a 1x1 DIB stretched over the region.
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = 1;
  info.bmiHeader.biHeight = -1;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* bits = nullptr;
  HBITMAP veil = CreateDIBSection(memory, &info, DIB_RGB_COLORS, &bits,
                                  nullptr, 0);
  if (veil) {
    std::uint8_t* pixel = static_cast<std::uint8_t*>(bits);
    pixel[0] = 6;
    pixel[1] = 9;
    pixel[2] = 8;
    pixel[3] = alpha;
    void* old = SelectObject(memory, veil);
    BLENDFUNCTION blend{};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    alpha_blend(dst, x, y, w, h, memory, 0, 0, 1, 1, blend);
    SelectObject(memory, old);
    DeleteObject(veil);
  }
  DeleteDC(memory);
}

// Framekit nine-slice panel: 48x48 source, 12px corners/edges (margins from
// nine_slice metadata), center and edges stretched to the destination frame.
inline void draw_nine_slice_panel(HDC dst, AlphaBlendProc alpha_blend,
                                  const OrbBitmap& panel, int x, int y, int w,
                                  int h, std::uint8_t alpha) {
  if (!alpha_blend || !panel.ok() || w <= 0 || h <= 0) return;
  constexpr int kSourceSize = 48;
  constexpr int kMargin = 12;
  const int src_inner = kSourceSize - 2 * kMargin;
  int corner = kMargin;
  if (w < 2 * kMargin) corner = w / 2;
  if (h < 2 * kMargin && h > 0) corner = (corner < h / 2) ? corner : h / 2;
  const int inner_w = w - 2 * corner;
  const int inner_h = h - 2 * corner;
  if (inner_w < 0 || inner_h < 0) return;

  BLENDFUNCTION blend{};
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = alpha;
  blend.AlphaFormat = AC_SRC_ALPHA;

  auto blit = [&](int sx, int sy, int sw, int sh, int dx, int dy, int dw,
                  int dh) {
    if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0) return;
    alpha_blend(dst, dx, dy, dw, dh, panel.dc, sx, sy, sw, sh, blend);
  };

  const int s = kSourceSize;
  const int m = kMargin;
  // Corners (1:1).
  blit(0, 0, m, m, x, y, corner, corner);
  blit(s - m, 0, m, m, x + w - corner, y, corner, corner);
  blit(0, s - m, m, m, x, y + h - corner, corner, corner);
  blit(s - m, s - m, m, m, x + w - corner, y + h - corner, corner, corner);
  // Edges (stretched).
  blit(m, 0, src_inner, m, x + corner, y, inner_w, corner);
  blit(m, s - m, src_inner, m, x + corner, y + h - corner, inner_w, corner);
  blit(0, m, m, src_inner, x, y + corner, corner, inner_h);
  blit(s - m, m, m, src_inner, x + w - corner, y + corner, corner, inner_h);
  // Center (stretched).
  blit(m, m, src_inner, src_inner, x + corner, y + corner, inner_w, inner_h);
}

}  // namespace wizard_splash_art
