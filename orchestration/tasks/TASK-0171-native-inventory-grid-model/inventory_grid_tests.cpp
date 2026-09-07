// inventory_grid_tests.cpp — TASK-0171 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "inventory_grid.hpp"

using namespace inventory_grid;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

Item make_item(std::uint32_t id, std::uint8_t w, std::uint8_t h,
               std::uint8_t x, std::uint8_t y, std::uint16_t stack = 1,
               std::uint16_t stack_max = 10) {
  Item item;
  item.id = id;
  item.width = w;
  item.height = h;
  item.x = x;
  item.y = y;
  item.stack_count = stack;
  item.stack_max = stack_max;
  return item;
}

void expect_status(Status got, Status expected, const std::string& label) {
  check(got == expected,
        label + ": expected " + name(expected) + " got " + name(got));
}

void test_default_geometry() {
  State s = make_default();
  check(s.width == kDefaultWidth, "default width 12");
  check(s.height == kDefaultHeight, "default height 6");
  check(s.valid(), "empty grid valid");
}

void test_place_and_bounds() {
  State s = make_default();
  expect_status(place(s, make_item(1, 1, 1, 0, 0)), Status::Ok, "place 1x1");
  check(s.valid() && s.occupancy_consistent(), "consistent after place");

  expect_status(place(s, make_item(2, 2, 2, 11, 0)), Status::OutOfBounds,
                "reject out of bounds width");
  expect_status(place(s, make_item(3, 2, 2, 0, 5)), Status::OutOfBounds,
                "reject out of bounds height");
}

void test_overlap_rejection() {
  State s = make_default();
  expect_status(place(s, make_item(1, 2, 2, 1, 1)), Status::Ok, "place 2x2");
  expect_status(place(s, make_item(2, 1, 1, 2, 2)), Status::Overlap,
                "reject overlapping 1x1");
  expect_status(place(s, make_item(3, 2, 2, 0, 0)), Status::Overlap,
                "reject overlapping 2x2");
}

void test_move_and_stack_metadata() {
  State s = make_default();
  expect_status(place(s, make_item(10, 2, 1, 0, 0, 5, 20)), Status::Ok,
                "place wide item");
  expect_status(place(s, make_item(11, 1, 1, 2, 0)), Status::Ok, "blocker");
  expect_status(move(s, 10, 4, 2), Status::Ok, "move to open cells");
  const std::size_t idx = find_index(s, 10);
  check(s.items[idx].stack_count == 5 && s.items[idx].stack_max == 20,
        "stack metadata preserved on move");
  expect_status(move(s, 10, 2, 0), Status::Overlap,
                "reject move into blocker cell");
}

void test_swap_accept_and_reject() {
  State s = make_default();
  expect_status(place(s, make_item(1, 2, 2, 0, 0)), Status::Ok, "swap A");
  expect_status(place(s, make_item(2, 2, 2, 4, 0)), Status::Ok, "swap B");
  expect_status(swap(s, 1, 2), Status::Ok, "swap positions");
  check(find_index(s, 1) != kMaxItems && s.items[find_index(s, 1)].x == 4,
        "item 1 moved to B slot");
  check(find_index(s, 2) != kMaxItems && s.items[find_index(s, 2)].x == 0,
        "item 2 moved to A slot");
}

void test_swap_reject_oversized() {
  State s = make_default();
  expect_status(place(s, make_item(3, 6, 2, 0, 0)), Status::Ok, "wide A");
  expect_status(place(s, make_item(4, 2, 2, 8, 0)), Status::Ok, "small B");
  expect_status(swap(s, 3, 4), Status::Overlap,
                "reject swap when footprint cannot fit swapped slot");
}

void test_remove_and_full() {
  State s = make_default();
  for (std::uint32_t id = 1; id <= static_cast<std::uint32_t>(kMaxItems); ++id) {
    const std::uint8_t idx = static_cast<std::uint8_t>(id - 1);
    const std::uint8_t x = idx % s.width;
    const std::uint8_t y = idx / s.width;
    expect_status(place(s, make_item(id, 1, 1, x, y)), Status::Ok, "fill slot");
  }
  expect_status(place(s, make_item(999, 1, 1, 0, 5)), Status::Full,
                "reject beyond item capacity");
  expect_status(remove(s, 5), Status::Ok, "remove one");
  expect_status(place(s, make_item(999, 1, 1, 0, 5)), Status::Ok,
                "place after remove");
}

void test_hover() {
  State s = make_default();
  expect_status(place(s, make_item(7, 2, 2, 3, 2)), Status::Ok, "hover item");
  expect_status(set_hover(s, 4, 3), Status::Ok, "hover inside item");
  check(hovered_item(s) == 7, "hover hits item 7");
  expect_status(set_hover(s, 0, 0), Status::Ok, "hover empty");
  check(hovered_item(s) == 0, "empty cell hover");
  expect_status(set_hover(s, 20, 0), Status::OutOfBounds, "hover oob");
  check(s.hover_x == -1, "hover cleared on oob");
}

void test_serialization_order() {
  State s = make_default();
  expect_status(place(s, make_item(30, 1, 1, 0, 0)), Status::Ok, "id 30");
  expect_status(place(s, make_item(10, 1, 1, 1, 0)), Status::Ok, "id 10");
  expect_status(place(s, make_item(20, 1, 1, 2, 0)), Status::Ok, "id 20");
  const auto order = serialization_order(s);
  check(s.items[order[0]].id == 10, "serialization first id 10");
  check(s.items[order[1]].id == 20, "serialization second id 20");
  check(s.items[order[2]].id == 30, "serialization third id 30");
}

void test_invalid_item_negative() {
  State s = make_default();
  Item bad = make_item(0, 1, 1, 0, 0);
  expect_status(place(s, bad), Status::InvalidItem, "reject id 0");
  Item zero_size = make_item(5, 0, 1, 0, 0);
  expect_status(place(s, zero_size), Status::InvalidItem, "reject zero width");
}

void test_deterministic_placement_replay() {
  State a = make_default();
  State b = make_default();
  const Item big = make_item(99, 2, 3, 5, 1, 3, 5);
  expect_status(place(a, big), Status::Ok, "replay place a");
  expect_status(place(b, big), Status::Ok, "replay place b");
  check(a == b, "deterministic placement identical");
}

}  // namespace

int main() {
  test_default_geometry();
  test_place_and_bounds();
  test_overlap_rejection();
  test_move_and_stack_metadata();
  test_swap_accept_and_reject();
  test_swap_reject_oversized();
  test_remove_and_full();
  test_hover();
  test_serialization_order();
  test_invalid_item_negative();
  test_deterministic_placement_replay();

  std::cout << "TASK-0171 inventory grid acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
