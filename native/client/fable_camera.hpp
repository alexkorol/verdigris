// Fable reference camera and render-only elevation. No window, GPU or core
// dependency. Ported equations: docs/reference/fable-demo/docs/ARCHITECTURE.md
// sections2/3 and src/game_template.html updateProjection/projT/terrainHD.
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>

namespace fable {

struct Camera {
  double x = 0.0;
  double y = 0.0;
  double zoom = 1.0;  // pixels per native world unit at the focus plane
  double width = 0.0;
  double height = 0.0;
  double horizon = -0.45;  // fractions of viewport height, not world units
  double focus = 0.65;
  double near_depth = 40.0;  // same world units as x/y and terrain elevation
  double far_multiplier = 3.2;
};

// The GPU receives these scalars verbatim (converted to float at upload).
// Do not derive another horizon/zoom/height in the terrain draw path.
struct Projection {
  double width = 0.0, height = 0.0;
  double cam_x = 0.0, cam_y = 0.0;
  double zoom = 0.0, horizon = 0.0, focus = 0.0;
  double dzp = 0.0, d0 = 0.0, k = 0.0, a = 0.0;
  double near_depth = 0.0, far_depth = 0.0;
  bool valid = false;
};

struct Point {
  double x = 0.0, y = 0.0;
  double scale = 0.0;  // pixels per world unit at this point's depth
  double depth = 0.0;  // raw camera depth; larger means farther away
  bool visible = false;  // near/far visibility; billboard extent clipping is separate
};

struct WorldPoint { double x = 0.0, y = 0.0; };
struct ClipPoint { double x = 0.0, y = 0.0, z = 0.0, w = 0.0; bool valid = false; };

inline double fit_zoom(double width, double height, double user_zoom = 1.0) {
  if (!std::isfinite(width) || !std::isfinite(height) || !std::isfinite(user_zoom)) return 0.05;
  return std::max(0.05, std::max(width / 1150.0, height / 1500.0) * user_zoom);
}

inline Projection make_projection(const Camera& camera) {
  Projection out;
  if (!std::isfinite(camera.width) || !std::isfinite(camera.height) ||
      camera.width < 10.0 || camera.height < 10.0 || !std::isfinite(camera.x) ||
      !std::isfinite(camera.y) || !std::isfinite(camera.zoom) ||
      !std::isfinite(camera.horizon) || !std::isfinite(camera.focus) ||
      !std::isfinite(camera.near_depth) || camera.near_depth <= 0.0 ||
      !std::isfinite(camera.far_multiplier) || camera.far_multiplier <= 1.0) return out;
  out.width = camera.width; out.height = camera.height;
  out.cam_x = camera.x; out.cam_y = camera.y;
  out.zoom = std::max(0.05, camera.zoom);
  out.horizon = camera.horizon * camera.height;
  out.focus = camera.focus * camera.height;
  const double span = out.focus - out.horizon;
  out.dzp = span / out.zoom;
  out.d0 = camera.y + out.dzp;
  out.k = out.zoom * out.dzp;
  out.a = span * out.dzp;
  out.near_depth = camera.near_depth;
  out.far_depth = out.dzp * camera.far_multiplier;
  // An invalid focus/near-plane configuration must not become giant clamped
  // sprites or poison a shader constant buffer during a zero-size resize.
  if (!std::isfinite(out.horizon) || !std::isfinite(out.focus) || !std::isfinite(out.d0) ||
      !std::isfinite(out.k) || !std::isfinite(out.a) || !std::isfinite(out.far_depth) ||
      out.dzp <= out.near_depth || span <= 0.0) return {};
  out.valid = true;
  return out;
}

inline Point project(const Projection& p, double wx, double wy, double elevation = 0.0) {
  if (!p.valid || !std::isfinite(wx) || !std::isfinite(wy) || !std::isfinite(elevation)) return {};
  Point out;
  out.depth = p.d0 - wy;
  const double dz = std::max(p.near_depth, out.depth);
  out.scale = p.k / dz;
  out.x = p.width * 0.5 + (wx - p.cam_x) * out.scale;
  out.y = p.horizon + p.a / dz - elevation * out.scale;
  if (!std::isfinite(out.x) || !std::isfinite(out.y) || !std::isfinite(out.depth)) return {};
  out.visible = out.depth >= p.near_depth && out.depth <= p.far_depth;
  return out;
}

// D3D clip z is [0,w], whereas the original WebGL shader uses [-w,w].
// An affine clip z is necessary for hardware triangle clipping: the demo's
// quadratic dz*(dz-near) maps vertex endpoints but clips crossing edges at
// the wrong depth. This correction does not change reference x/y/w or UVs.
// Use raw depth here so the GPU clips geometry crossing the near plane;
// project() only clamps hidden points to keep its CPU result finite.
inline ClipPoint d3d_clip(const Projection& p, double wx, double wy, double elevation = 0.0) {
  if (!p.valid || !std::isfinite(wx) || !std::isfinite(wy) || !std::isfinite(elevation)) return {};
  const double dz = p.d0 - wy;
  ClipPoint out;
  out.x = (2.0 / p.width) * (wx - p.cam_x) * p.k;
  out.y = dz - (2.0 / p.height) * (p.horizon * dz + p.a - elevation * p.k);
  out.z = p.far_depth * (dz - p.near_depth) / (p.far_depth - p.near_depth);
  out.w = dz;
  out.valid = std::isfinite(out.x) && std::isfinite(out.y) && std::isfinite(out.z) && std::isfinite(out.w);
  return out.valid ? out : ClipPoint{};
}

// Flat or known-elevation inverse. A pixel at/above the horizon, beyond the
// render range or inside the near plane has no pickable ground intersection.
inline std::optional<WorldPoint> unproject(const Projection& p, double sx, double sy,
                                          double elevation = 0.0) {
  if (!p.valid || !std::isfinite(sx) || !std::isfinite(sy) || !std::isfinite(elevation)) return std::nullopt;
  const double row = sy - p.horizon;
  if (row <= 0.0) return std::nullopt;
  const double dz = (p.a - elevation * p.k) / row;
  if (!std::isfinite(dz) || dz < p.near_depth || dz > p.far_depth) return std::nullopt;
  const WorldPoint out{p.cam_x + (sx - p.width * 0.5) * dz / p.k, p.d0 - dz};
  if (!std::isfinite(out.x) || !std::isfinite(out.y)) return std::nullopt;
  return out;
}

enum class SceneElevation { Interior, Outdoor };
struct GaussianHill {
  double x = 0.0, y = 0.0;
  double radius_x = 1.0, radius_y = 1.0;
  double amplitude = 0.0;
};
struct HeightField {
  SceneElevation scene = SceneElevation::Interior;
  double max_abs_height = 0.0;
  std::array<GaussianHill, 4> hills{};
};

// Scene coordinates and seed are inputs, never camera state. This is a small
// render-only relief layer, not terrain collision or a copy of the demo map.
inline HeightField make_height_field(SceneElevation scene, double min_x, double min_y,
                                      double max_x, double max_y, std::uint64_t seed = 0,
                                      double amplitude = 35.0) {
  HeightField out;
  if (scene == SceneElevation::Interior || !std::isfinite(min_x) || !std::isfinite(min_y) ||
      !std::isfinite(max_x) || !std::isfinite(max_y) || max_x <= min_x || max_y <= min_y ||
      !std::isfinite(amplitude) || amplitude <= 0.0) return out;
  const double width = max_x - min_x, height = max_y - min_y;
  if (!std::isfinite(width) || !std::isfinite(height)) return out;
  out.scene = scene;
  out.max_abs_height = amplitude;
  // Fixed unsigned mixing avoids mutable gameplay RNG or platform hash salts.
  const auto unit = [&seed]() {
    seed += 0x9e3779b97f4a7c15ULL;
    std::uint64_t bits = seed;
    bits = (bits ^ (bits >> 30)) * 0xbf58476d1ce4e5b9ULL;
    bits = (bits ^ (bits >> 27)) * 0x94d049bb133111ebULL;
    bits ^= bits >> 31;
    return static_cast<double>(bits >> 11) * (1.0 / 9007199254740992.0);
  };
  constexpr double weights[]{0.62, 0.47, 0.38, -0.42};
  for (std::size_t index = 0; index < out.hills.size(); ++index) {
    auto& hill = out.hills[index];
    hill.x = min_x + width * (0.12 + unit() * 0.76);
    hill.y = min_y + height * (0.12 + unit() * 0.76);
    hill.radius_x = std::max(amplitude * 8.0, width * (0.23 + unit() * 0.20));
    hill.radius_y = std::max(amplitude * 8.0, height * (0.23 + unit() * 0.20));
    // Positive weights sum to1.47: even coincident hill peaks stay bounded
    // without terraces or a hard clamp in the normal generated field.
    hill.amplitude = amplitude * weights[index] / 1.47;
  }
  return out;
}

inline double sample_height(const HeightField& field, double wx, double wy) {
  if (field.scene == SceneElevation::Interior || !std::isfinite(wx) || !std::isfinite(wy) ||
      !std::isfinite(field.max_abs_height) || field.max_abs_height <= 0.0) return 0.0;
  double height = 0.0;
  for (const auto& hill : field.hills) {
    if (!std::isfinite(hill.x) || !std::isfinite(hill.y) || !std::isfinite(hill.radius_x) ||
        !std::isfinite(hill.radius_y) || !std::isfinite(hill.amplitude) ||
        hill.radius_x <= 0.0 || hill.radius_y <= 0.0) continue;
    const double dx = (wx - hill.x) / hill.radius_x, dy = (wy - hill.y) / hill.radius_y;
    height += hill.amplitude * std::exp(-2.2 * (dx * dx + dy * dy));
  }
  return std::clamp(height, -field.max_abs_height, field.max_abs_height);
}

inline Point project_ground(const Projection& p, const HeightField& field, double wx, double wy) {
  return project(p, wx, wy, sample_height(field, wx, wy));
}

// Intersect the screen ray with the same smooth height used for mesh vertices
// and billboard feet. Bisection provides bounded iterative elevation correction
// and avoids a flat-plane pick offset on hills. Low outdoor relief is intended;
// this is not a general multiple-ridge occlusion query.
inline std::optional<WorldPoint> pick_ground(const Projection& p, const HeightField& field,
                                            double sx, double sy) {
  if (field.scene == SceneElevation::Interior) return unproject(p, sx, sy);
  if (!p.valid || !std::isfinite(sx) || !std::isfinite(sy) || sy <= p.horizon) return std::nullopt;
  const double row = sy - p.horizon;
  const auto point_at_depth = [&](double dz) {
    return WorldPoint{p.cam_x + (sx - p.width * 0.5) * dz / p.k, p.d0 - dz};
  };
  const auto residual = [&](double dz) {
    const auto at = point_at_depth(dz);
    return row * dz + p.k * sample_height(field, at.x, at.y) - p.a;
  };
  double low = p.near_depth, high = p.far_depth;
  const double lo_error = residual(low), hi_error = residual(high);
  if (!std::isfinite(lo_error) || !std::isfinite(hi_error) || lo_error > 0.0 || hi_error < 0.0)
    return std::nullopt;
  for (int iteration = 0; iteration < 48; ++iteration) {
    const double middle = low + (high - low) * 0.5;
    if (residual(middle) < 0.0) low = middle;
    else high = middle;
  }
  const auto out = point_at_depth(low + (high - low) * 0.5);
  const auto screen = project_ground(p, field, out.x, out.y);
  if (!screen.visible || std::abs(screen.x - sx) > 1e-5 || std::abs(screen.y - sy) > 1e-5)
    return std::nullopt;
  return out;
}

}  // namespace fable
