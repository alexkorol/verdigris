// gate_interaction_tests.cpp — TASK-0175 acceptance tests.

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "gate_interaction.hpp"

using namespace gate_interaction;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void set_label(GateDef& gate, const char* text) {
  for (std::size_t i = 0; i < kLabelCapacity; ++i) {
    gate.label[i] = text[i] != '\0' ? text[i] : '\0';
    if (text[i] == '\0') break;
  }
}

World make_world() {
  World w;
  GateDef g1;
  g1.id = 1;
  g1.destination_zone = 100;
  g1.center = {50, 50};
  g1.radius = 20;
  set_label(g1, "North Gate");
  w.gates[0] = g1;

  GateDef g2;
  g2.id = 2;
  g2.destination_zone = 200;
  g2.center = {200, 50};
  g2.radius = 15;
  set_label(g2, "East Ruins");
  w.gates[1] = g2;

  w.count = 2;
  return w;
}

void test_hover_highlight_and_label() {
  const World w = make_world();
  const State s = update_hover(w, {55, 52});
  check(s.hover == Hover::Highlighted, "in-range highlight");
  check(s.hovered_index == 0, "closest gate index");
  check(std::strcmp(hovered_label(w, s), "North Gate") == 0, "label visible");
}

void test_out_of_range() {
  const World w = make_world();
  const State s = update_hover(w, {50, 85});
  check(s.hover == Hover::OutOfRange, "near but out of range");
  const Decision d = activate(w, s, false);
  check(d.status == Status::OutOfRange, "activate blocked out of range");
}

void test_normal_and_ctrl_entry() {
  const World w = make_world();
  const State s = update_hover(w, {50, 50});
  const Decision normal = activate(w, s, false);
  check(normal.command == Command::EnterZone, "normal entry");
  check(normal.destination_zone == 100, "destination zone");
  const Decision fresh = activate(w, s, true);
  check(fresh.command == Command::FreshInstance, "ctrl fresh instance");
}

void test_inaccessible_gate() {
  World w = make_world();
  w.gates[0].accessible = false;
  const State s = update_hover(w, {50, 50});
  check(s.hover == Hover::Inaccessible, "inaccessible hover");
  const Decision d = activate(w, s, false);
  check(d.status == Status::Inaccessible, "inaccessible activate rejected");
}

void test_closest_gate_wins() {
  const World w = make_world();
  const State s = update_hover(w, {198, 52});
  check(s.hovered_index == 1, "east gate closer");
}

void test_no_gate_not_found() {
  World w;
  w.count = 0;
  const State s = update_hover(w, {0, 0});
  check(s.hover == Hover::None, "no gates");
  const Decision d = activate(w, s, false);
  check(d.status == Status::NotFound, "not found");
}

void test_deterministic_replay() {
  const World w = make_world();
  const State a = update_hover(w, {55, 50});
  const State b = update_hover(w, {55, 50});
  check(a.hovered_index == b.hovered_index && a.hover == b.hover,
        "hover deterministic");
  check(activate(w, a, true) == activate(w, b, true), "activate deterministic");
}

}  // namespace

int main() {
  test_hover_highlight_and_label();
  test_out_of_range();
  test_normal_and_ctrl_entry();
  test_inaccessible_gate();
  test_closest_gate_wins();
  test_no_gate_not_found();
  test_deterministic_replay();

  std::cout << "TASK-0175 gate interaction acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
