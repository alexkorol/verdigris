#include "../client/fable_camera.hpp"

#include <cmath>
#include <cstdio>
#include <limits>

namespace {
int failures = 0;
void expect(bool condition, const char* label) {
  if (!condition) { std::printf("FAIL: %s\n", label); ++failures; }
}
bool close(double a, double b, double tolerance = 1e-8) { return std::abs(a - b) <= tolerance; }

// Independently evaluate the actual vertex contract with float arithmetic,
// including D3D's [0,1] depth convention and viewport transform.
fable::Point shader_screen(const fable::Projection& p, double wx, double wy, double height) {
  const float w = static_cast<float>(p.width), h = static_cast<float>(p.height);
  const float cam_x = static_cast<float>(p.cam_x);
  const float d0 = static_cast<float>(p.d0), k = static_cast<float>(p.k);
  const float a = static_cast<float>(p.a), horizon = static_cast<float>(p.horizon);
  const float dz = d0 - static_cast<float>(wy);
  const float xc = (2.0f / w) * (static_cast<float>(wx) - cam_x) * k;
  const float yc = dz - (2.0f / h) * (horizon * dz + a - static_cast<float>(height) * k);
  return {(xc / dz + 1.0f) * w * 0.5f, (1.0f - yc / dz) * h * 0.5f, k / dz, dz, true};
}

void reference_camera_and_depth() {
  fable::Camera camera{380.0, 560.0, fable::fit_zoom(1366, 768), 1366, 768};
  const auto p = fable::make_projection(camera);
  expect(p.valid && close(p.horizon, -0.45 * 768) && close(p.focus, 0.65 * 768),
         "reference horizon/focus constants are viewport-relative");
  expect(close(p.dzp, (p.focus - p.horizon) / camera.zoom) &&
         close(p.a, (p.focus - p.horizon) * p.dzp) && close(p.k, camera.zoom * p.dzp),
         "camera parameters retain the published Fable equations");
  const auto focus = fable::project(p, camera.x, camera.y);
  expect(focus.visible && close(focus.x, 683) && close(focus.y, 499.2) && close(focus.scale, camera.zoom),
         "camera anchor sits on the focus row at exactly focus zoom");
  const auto upper = fable::unproject(p, 683, 768 * 0.05);
  const auto lower = fable::unproject(p, 683, 768 * 0.95);
  expect(upper && lower, "both distant and foreground ground rows can be picked");
  if (upper && lower) {
    const auto far = fable::project(p, upper->x, upper->y);
    const auto near = fable::project(p, lower->x, lower->y);
    expect(close(near.scale / far.scale, 2.8) && near.depth < far.depth && lower->y > upper->y,
           "reference perspective yields about3x foreground scale and correct world-y depth order");
  }
  const auto near_clip = fable::d3d_clip(p, camera.x, p.d0 - p.near_depth);
  const auto far_clip = fable::d3d_clip(p, camera.x, p.d0 - p.far_depth);
  expect(close(near_clip.z / near_clip.w, 0.0) && close(far_clip.z / far_clip.w, 1.0),
         "terrain near/far map to D3D normalized depth0and1");
  const auto front = fable::d3d_clip(p, camera.x, p.d0 - p.near_depth * 2.5);
  const auto behind = fable::d3d_clip(p, camera.x, p.d0 + p.near_depth * 2.5);
  const double near_t = front.z / (front.z - behind.z);
  expect(close(front.w + (behind.w - front.w) * near_t, p.near_depth),
         "hardware interpolation clips a camera-crossing terrain edge at the actual near plane");
  const auto outside_far = fable::d3d_clip(p, camera.x, p.d0 - p.far_depth * 1.5);
  const double gap0 = front.w - front.z, gap1 = outside_far.w - outside_far.z;
  const double far_t = gap0 / (gap0 - gap1);
  expect(close(front.w + (outside_far.w - front.w) * far_t, p.far_depth),
         "hardware interpolation clips a long terrain edge at the actual far plane");
  expect(!fable::project(p, camera.x, p.d0 - p.near_depth * 0.5).visible &&
         !fable::project(p, camera.x, p.d0 - p.far_depth * 1.1).visible,
         "billboard depth visibility uses the same terrain clipping range");
}

void projection_matches_shader_while_panning() {
  const auto terrain = fable::make_height_field(fable::SceneElevation::Outdoor, -3500, -2400, 3500, 2400, 42);
  for (const double width : {960.0, 1366.0, 3440.0}) {
    for (const double camera_x : {-900.25, 0.125, 1200.75}) {
      for (const double camera_y : {-700.5, 100.75, 900.25}) {
        const fable::Camera camera{camera_x, camera_y, fable::fit_zoom(width, 900), width, 900};
        const auto p = fable::make_projection(camera);
        for (const double dx : {-300.125, 0.0, 310.875}) {
          for (const double dy : {-300.25, 0.0, 100.125}) {
            const double wx = camera_x + dx, wy = camera_y + dy;
            const double height = fable::sample_height(terrain, wx, wy);
            const auto sprite = fable::project_ground(p, terrain, wx, wy);
            const auto terrain_clip = fable::d3d_clip(p, wx, wy, height);
            expect(sprite.visible && terrain_clip.valid, "pan parity fixtures remain in the camera range");
            expect(close(sprite.x, (terrain_clip.x / terrain_clip.w + 1) * p.width * 0.5) &&
                   close(sprite.y, (1 - terrain_clip.y / terrain_clip.w) * p.height * 0.5),
                   "sprite feet and the same terrain vertex have identical projection throughout pan");
            const auto shader = shader_screen(p, wx, wy, height);
            expect(close(sprite.x, shader.x, 0.002) && close(sprite.y, shader.y, 0.002) &&
                   close(sprite.scale, shader.scale, 0.00002),
                   "single-precision shader camera stays within0.002pixel of CPU sprite projection");
          }
        }
      }
    }
  }
  // A stationary prop and its surface sample must remain coincident while the
  // camera moves. Their scale is allowed to change: this is perspective.
  const double wx = 300.25, wy = 210.75;
  const double fixed_height = fable::sample_height(terrain, wx, wy);
  for (double pan = -300; pan <= 300; pan += 37.5) {
    const auto p = fable::make_projection({pan, pan * 0.4, 1.1, 1366, 768});
    const auto sprite = fable::project_ground(p, terrain, wx, wy);
    const auto vertex = fable::d3d_clip(p, wx, wy, fixed_height);
    expect(close(sprite.y, (1 - vertex.y / vertex.w) * p.height * 0.5),
           "fixed world relief cannot slide relative to its prop during camera pan");
  }
}

void inverse_ground_picking() {
  const auto outdoor = fable::make_height_field(fable::SceneElevation::Outdoor, -3500, -2400, 3500, 2400, 901, 35);
  const auto interior = fable::make_height_field(fable::SceneElevation::Interior, -3500, -2400, 3500, 2400, 901, 35);
  for (const double zoom : {0.35, 0.83, 1.25, 2.1}) {
    const auto p = fable::make_projection({215.375, -100.625, zoom, 1366.5, 900.25, -0.55, 0.67});
    for (const double wx : {-225.125, 100.875, 690.25}) {
      for (const double wy : {-330.375, -25.125, 230.75}) {
        for (const double height : {-25.0, 0.0, 35.0}) {
          const auto screen = fable::project(p, wx, wy, height);
          const auto back = fable::unproject(p, screen.x, screen.y, height);
          expect(back && close(back->x, wx) && close(back->y, wy),
                 "fractional world/viewport coordinates round-trip at known elevation");
        }
        const auto raised = fable::project_ground(p, outdoor, wx, wy);
        const auto picked = fable::pick_ground(p, outdoor, raised.x, raised.y);
        expect(picked && close(picked->x, wx, 1e-6) && close(picked->y, wy, 1e-6),
               "iterative ground picking corrects elevation instead of returning the flat-plane offset");
        const auto flat = fable::project_ground(p, interior, wx, wy);
        const auto picked_flat = fable::pick_ground(p, interior, flat.x, flat.y);
        expect(picked_flat && close(picked_flat->x, wx) && close(picked_flat->y, wy),
               "interior picking uses the exact flat inverse");
      }
    }
    expect(!fable::pick_ground(p, outdoor, 200, p.horizon) &&
           !fable::unproject(p, 200, p.horizon - 10), "sky pixels have no fabricated ground pick");
  }
}

void bounded_scene_elevation_and_invalid_inputs() {
  const auto a = fable::make_height_field(fable::SceneElevation::Outdoor, -1200, -800, 1200, 800, 777, 35);
  const auto b = fable::make_height_field(fable::SceneElevation::Outdoor, -1200, -800, 1200, 800, 777, 35);
  const auto other = fable::make_height_field(fable::SceneElevation::Outdoor, -1200, -800, 1200, 800, 778, 35);
  const auto flat = fable::make_height_field(fable::SceneElevation::Interior, -1200, -800, 1200, 800, 777, 35);
  bool seed_changes_height = false, has_relief = false;
  for (int y = -2400; y <= 2400; y += 113) {
    for (int x = -3600; x <= 3600; x += 137) {
      const double height = fable::sample_height(a, x, y);
      expect(std::isfinite(height) && std::abs(height) <= 35, "outdoor relief is bounded across scene and margins");
      expect(height == fable::sample_height(b, x, y), "same scene/seed reconstructs identical render-only elevation");
      expect(fable::sample_height(flat, x, y) == 0, "interiors are exactly flat regardless of seed");
      seed_changes_height = seed_changes_height || std::abs(height - fable::sample_height(other, x, y)) > 0.1;
      has_relief = has_relief || std::abs(height) > 1.0;
      expect(std::abs(fable::sample_height(a, x + 0.125, y) - height) < 0.125 &&
             std::abs(fable::sample_height(a, x, y + 0.125) - height) < 0.125,
             "outdoor hills remain gentle and continuous at fractional native coordinates");
    }
  }
  expect(seed_changes_height && has_relief, "outdoor scene relief is deterministic without being identically zero");
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double infinity = std::numeric_limits<double>::infinity();
  for (const auto camera : {fable::Camera{}, fable::Camera{0, 0, 1, 0, 768},
                           fable::Camera{0, 0, 1, 1366, 0}, fable::Camera{0, 0, 1, 9.5, 768},
                           fable::Camera{0, 0, nan, 1366, 768}, fable::Camera{infinity, 0, 1, 1366, 768},
                           fable::Camera{0, 0, 1, 1366, 768, 0.7, 0.65}}) {
    const auto p = fable::make_projection(camera);
    const auto point = fable::project(p, 100, 100);
    expect(!p.valid && !point.visible && std::isfinite(point.x) && std::isfinite(point.y) &&
           !fable::unproject(p, 100, 100) && !fable::d3d_clip(p, 100, 100).valid,
           "zero/startup/invalid camera returns a finite nonrendering result");
  }
  const auto floored = fable::make_projection({0, 0, 0, 1366.5, 768.25});
  expect(floored.valid && floored.zoom == 0.05 && floored.width == 1366.5 && floored.height == 768.25,
         "zoom floor prevents division by zero without truncating fractional viewport size");
  expect(!fable::project(floored, nan, 0).visible && !fable::unproject(floored, infinity, 100) &&
         fable::sample_height(a, nan, 100) == 0, "nonfinite sampling inputs cannot propagate NaNs into render packets");
}
}  // namespace

int main() {
  reference_camera_and_depth();
  projection_matches_shader_while_panning();
  inverse_ground_picking();
  bounded_scene_elevation_and_invalid_inputs();
  if (failures) { std::printf("fable camera tests: %d FAILURE(S)\n", failures); return 1; }
  std::printf("fable camera tests: PASS\n");
  return 0;
}
