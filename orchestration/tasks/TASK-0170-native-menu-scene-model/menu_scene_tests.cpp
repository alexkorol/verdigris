// menu_scene_tests.cpp — TASK-0170 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "menu_scene.hpp"

using namespace menu_scene;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

State title_state() {
  State s;
  s.root = Root::Title;
  return s;
}

void expect_decision(const Decision& d, Status status, Disposition disposition,
                     const State& expected_next, const std::string& label) {
  check(d.status == status,
        label + ": status is " + name(status) + " (got " + name(d.status) +
            ")");
  check(d.disposition == disposition,
        label + ": disposition is " + name(disposition) + " (got " +
            name(d.disposition) + ")");
  check(d.next == expected_next, label + ": next state matches exactly");
}

void test_title_escape_never_quits() {
  const State title = title_state();
  for (const Intent intent : {Intent::Escape, Intent::Cancel}) {
    const Decision d = reduce(title, intent);
    expect_decision(d, Status::Ok, Disposition::Consumed, title,
                    std::string("title ") + name(intent) + " consumed");
  }
}

void test_title_start_and_explicit_quit() {
  const State title = title_state();
  const Decision start = reduce(title, Intent::Start);
  expect_decision(start, Status::Ok, Disposition::Consumed, playing_state(),
                  "title start -> playing");

  const Decision quit = reduce(title, Intent::Quit);
  expect_decision(quit, Status::Ok, Disposition::RequestQuit, title,
                  "title explicit quit requests quit");
}

void test_title_gameplay_intents_consumed() {
  const State title = title_state();
  for (const Intent intent :
       {Intent::Move, Intent::Attack, Intent::Interact}) {
    const Decision d = reduce(title, intent);
    expect_decision(d, Status::Ok, Disposition::Consumed, title,
                    std::string("title ") + name(intent) + " consumed");
  }
}

void test_playing_escape_opens_pause() {
  const State playing = playing_state();
  State expected;
  expected.root = Root::Paused;
  expected.pause_depth = 1;
  expected.pause_stack[0] = PausePane::Main;

  for (const Intent intent : {Intent::Escape, Intent::Cancel}) {
    const Decision d = reduce(playing, intent);
    expect_decision(d, Status::Ok, Disposition::Consumed, expected,
                    std::string("playing ") + name(intent) + " opens pause");
  }
}

void test_playing_gameplay_passes() {
  const State playing = playing_state();
  for (const Intent intent :
       {Intent::Move, Intent::Attack, Intent::Interact}) {
    const Decision d = reduce(playing, intent);
    expect_decision(d, Status::Ok, Disposition::PassToGameplay, playing,
                    std::string("playing ") + name(intent) +
                        " passes to gameplay");
  }
}

void test_playing_escape_never_quits() {
  const State playing = playing_state();
  const Decision d = reduce(playing, Intent::Escape);
  check(d.disposition != Disposition::RequestQuit,
        "playing escape must not request quit");
}

void test_pause_resume_and_repeated_escape() {
  State paused = paused_with(PausePane::Main);

  const Decision resume = reduce(paused, Intent::Resume);
  expect_decision(resume, Status::Ok, Disposition::Consumed, playing_state(),
                  "paused resume -> playing");

  paused = paused_with(PausePane::Main);
  const Decision esc1 = reduce(paused, Intent::Escape);
  expect_decision(esc1, Status::Ok, Disposition::Consumed, playing_state(),
                  "paused main escape resumes");

  paused = paused_with(PausePane::Main);
  const Decision push = reduce(paused, Intent::OpenSettings);
  State with_settings = push.next;
  check(with_settings.focused_pause() == PausePane::Settings,
        "settings pane opened on stack");

  const Decision esc2 = reduce(with_settings, Intent::Escape);
  expect_decision(esc2, Status::Ok, Disposition::Consumed, paused,
                  "nested escape pops to main pause");

  const Decision esc3 = reduce(paused, Intent::Escape);
  expect_decision(esc3, Status::Ok, Disposition::Consumed, playing_state(),
                  "second escape resumes play");
}

void test_pause_explicit_quit_flow() {
  State paused = paused_with(PausePane::Main);

  const Decision open_confirm = reduce(paused, Intent::Quit);
  State confirm = open_confirm.next;
  check(confirm.focused_pause() == PausePane::ConfirmQuit,
        "quit from main opens confirm dialog");

  const Decision confirm_quit = reduce(confirm, Intent::Confirm);
  check(confirm_quit.disposition == Disposition::RequestQuit,
        "confirm on quit dialog requests quit");

  paused = paused_with(PausePane::Main);
  const Decision direct = reduce(paused, Intent::Quit);
  State at_confirm = direct.next;
  const Decision quit_cmd = reduce(at_confirm, Intent::Quit);
  check(quit_cmd.disposition == Disposition::RequestQuit,
        "second quit at confirm also requests quit");
}

void test_pause_escape_never_quits() {
  State paused = paused_with(PausePane::Main);
  const Decision d = reduce(paused, Intent::Escape);
  check(d.disposition != Disposition::RequestQuit,
        "pause escape must not request quit");

  State nested = push_pause_pane(paused, PausePane::Settings).next;
  const Decision d2 = reduce(nested, Intent::Escape);
  check(d2.disposition != Disposition::RequestQuit,
        "nested pause escape must not request quit");
}

void test_negative_escape_never_quits_globally() {
  std::vector<State> states;
  states.push_back(title_state());
  states.push_back(playing_state());
  states.push_back(paused_with(PausePane::Main));
  states.push_back(paused_with(PausePane::Settings));
  states.push_back(paused_with(PausePane::ConfirmQuit));

  State stacked = paused_with(PausePane::Main);
  stacked = push_pause_pane(stacked, PausePane::Help).next;
  states.push_back(stacked);

  for (const State& s : states) {
  for (const Intent intent : {Intent::Escape, Intent::Cancel}) {
      const Decision d = reduce(s, intent);
      check(d.disposition != Disposition::RequestQuit,
            std::string("negative: ") + name(s.root) + " " + name(intent) +
                " never requests quit");
    }
  }
}

void test_invalid_state_refused() {
  State corrupt;
  corrupt.root = Root::Playing;
  corrupt.pause_depth = 1;
  corrupt.pause_stack[0] = PausePane::Main;
  const Decision d = reduce(corrupt, Intent::Escape);
  check(d.status == Status::InvalidState, "playing with pause stack refused");
}

void test_determinism_replay() {
  State s = title_state();
  s = reduce(s, Intent::Start).next;
  for (int i = 0; i < 3; ++i) {
    s = reduce(s, Intent::Escape).next;
    s = reduce(s, Intent::Escape).next;
  }
  const State replay = reduce(title_state(), Intent::Start).next;
  check(s == replay, "deterministic replay stable");
}

}  // namespace

int main() {
  test_title_escape_never_quits();
  test_title_start_and_explicit_quit();
  test_title_gameplay_intents_consumed();
  test_playing_escape_opens_pause();
  test_playing_gameplay_passes();
  test_playing_escape_never_quits();
  test_pause_resume_and_repeated_escape();
  test_pause_explicit_quit_flow();
  test_pause_escape_never_quits();
  test_negative_escape_never_quits_globally();
  test_invalid_state_refused();
  test_determinism_replay();

  std::cout << "TASK-0170 menu scene acceptance: " << g_checks << " checks passed\n";
  return 0;
}
