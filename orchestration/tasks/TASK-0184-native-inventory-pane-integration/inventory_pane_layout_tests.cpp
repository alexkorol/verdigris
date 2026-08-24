// inventory_pane_layout_tests.cpp — TASK-0184 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "inventory_pane_layout.hpp"

using namespace inventory_pane_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

inventory_grid::State make_grid_with_dagger() {
  inventory_grid::State grid = inventory_grid::make_default();
  inventory_grid::Item item;
  item.id = 101;
  item.width = 1;
  item.height = 1;
  item.stack_count = 1;
  item.stack_max = 1;
  item.x = 0;
  item.y = 0;
  check(inventory_grid::place(grid, item) == inventory_grid::Status::Ok,
        "place dagger");
  return grid;
}

void test_pane_position_1080p() {
  const Viewport vp{1920, 1080};
  const PixelRect pane = gear_pane_rect(vp);
  check(pane.x == 1516, "gear_pane_rect x");
  inventory_grid::State grid = inventory_grid::make_default();
  paper_doll::State doll{};
  const LayoutPlan plan = plan_gear_overlay(vp, grid, doll);
  check(plan.valid, "layout valid");
  check(plan.pane.outer.x == 1516, "pane x pinned right");
  check(plan.pane.outer.width == 380, "pane width");
  check(plan.grid.valid(), "grid plan valid");
  check(plan.grid.columns == 12, "12 columns");
  check(plan.grid.rows == 6, "6 rows");
  check(plan.grid.cell_w > 0, "positive cell width");
}

void test_item_art_blit_planned() {
  const Viewport vp{1366, 768};
  inventory_grid::State grid = make_grid_with_dagger();
  paper_doll::State doll{};
  const LayoutPlan plan = plan_gear_overlay(vp, grid, doll);
  check(plan.item_blit_count == 1, "one item blit");
  check(plan.item_blits[0].item_id == 101u, "dagger id");
  check(plan.item_blits[0].blit.dst_w > 0, "blit width");
}

void test_doll_slot_frames() {
  const Viewport vp{1280, 720};
  inventory_grid::State grid = inventory_grid::make_default();
  paper_doll::State doll{};
  paper_doll::Item weapon;
  weapon.id = 102;
  weapon.kind = paper_doll::Kind::Weapon;
  check(paper_doll::equip(doll, weapon, paper_doll::Slot::MainHand) ==
            paper_doll::Status::Ok,
        "equip weapon");
  const LayoutPlan plan = plan_gear_overlay(vp, grid, doll);
  const DollSlotPlan& main =
      plan.doll_slots[static_cast<std::size_t>(paper_doll::Slot::MainHand)];
  check(main.slot_frame.valid, "main hand slot frame");
  check(main.has_equipped, "main hand art");
  check(main.equipped_blit.item_id == 102u, "boar pike art");
}

void test_invalid_viewport_rejected() {
  const Viewport bad{0, 768};
  inventory_grid::State grid = inventory_grid::make_default();
  paper_doll::State doll{};
  check(!plan_gear_overlay(bad, grid, doll).valid, "zero width rejected");
}

void test_checksum_stable() {
  const Viewport vp{960, 600};
  inventory_grid::State grid = make_grid_with_dagger();
  paper_doll::State doll{};
  const std::uint32_t a =
      plan_checksum(plan_gear_overlay(vp, grid, doll));
  const std::uint32_t b =
      plan_checksum(plan_gear_overlay(vp, grid, doll));
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_pane_position_1080p();
  test_item_art_blit_planned();
  test_doll_slot_frames();
  test_invalid_viewport_rejected();
  test_checksum_stable();
  std::cout << "inventory_pane_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
