#pragma once

// Presentation of the authoritative, centre-addressed walkability grid.
// No generated scenery/collision entries: these rectangles only paint its cells.
#ifdef _WIN32
#include "camera2d.hpp"
#include "raster_art.hpp"
#include <vector>

namespace raster_walls {
inline constexpr const char* kAsset = "wall_stone_cutaway";
inline constexpr BYTE kCutawayAlpha = 80;
enum Edge : unsigned { North = 1, East = 2, South = 4, West = 8 };

struct Grid {
  int width = 0, height = 0;
  const std::vector<std::uint8_t>& walkable;
  bool valid() const {
    return width > 0 && height > 0 &&
        walkable.size() / static_cast<std::size_t>(width) == static_cast<std::size_t>(height) &&
        walkable.size() % static_cast<std::size_t>(width) == 0;
  }
  bool open(int x, int y) const {
    return valid() && x >= 0 && y >= 0 && x < width && y < height &&
        walkable[static_cast<std::size_t>(y) * width + x] != 0;
  }
  unsigned exposure(int x, int y) const {
    return (open(x, y - 1) ? North : 0u) | (open(x + 1, y) ? East : 0u) |
           (open(x, y + 1) ? South : 0u) | (open(x - 1, y) ? West : 0u);
  }
};

struct Module {
  int x = 0, y = 0;
  unsigned exposed = 0;
  RECT ground{}, pixels{};
  double depth = 0;
};

inline bool overlaps(const RECT& a, const RECT& b) {
  return a.left < a.right && a.top < a.bottom && b.left < b.right && b.top < b.bottom &&
      a.left < b.right && a.right > b.left && a.top < b.bottom && a.bottom > b.top;
}

inline Module place(int x, int y, unsigned exposed, const camera2d::Camera& camera,
                    const camera2d::Screen& screen, double tile) {
  const auto a = camera2d::project(camera, screen, (x - .5) * tile, (y - .5) * tile);
  const auto b = camera2d::project(camera, screen, (x + .5) * tile, (y + .5) * tile);
  const int lift = std::max(1, static_cast<int>(std::lround(tile * .5 * camera.zoom)));
  return {x, y, exposed, {a.x, a.y, b.x, b.y}, {a.x, a.y - lift, b.x, b.y},
          camera2d::draw_order_key((y + .5) * tile, x * tile)};
}

inline std::vector<Module> collect(const Grid& grid, const camera2d::Camera& camera,
                                    const camera2d::Screen& screen, double tile) {
  std::vector<Module> result;
  if (!grid.valid() || !std::isfinite(tile) || tile <= 0 ||
      !std::isfinite(camera.zoom) || camera.zoom <= 0 ||
      !std::isfinite(camera.x) || !std::isfinite(camera.y) ||
      screen.width <= 0 || screen.height <= 0) return result;
  const double hx = screen.width * .5 / camera.zoom;
  const double hy = screen.height * .5 / camera.zoom;
  const auto clamp_cell = [](double value, int count) {
    return static_cast<int>(std::clamp(value, 0.0, static_cast<double>(count - 1)));
  };
  const int x0 = clamp_cell(std::floor((camera.x - hx) / tile - .5), grid.width);
  const int x1 = clamp_cell(std::ceil((camera.x + hx) / tile + .5), grid.width);
  const int y0 = clamp_cell(std::floor((camera.y - hy) / tile - .5), grid.height);
  // A footprint below the viewport can still expose its elevated north cap.
  const int y1 = clamp_cell(std::ceil((camera.y + hy) / tile + 1.0), grid.height);
  const RECT viewport{0, 0, screen.width, screen.height};
  for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) {
    if (grid.open(x, y)) continue;
    const auto module = place(x, y, grid.exposure(x, y), camera, screen, tile);
    // Keep interiors too: deleting mask==0 cells exposes false floor holes.
    if (overlaps(module.pixels, viewport)) result.push_back(module);
  }
  return result;
}

// Same nearest-neighbour texel bounds and full-canvas pivot as draw_sprite.
inline RECT sprite_ink(const char* name, int cx, int feet, int height) {
  const auto size = raster_art::dimensions(name);
  const auto ink = raster_art::content_bounds(name);
  if (!size.valid() || height <= 0) return {};
  const int width = std::max(1, static_cast<int>(std::lround(double(height) * size.width / size.height)));
  const auto edge = [](int coordinate, int target, int source) {
    return static_cast<int>(std::ceil(double(coordinate) * target / source - .5));
  };
  return {cx - width / 2 + edge(ink.left, width, size.width),
          feet - height + edge(ink.top, height, size.height),
          cx - width / 2 + edge(ink.right, width, size.width),
          feet - height + edge(ink.bottom, height, size.height)};
}

enum class Paint { Invalid, Raster, Cutaway, MissingFallback };

inline Paint draw(HDC dc, const Module& module, const RECT& player_ink,
                  const char* name = kAsset) {
  const RECT& rect = module.pixels;
  const int width = rect.right - rect.left, height = rect.bottom - rect.top;
  if (!dc || width <= 0 || height <= 0 || width > raster_art::detail::kMaxDimension ||
      height > raster_art::detail::kMaxDimension) return Paint::Invalid;
  auto* source = raster_art::detail::asset(name);
  if (!source) {
    // Asset absence alone selects the geometric fallback. It remains in the
    // same depth pass and uses the same footprint/lift as the raster module.
    // Leave its cap open over the player while retaining the solid south face.
    const int face_y = rect.top + (height * 2 + 2) / 3;
    RECT cap{rect.left, rect.top, rect.right, face_y};
    RECT face{rect.left, face_y, rect.right, rect.bottom};
    const auto brush = static_cast<HBRUSH>(GetStockObject(DC_BRUSH));
    const COLORREF old = GetDCBrushColor(dc);
    if (!overlaps(rect, player_ink)) {
      SetDCBrushColor(dc, RGB(84, 80, 71)); FillRect(dc, &cap, brush);
    }
    SetDCBrushColor(dc, RGB(38, 35, 30)); FillRect(dc, &face, brush);
    SetDCBrushColor(dc, RGB(116, 107, 88)); FrameRect(dc, &rect, brush);
    SetDCBrushColor(dc, old);
    return Paint::MissingFallback;
  }
  // A malformed promoted asset must fail its gate, not silently substitute
  // the old block. Straight opaque edges are the contiguous module contract.
  if (source->size.width != 64 || source->size.height != 96 || !source->opaque)
    return Paint::Invalid;
  if (!overlaps(rect, player_ink))
    return raster_art::draw_ground(dc, name, rect) ? Paint::Raster : Paint::Invalid;
  auto* surface = raster_art::detail::scaled(*source, width, height, false);
  return surface && raster_art::detail::composite(dc, *surface, rect.left, rect.top,
              true, kCutawayAlpha) ? Paint::Cutaway : Paint::Invalid;
}
}  // namespace raster_walls
#endif
