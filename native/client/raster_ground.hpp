#pragma once

// World-space material composition for the pixel floor. Authored raster art
// supplies surface detail; a small landmark layout supplies broad roads and
// planting. Neither the material field nor its cache participates in collision.
#ifdef _WIN32
#include "raster_art.hpp"
#include <array>

namespace raster_ground {

inline constexpr int kTilePixels = 64;
// At most 20x20 cells including the arena's one-tile cache skirt. Leave room
// for that entire working set plus a camera transition without LRU thrashing.
inline constexpr std::size_t kMaxTiles = 512;  // 8 MiB of logical pixel surfaces.

struct Point { double x = 0, y = 0; };
struct Road { Point a{}, b{}; double radius = 45; };
struct Patch { Point center{}; double radius = 0; };
struct Layout {
  std::array<Road, 16> roads{};
  std::array<Patch, 32> planting{};
  std::array<Patch, 32> solids{};
  int road_count = 0, planting_count = 0, solid_count = 0;
  double tile_units = 1;
  std::uint64_t key = 0;
  bool active = false;

  void road(double ax, double ay, double bx, double by, double radius) {
    if (road_count < static_cast<int>(roads.size()))
      roads[road_count++] = {{ax, ay}, {bx, by}, radius};
  }
};

inline std::uint32_t hash(int x, int y) {
  std::uint32_t h = static_cast<std::uint32_t>(x) * 374761393u +
                    static_cast<std::uint32_t>(y) * 668265263u;
  h = (h ^ (h >> 13)) * 1274126177u;
  return h ^ (h >> 16);
}

inline double smooth(double t) {
  t = std::clamp(t, 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

inline double noise(double x, double y) {
  const int ix = static_cast<int>(std::floor(x));
  const int iy = static_cast<int>(std::floor(y));
  const double fx = smooth(x - ix), fy = smooth(y - iy);
  const auto v = [](int a, int b) { return (hash(a, b) & 65535u) / 65535.0; };
  const double a = v(ix, iy) * (1 - fx) + v(ix + 1, iy) * fx;
  const double b = v(ix, iy + 1) * (1 - fx) + v(ix + 1, iy + 1) * fx;
  return a * (1 - fy) + b * fy;
}

inline double distance_to_segment(Point p, const Road& road) {
  const double dx = road.b.x - road.a.x, dy = road.b.y - road.a.y;
  const double length2 = dx * dx + dy * dy;
  const double t = length2 > 0
      ? std::clamp(((p.x - road.a.x) * dx + (p.y - road.a.y) * dy) / length2, 0.0, 1.0)
      : 0.0;
  return std::hypot(p.x - road.a.x - t * dx, p.y - road.a.y - t * dy);
}

struct Coverage { double road = 0, planting = 0, shade = 0; };

inline Coverage coverage(const Layout& layout, double x, double y) {
  const Point p{x, y};
  const double edge = (noise(x / 54.0, y / 54.0) - 0.5) * 15.0;
  Coverage value;
  for (int i = 0; i < layout.road_count; ++i) {
    const Road& r = layout.roads[i];
    value.road = std::max(value.road,
        smooth((r.radius + edge + 14.0 - distance_to_segment(p, r)) / 28.0));
  }
  for (int i = 0; i < layout.solid_count; ++i) {
    const Patch& s = layout.solids[i];
    const double distance = std::hypot(x - s.center.x, y - s.center.y);
    // Worn ground stops at an obstacle's perimeter; it is not an implied
    // passage through the underlying authoritative collision circle.
    value.road *= smooth((distance - s.radius + 4.0) / 18.0);
  }
  for (int i = 0; i < layout.planting_count; ++i) {
    const Patch& s = layout.planting[i];
    const double distance = std::hypot(x - s.center.x, y - s.center.y);
    value.planting = std::max(value.planting,
        smooth((s.radius + edge * 2.0 - distance) / 65.0));
  }
  value.planting *= 1.0 - value.road;
  value.shade = noise(x / 170.0, y / 170.0);
  return value;
}

namespace detail {
using Pixels = std::array<std::uint32_t, kTilePixels * kTilePixels>;
struct Patterns {
  Pixels earth{}, moss{};
  std::array<std::uint16_t, kTilePixels * kTilePixels> pair_index{};
  std::vector<std::array<std::uint32_t, 144>> material_colors;
  std::string earth_name;
  bool ready = false;
  std::uint64_t generation = 0;
};
struct Entry {
  std::unique_ptr<raster_art::detail::Surface> surface;
  std::uint64_t used = 0;
};
using Key = std::tuple<std::uint64_t, double, int, int>;
struct Cache {
  Patterns patterns;
  std::map<Key, Entry> tiles;
  std::uint64_t clock = 0, builds = 0, hits = 0;
};
inline Cache& cache() { static Cache result; return result; }

inline std::uint32_t material(std::uint32_t earth, std::uint32_t moss,
                              const Coverage& field);

inline bool copy_pattern(const char* name, Pixels& out) {
  auto* source = raster_art::detail::asset(name);
  if (!source || !source->opaque) return false;
  auto* surface = raster_art::detail::scaled(*source, kTilePixels, kTilePixels, false);
  if (!surface || !surface->pixels) return false;
  std::memcpy(out.data(), surface->pixels, sizeof(out));
  return true;
}

inline bool patterns(const char* earth_name) {
  Cache& c = cache();
  if (c.patterns.ready && c.patterns.earth_name == earth_name &&
      c.patterns.generation == raster_art::asset_generation()) return true;
  Patterns next;
  if (!copy_pattern(earth_name, next.earth) ||
      !copy_pattern("terrain_moss", next.moss)) return false;
  // The generated textures have small palettes. Precompute their discrete
  // material combinations once rather than rounding/mixing three channels
  // for every newly revealed world pixel while the player moves.
  std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint16_t> pairs;
  for (std::size_t i = 0; i < next.earth.size(); ++i) {
    const auto pair = std::make_pair(next.earth[i], next.moss[i]);
    const auto [found, inserted] = pairs.try_emplace(pair,
        static_cast<std::uint16_t>(next.material_colors.size()));
    if (inserted) {
      std::array<std::uint32_t, 144> colors{};
      for (int shade = 0; shade < 4; ++shade)
        for (int planting = 0; planting < 6; ++planting)
          for (int road = 0; road < 6; ++road)
            colors[shade * 36 + planting * 6 + road] = material(pair.first,
                pair.second, {road / 5.0, planting / 5.0, shade / 3.0});
      next.material_colors.push_back(colors);
    }
    next.pair_index[i] = found->second;
  }
  next.earth_name = earth_name;
  next.ready = true;
  next.generation = raster_art::asset_generation();
  c.patterns = std::move(next);
  c.tiles.clear();
  return true;
}

inline std::uint32_t material(std::uint32_t earth, std::uint32_t moss,
                              const Coverage& field) {
  // Six broad, discrete shade bands preserve pixel clusters. Material edges
  // are authored at logical pixel resolution, never blurred at screen scale.
  const double road = std::round(field.road * 5.0) / 5.0;
  const double planting = std::round(field.planting * 5.0) / 5.0;
  const double shade = std::round(field.shade * 3.0) / 3.0;
  const double brightness = 0.80 + road * 0.21 + shade * 0.055;
  std::uint32_t result = 0xff000000u;
  for (int shift : {0, 8, 16}) {
    const double base = static_cast<double>((earth >> shift) & 255u);
    const double leaf = static_cast<double>((moss >> shift) & 255u);
    const double planted = base * 0.60 + leaf * 0.40;
    const int channel = static_cast<int>(std::lround(
        (base * (1 - planting) + planted * planting) * brightness));
    result |= static_cast<std::uint32_t>(std::clamp(channel, 0, 255)) << shift;
  }
  return result;
}

inline raster_art::detail::Surface* tile(const Layout& layout, int tx, int ty) {
  Cache& c = cache();
  const Key key{layout.key, layout.tile_units, tx, ty};
  auto found = c.tiles.find(key);
  if (found != c.tiles.end()) {
    found->second.used = ++c.clock;
    ++c.hits;
    return found->second.surface.get();
  }
  if (c.tiles.size() >= kMaxTiles) {
    auto oldest = std::min_element(c.tiles.begin(), c.tiles.end(),
        [](const auto& a, const auto& b) { return a.second.used < b.second.used; });
    c.tiles.erase(oldest);
  }
  auto surface = std::make_unique<raster_art::detail::Surface>();
  if (!surface->create(kTilePixels, kTilePixels)) return nullptr;
  auto* pixels = static_cast<std::uint32_t*>(surface->pixels);
  const double step = layout.tile_units / kTilePixels;
  // Roads and planting vary over tens of world units. Evaluate that broad
  // field on a shared world lattice, then interpolate before discrete shade
  // selection. Computing every distance/noise term per source pixel made a
  // newly revealed pair of tile columns stall movement for over a frame.
  constexpr int field_step = 4;
  constexpr int field_side = kTilePixels / field_step + 1;
  std::array<Coverage, field_side * field_side> field{};
  Coverage* const samples = field.data();
  for (int y = 0; y < field_side; ++y) {
    for (int x = 0; x < field_side; ++x) {
      const double wx = (static_cast<double>(tx) * kTilePixels + x * field_step + 0.5) * step;
      const double wy = (static_cast<double>(ty) * kTilePixels + y * field_step + 0.5) * step;
      samples[static_cast<std::size_t>(y) * field_side + x] = coverage(layout, wx, wy);
    }
  }
  const auto mix = [](const Coverage& a, const Coverage& b, double t) {
    return Coverage{a.road + (b.road - a.road) * t,
                    a.planting + (b.planting - a.planting) * t,
                    a.shade + (b.shade - a.shade) * t};
  };
  // Adjacent output rows reuse the same horizontally interpolated lattice
  // rows. Preserve the original horizontal-then-vertical arithmetic order,
  // but perform 1,088 horizontal mixes instead of 8,192 per tile. This also
  // avoids relying on compiler inlining for the production build's hot loop.
  std::array<Coverage, field_side * kTilePixels> horizontal{};
  Coverage* const rows = horizontal.data();
  for (int y = 0; y < field_side; ++y) {
    const Coverage* const source_row = samples + y * field_side;
    Coverage* const output_row = rows + y * kTilePixels;
    for (int x = 0; x < kTilePixels; ++x) {
      const int fx = x / field_step;
      const double dx = (x % field_step) / static_cast<double>(field_step);
      output_row[x] = mix(source_row[fx], source_row[fx + 1], dx);
    }
  }
  const auto* const pairs = c.patterns.pair_index.data();
  const auto* const palettes = c.patterns.material_colors.data();
  for (int y = 0; y < kTilePixels; ++y) {
    const Coverage* const top = rows + (y / field_step) * kTilePixels;
    const Coverage* const bottom = top + kTilePixels;
    const double dy = (y % field_step) / static_cast<double>(field_step);
    for (int x = 0; x < kTilePixels; ++x) {
      const Coverage interpolated = mix(top[x], bottom[x], dy);
      const std::size_t i = static_cast<std::size_t>(y) * kTilePixels + x;
      const int road = static_cast<int>(interpolated.road * 5.0 + 0.5);
      const int planting = static_cast<int>(interpolated.planting * 5.0 + 0.5);
      const int shade = static_cast<int>(interpolated.shade * 3.0 + 0.5);
      pixels[i] = palettes[pairs[i]].data()[shade * 36 + planting * 6 + road];
    }
  }
  auto* result = surface.get();
  c.tiles.emplace(key, Entry{std::move(surface), ++c.clock});
  ++c.builds;
  return result;
}
}  // namespace detail

inline bool draw(HDC dc, const Layout& layout, int tx, int ty, const RECT& cell,
                 const char* earth_name) {
  const long long w = static_cast<long long>(cell.right) - cell.left;
  const long long h = static_cast<long long>(cell.bottom) - cell.top;
  if (!dc || !layout.active || w <= 0 || h <= 0 ||
      w > raster_art::detail::kMaxDimension || h > raster_art::detail::kMaxDimension ||
      !std::isfinite(layout.tile_units) || layout.tile_units <= 0) return false;
  const int width = static_cast<int>(w), height = static_cast<int>(h);
  if (!RectVisible(dc, &cell)) return true;
  if (!detail::patterns(earth_name)) return false;
  auto* tile = detail::tile(layout, tx, ty);
  if (!tile) return false;
  const int previous = SetStretchBltMode(dc, COLORONCOLOR);
  const bool painted = StretchBlt(dc, cell.left, cell.top, width, height,
      tile->dc, 0, 0, kTilePixels, kTilePixels, SRCCOPY) != FALSE;
  if (previous) SetStretchBltMode(dc, previous);
  return painted;
}

}  // namespace raster_ground
#endif
