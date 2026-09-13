// gate_overlay_layout_tests.cpp — TASK-0188 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "gate_overlay_layout.hpp"

using namespace gate_overlay_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

gate_interaction::GateDef make_gate(std::uint32_t id, std::uint32_t zone,
                                    std::int16_t x, std::int16_t y,
                                    const char* label) {
  gate_interaction::GateDef gate;
  gate.id = id;
  gate.destination_zone = zone;
  gate.center = {x, y};
  gate.radius = 32;
  gate.accessible = true;
  for (std::size_t i = 0; i < gate_interaction::kLabelCapacity &&
                          label[i] != '\0';
       ++i) {
    gate.label[i] = label[i];
  }
  return gate;
}

void test_highlighted_gate_ring() {
  gate_interaction::World world;
  world.gates[0] = make_gate(5, 12, 400, 300, "North");
  world.count = 1;

  const gate_interaction::State state =
      gate_interaction::update_hover(world, {400, 300});
  const Viewport vp{1920, 1080};
  const LayoutPlan plan = plan_gate_overlay(world, state, vp);

  check(plan.valid, "layout valid");
  check(plan.count == 1, "one gate entry");
  check(plan.gates[0].visible, "overlay visible");
  check(plan.gates[0].highlight == HighlightKind::Highlighted, "highlighted");
  check(plan.gates[0].ring.width == 76, "ring width includes pad");
  check(plan.gates[0].ring.x == 362, "ring x centered");
  check(plan.gates[0].label_visible, "label visible");
  check(plan.gates[0].label.width == 40, "five-char label width");
}

void test_out_of_range_dim_highlight() {
  gate_interaction::World world;
  world.gates[0] = make_gate(2, 8, 500, 500, "East");
  world.count = 1;

  const gate_interaction::State state =
      gate_interaction::update_hover(world, {540, 500});
  const Viewport vp{1280, 720};
  const LayoutPlan plan = plan_gate_overlay(world, state, vp);

  check(plan.gates[0].visible, "near gate still shows overlay");
  check(plan.gates[0].highlight == HighlightKind::OutOfRange, "out of range");
  check(plan.gates[0].label_visible, "destination label shown");
}

void test_inaccessible_warning() {
  gate_interaction::World world;
  world.gates[0] = make_gate(9, 3, 200, 200, "Blocked");
  world.gates[0].accessible = false;
  world.count = 1;

  const gate_interaction::State state =
      gate_interaction::update_hover(world, {200, 200});
  const Viewport vp{960, 600};
  const LayoutPlan plan = plan_gate_overlay(world, state, vp);

  check(plan.gates[0].highlight == HighlightKind::Inaccessible,
        "inaccessible highlight");
  check(plan.gates[0].visible, "overlay visible in range");
}

void test_non_hovered_gate_hidden() {
  gate_interaction::World world;
  world.gates[0] = make_gate(1, 2, 100, 100, "A");
  world.gates[1] = make_gate(2, 3, 900, 900, "B");
  world.count = 2;

  const gate_interaction::State state =
      gate_interaction::update_hover(world, {100, 100});
  const Viewport vp{1920, 1080};
  const LayoutPlan plan = plan_gate_overlay(world, state, vp);

  check(plan.count == 2, "both gates listed");
  check(plan.gates[0].visible, "hovered gate visible");
  check(!plan.gates[1].visible, "other gate hidden");
}

void test_checksum_stable() {
  gate_interaction::World world;
  world.gates[0] = make_gate(4, 6, 320, 240, "Gate");
  world.count = 1;
  const gate_interaction::State state =
      gate_interaction::update_hover(world, {320, 240});
  const Viewport vp{1366, 768};
  const LayoutPlan plan = plan_gate_overlay(world, state, vp);
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_highlighted_gate_ring();
  test_out_of_range_dim_highlight();
  test_inaccessible_warning();
  test_non_hovered_gate_hidden();
  test_checksum_stable();
  std::cout << "gate_overlay_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
