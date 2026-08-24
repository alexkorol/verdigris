// spell_lattice_tests.cpp — TASK-0195 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "spell_lattice.hpp"

using namespace spell_lattice;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void expect_status(Status got, Status expected, const std::string& label) {
  check(got == expected,
        label + ": expected " + name(expected) + " got " + name(got));
}

void test_plane_adjacency_rules() {
  check(plane_can_descend(0, 0, 0), "source to adversarial");
  check(plane_can_descend(0, 0, 1), "source to natural");
  check(plane_can_descend(1, 0, 0), "adversarial to destructive");
  check(plane_can_descend(1, 1, 2), "natural to creative");
  check(!plane_can_descend(1, 1, 0), "natural to destructive blocked");
  check(plane_can_descend(2, 0, 0), "destructive to fire");
  check(plane_can_descend(2, 2, 3), "creative to earth");
}

void test_owner_demo_elemental_weave() {
  State s = make_owner_demo_plane_state();
  expect_status(append_weave(s, Node::Adversarial), Status::Ok, "alignment");
  expect_status(append_weave(s, Node::Destructive), Status::Ok, "focus");
  expect_status(append_weave(s, Node::Fire), Status::Ok, "fire element");
  check(owner_demo_elemental_choice_complete(s), "elemental choice complete");
  check(chosen_element(s) == Node::Fire, "fire chosen");
  check(std::string(school_for_path(s)) == "Chaos", "chaos school");
  check(has_tag_fire(chosen_element(s)), "fire tag");
}

void test_natural_life_fire_path() {
  State s = make_owner_demo_plane_state();
  append_weave(s, Node::Natural);
  append_weave(s, Node::Sustaining);
  append_weave(s, Node::Water);
  check(owner_demo_elemental_choice_complete(s), "water path complete");
  check(std::string(school_for_path(s)) == "Life", "life school");
}

void test_illegal_skip_stratum() {
  State s = make_owner_demo_plane_state();
  expect_status(append_weave(s, Node::Fire), Status::InvalidStratum,
                "cannot skip to element");
}

void test_slot_layout_matches_wizard() {
  check(slot_for(Node::Manifestation).row == 6, "manifestation row");
  check(kLatticeSlotCount == 16, "sixteen lattice slots");
}

}  // namespace

int main() {
  test_plane_adjacency_rules();
  test_owner_demo_elemental_weave();
  test_natural_life_fire_path();
  test_illegal_skip_stratum();
  test_slot_layout_matches_wizard();
  std::cout << "spell_lattice_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
