// owner_menu_input_tests.cpp — TASK-0183 bridge acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "owner_menu_input.hpp"

using namespace owner_menu_input;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

State playing_state() {
  State s;
  s.menu = menu_scene::playing_state();
  return s;
}

State gear_open_state() {
  State s = playing_state();
  const auto fd =
      input_focus::push_surface(s.focus, input_focus::Surface::Gear);
  s.focus = fd.next;
  return s;
}

void test_bare_escape_opens_pause_not_quit() {
  State s = playing_state();
  const Decision d = reduce(s, menu_scene::Intent::Escape);
  check(d.disposition == menu_scene::Disposition::Consumed,
        "bare escape consumed");
  check(d.next.menu.root == menu_scene::Root::Paused,
        "bare escape opens pause");
  check(!d.next.focus.depth, "focus stays empty");
}

void test_gear_escape_closes_before_pause() {
  State s = gear_open_state();
  const Decision d = reduce(s, menu_scene::Intent::Escape);
  check(d.disposition == menu_scene::Disposition::Consumed,
        "gear escape consumed");
  check(d.next.menu.root == menu_scene::Root::Playing,
        "still playing after gear close");
  check(d.next.focus.depth == 0, "gear closed");
}

void test_gear_then_bare_escape_opens_pause() {
  State s = gear_open_state();
  s = reduce(s, menu_scene::Intent::Escape).next;
  const Decision d = reduce(s, menu_scene::Intent::Escape);
  check(d.next.menu.root == menu_scene::Root::Paused,
        "second escape opens pause");
  check(d.disposition != menu_scene::Disposition::RequestQuit,
        "never request quit on bare escape");
}

void test_title_escape_never_quits() {
  State s;
  s.menu.root = menu_scene::Root::Title;
  const Decision d = reduce(s, menu_scene::Intent::Escape);
  check(d.disposition != menu_scene::Disposition::RequestQuit,
        "title escape never quits");
  check(d.next.menu.root == menu_scene::Root::Title, "title unchanged");
}

void test_open_gear_via_pane_reducer() {
  State s = playing_state();
  const Decision d =
      reduce_pane(s, input_focus::Intent::OpenGear);
  check(d.next.focus.depth == 1, "gear opened");
  check(d.next.focus.focused() == input_focus::Surface::Gear, "gear focused");
}

void test_gameplay_passes_when_unfocused() {
  State s = playing_state();
  const Decision d = reduce(s, menu_scene::Intent::Move);
  check(d.disposition == menu_scene::Disposition::PassToGameplay,
        "move passes to gameplay");
}

void test_gameplay_blocked_when_gear_open() {
  State s = gear_open_state();
  const Decision d = reduce(s, menu_scene::Intent::Move);
  check(d.disposition == menu_scene::Disposition::Consumed,
        "move blocked with gear open");
}

}  // namespace

int main() {
  test_bare_escape_opens_pause_not_quit();
  test_gear_escape_closes_before_pause();
  test_gear_then_bare_escape_opens_pause();
  test_title_escape_never_quits();
  test_open_gear_via_pane_reducer();
  test_gameplay_passes_when_unfocused();
  test_gameplay_blocked_when_gear_open();
  std::cout << "owner_menu_input_tests: " << g_checks << " checks passed\n";
  return 0;
}
