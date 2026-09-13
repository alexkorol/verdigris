// spell_lattice_layout_tests.cpp — TASK-0196 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "spell_lattice_layout.hpp"

using namespace spell_lattice_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_source_in_path_neighbors_clickable() {
  spell_lattice::State state = spell_lattice::make_owner_demo_plane_state();
  const LayoutPlan plan = plan_lattice_panel(Viewport{1920, 1080}, state);
  check(plan.valid, "layout valid");
  check(plan.node_count == 16u, "plane nodes laid out");
  check(plan.nodes[0].in_path, "source in path");
  check(plan.nodes[1].clickable || plan.nodes[2].clickable,
        "row-one nodes clickable");
}

void test_elemental_choice_layout() {
  spell_lattice::State state = spell_lattice::make_owner_demo_plane_state();
  check(spell_lattice::append_weave(state, spell_lattice::Node::Adversarial) ==
            spell_lattice::Status::Ok,
        "adversarial");
  check(spell_lattice::append_weave(state, spell_lattice::Node::Destructive) ==
            spell_lattice::Status::Ok,
        "destructive");
  check(spell_lattice::append_weave(state, spell_lattice::Node::Fire) ==
            spell_lattice::Status::Ok,
        "fire");
  const LayoutPlan plan = plan_lattice_panel(Viewport{1366, 768}, state);
  check(plan.elemental_choice_complete, "choice complete");
  check(spell_lattice::chosen_element(state) == spell_lattice::Node::Fire,
        "fire chosen");
}

void test_completed_weave_locks_upper_strata() {
  spell_lattice::State state = spell_lattice::make_owner_demo_plane_state();
  check(spell_lattice::append_weave(state, spell_lattice::Node::Adversarial) ==
            spell_lattice::Status::Ok,
        "step 1");
  check(spell_lattice::append_weave(state, spell_lattice::Node::Destructive) ==
            spell_lattice::Status::Ok,
        "step 2");
  check(spell_lattice::append_weave(state, spell_lattice::Node::Fire) ==
            spell_lattice::Status::Ok,
        "step 3");
  const LayoutPlan plan = plan_lattice_panel(Viewport{1280, 720}, state);
  check(!plan.nodes[1].clickable, "adversarial locked after choice");
}

void test_checksum_stable() {
  spell_lattice::State state = spell_lattice::make_owner_demo_plane_state();
  const LayoutPlan plan = plan_lattice_panel(Viewport{960, 600}, state);
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_source_in_path_neighbors_clickable();
  test_elemental_choice_layout();
  test_completed_weave_locks_upper_strata();
  test_checksum_stable();
  std::cout << "spell_lattice_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
