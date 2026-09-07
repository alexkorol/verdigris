// framekit_renderer_tests.cpp — TASK-0180 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "framekit_renderer.hpp"

using namespace framekit_renderer;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_panel_nine_slice() {
  const NineSliceAsset panel = default_panel_asset();
  const Rect dest{10, 20, 200, 120};
  const NineSlicePlan plan = plan_nine_slice(dest, panel);
  check(plan.valid, "panel plan valid");
  const BlitRegion& center =
      plan.regions[static_cast<std::size_t>(Piece::Center)];
  check(center.dst_w == 176, "center width stretched");
  check(center.dst_h == 96, "center height stretched");
  check(center.src_w == 24, "center source width");
  check(center.src_h == 24, "center source height");
  check(center.dst_x == 22, "center dst x");
  check(center.dst_y == 32, "center dst y");
}

void test_slot_minimum_size() {
  const NineSliceAsset slot = default_slot_asset();
  const Rect too_small{0, 0, 20, 20};
  check(!plan_nine_slice(too_small, slot).valid, "reject undersized dest");
  const Rect ok{0, 0, 64, 64};
  check(plan_nine_slice(ok, slot).valid, "slot fits 64x64");
}

void test_corner_fixed() {
  const NineSliceAsset panel = default_panel_asset();
  const Rect dest{0, 0, 100, 80};
  const NineSlicePlan plan = plan_nine_slice(dest, panel);
  const BlitRegion& tl =
      plan.regions[static_cast<std::size_t>(Piece::TopLeft)];
  check(tl.dst_w == 12 && tl.dst_h == 12, "top-left corner fixed");
  check(tl.src_w == 12 && tl.src_h == 12, "top-left src fixed");
}

void test_sprite_blit() {
  const TextureFootprint fp{16, 16};
  const Rect dest{5, 5, 32, 32};
  const BlitRegion blit = plan_sprite(dest, TextureId::OrbVitality, fp);
  check(blit.dst_w == 32, "sprite dest width");
  check(blit.src_w == 16, "sprite src width");
  check(blit.texture == TextureId::OrbVitality, "sprite texture id");
}

void test_deterministic_checksum() {
  const NineSliceAsset panel = default_panel_asset();
  const Rect dest{1, 2, 128, 96};
  const std::uint32_t a = plan_checksum(plan_nine_slice(dest, panel));
  const std::uint32_t b = plan_checksum(plan_nine_slice(dest, panel));
  check(a == b, "deterministic checksum");
  check(a != 0u, "nonzero checksum");
}

void test_invalid_asset() {
  NineSliceAsset bad;
  bad.insets = {20, 20, 20, 20};
  bad.source = {32, 32};
  check(!bad.valid(), "insets exceed source invalid");
  check(!plan_nine_slice({0, 0, 100, 100}, bad).valid, "bad asset rejected");
}

}  // namespace

int main() {
  test_panel_nine_slice();
  test_slot_minimum_size();
  test_corner_fixed();
  test_sprite_blit();
  test_deterministic_checksum();
  test_invalid_asset();

  std::cout << "TASK-0180 framekit renderer acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
