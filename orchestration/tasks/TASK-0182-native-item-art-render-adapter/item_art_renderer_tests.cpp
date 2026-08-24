// item_art_renderer_tests.cpp — TASK-0182 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "item_art_renderer.hpp"

using namespace item_art_renderer;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_known_weapon() {
  const CellRect cell{0, 0, 64, 64};
  const ResolveResult result = resolve(101, cell);
  check(result.status == Status::Ok, "weapon found");
  check(result.entry.category == Category::Weapon, "weapon category");
  check(result.blit.dst_w <= 64, "fits cell width");
  check(result.blit.dst_h <= 64, "fits cell height");
  check(result.blit.src_w == 344, "source footprint width");
}

void test_aspect_fit_tall_item() {
  const CellRect cell{10, 10, 48, 48};
  const ResolveResult result = resolve(102, cell);
  check(result.status == Status::Ok, "pike found");
  check(result.blit.dst_h == 48, "height clamped to cell");
  check(result.blit.dst_w < 48, "width scaled down");
}

void test_not_found() {
  const CellRect cell{0, 0, 32, 32};
  check(resolve(999, cell).status == Status::NotFound, "unknown id");
}

void test_recoverable_item() {
  const ResolveResult result = resolve(602, {0, 0, 80, 80});
  check(result.status == Status::Ok, "offering bowl");
  check(result.entry.category == Category::Recoverable, "recoverable");
}

void test_catalog_complete() {
  const auto catalog = default_catalog();
  for (const CatalogEntry& entry : catalog) {
    check(entry.valid(), "catalog entry valid");
  }
}

void test_deterministic_checksum() {
  const std::uint32_t a = catalog_checksum();
  const std::uint32_t b = catalog_checksum();
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_known_weapon();
  test_aspect_fit_tall_item();
  test_not_found();
  test_recoverable_item();
  test_catalog_complete();
  test_deterministic_checksum();

  std::cout << "TASK-0182 item art renderer acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
