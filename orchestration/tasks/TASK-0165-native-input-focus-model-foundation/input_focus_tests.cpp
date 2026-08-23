// input_focus_tests.cpp — TASK-0165 acceptance tests.
//
// Self-contained: includes only the production header under test
// (native/client/input_focus.hpp, resolved by the harness include path) and
// the standard library. Exits nonzero on the first failed check, mirroring
// the convention of native/tests/core_tests.cpp and the TASK-0158 harness.

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "input_focus.hpp"

using namespace input_focus;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

State empty_state() { return State{}; }

State with_open(Surface surface) {
  State s;
  s.depth = 1;
  s.stack[0] = surface;
  return s;
}

// Canonical-form invariant: occupied slots hold open surfaces, the tail is
// zeroed. Every decision produced from a canonical state must be canonical.
void expect_canonical(const State& s, const std::string& label) {
  check(s.valid(), label + ": state is valid");
  check(s.depth <= kMaxOpenPanes, label + ": depth within capacity");
  for (std::size_t i = 0; i < static_cast<std::size_t>(s.depth); ++i) {
    check(s.stack[i] != Surface::None,
          label + ": occupied slot " + std::to_string(i) + " is open");
  }
  for (std::size_t i = static_cast<std::size_t>(s.depth);
       i < kMaxOpenPanes; ++i) {
    check(s.stack[i] == Surface::None,
          label + ": tail slot " + std::to_string(i) + " stays zeroed");
  }
}

void expect_decision(const Decision& d, Status status,
                     Disposition disposition, const State& expected_next,
                     const std::string& label) {
  check(d.status == status,
        label + ": status is " + name(status) + " (got " + name(d.status) +
            ")");
  check(d.disposition == disposition,
        label + ": disposition is " + name(disposition) + " (got " +
            name(d.disposition) + ")");
  check(d.next == expected_next, label + ": next state matches exactly");
}

// --- Section 1: no focus (gameplay owns the input) ------------------------

void test_no_focus() {
  const State empty = empty_state();

  // First contract half: with nothing left to close, bare Esc (and its
  // Cancel alias) requests quit instead of exiting unconditionally.
  for (const Intent intent : {Intent::Escape, Intent::Cancel}) {
    const Decision d = reduce(empty, intent);
    expect_decision(d, Status::Ok, Disposition::RequestQuit, empty,
                    std::string("no-focus ") + name(intent) + " requests quit");
  }

  // Gameplay intents flow to the simulation untouched.
  for (const Intent intent :
       {Intent::Move, Intent::Attack, Intent::Interact}) {
    const Decision d = reduce(empty, intent);
    expect_decision(d, Status::Ok, Disposition::PassToGameplay, empty,
                    std::string("no-focus ") + name(intent) +
                        " passes to gameplay");
  }

  // UI-only intents with no focused UI are swallowed deterministically.
  for (const Intent intent :
       {Intent::NavigatePrevious, Intent::NavigateNext, Intent::Confirm,
        Intent::TextInput}) {
    const Decision d = reduce(empty, intent);
    expect_decision(d, Status::Ok, Disposition::Consumed, empty,
                    std::string("no-focus ") + name(intent) +
                        " consumed as a no-op");
  }

  // Open hotkeys push their pane onto the empty stack.
  for (const Surface pane :
       {Surface::Gear, Surface::Character, Surface::Passive}) {
    const Intent open = pane == Surface::Gear     ? Intent::OpenGear
                        : pane == Surface::Character ? Intent::OpenCharacter
                                                       : Intent::OpenPassive;
    const Decision d = reduce(empty, open);
    check(d.status == Status::Ok,
          std::string("no-focus ") + name(open) + " succeeds");
    check(d.disposition == Disposition::Consumed,
          std::string("no-focus ") + name(open) + " is consumed");
    check(d.next.depth == 1,
          std::string("no-focus ") + name(open) + " opens exactly one pane");
    check(d.next.focused() == pane,
          std::string("no-focus ") + name(open) + " focuses " +
              name(pane));
    expect_canonical(d.next, std::string("no-focus ") + name(open));
  }

  // Unknown intents fail closed at any state.
  for (const auto raw : {kIntentCount, static_cast<std::uint8_t>(200),
                         static_cast<std::uint8_t>(255)}) {
    const Decision d = reduce(empty, static_cast<Intent>(raw));
    expect_decision(d, Status::InvalidState, Disposition::Consumed, empty,
                    "no-focus unknown intent " + std::to_string(raw) +
                        " refused");
  }
}

// --- Section 2: nonmodal panes (Gear / Character / Passive) ---------------

void test_nonmodal_pane(Surface pane) {
  const std::string who = name(pane);

  // Opened via hotkey: exactly one pane, focused.
  const State opened = reduce(empty_state(), pane == Surface::Gear
                                                  ? Intent::OpenGear
                                                  : pane == Surface::Character
                                                        ? Intent::OpenCharacter
                                                        : Intent::OpenPassive)
                           .next;
  expect_canonical(opened, who + " opened");
  check(opened.focused() == pane, who + " holds focus after open");

  // First-Esc closes the pane, second bare Esc requests quit.
  const Decision close1 = reduce(opened, Intent::Escape);
  expect_decision(close1, Status::Ok, Disposition::Consumed, empty_state(),
                  who + " first Esc closes");
  const Decision close2 = reduce(close1.next, Intent::Escape);
  expect_decision(close2, Status::Ok, Disposition::RequestQuit, empty_state(),
                  who + " second Esc requests quit");
  const Decision cancel1 = reduce(opened, Intent::Cancel);
  expect_decision(cancel1, Status::Ok, Disposition::Consumed, empty_state(),
                  who + " Cancel closes identically");

  // Gameplay input never leaks through a focused pane.
  for (const Intent intent :
       {Intent::Move, Intent::Attack, Intent::Interact}) {
    const Decision d = reduce(opened, intent);
    expect_decision(d, Status::Ok, Disposition::Consumed, opened,
                    who + " suppresses " + name(intent));
  }

  // Navigation and confirm consume deterministically; focus never shifts.
  for (const Intent intent :
       {Intent::NavigatePrevious, Intent::NavigateNext, Intent::Confirm,
        Intent::TextInput}) {
    const Decision d = reduce(opened, intent);
    expect_decision(d, Status::Ok, Disposition::Consumed, opened,
                    who + " consumes " + name(intent));
  }

  // Open-or-switch hotkeys replace the top pane without changing depth.
  for (const Surface target :
       {Surface::Gear, Surface::Character, Surface::Passive}) {
    const Intent open =
        target == Surface::Gear     ? Intent::OpenGear
        : target == Surface::Character ? Intent::OpenCharacter
                                        : Intent::OpenPassive;
    const Decision d = reduce(opened, open);
    check(d.status == Status::Ok, who + " switch to " + name(target) + " ok");
    check(d.disposition == Disposition::Consumed,
          who + " switch to " + name(target) + " consumed");
    check(d.next.depth == 1,
          who + " switch to " + name(target) + " keeps depth 1");
    check(d.next.focused() == target,
          who + " switch to " + name(target) + " focuses the target");
    expect_canonical(d.next, who + " switch to " + name(target));
  }

  // A modal dialog stacks above the pane and closes first.
  const State stacked = push_surface(opened, Surface::Modal).next;
  check(stacked.depth == 2, who + " + modal stack depth is 2");
  check(stacked.focused() == Surface::Modal,
        who + " + modal focuses the modal first");
  const Decision pop_modal = reduce(stacked, Intent::Escape);
  check(pop_modal.next == opened,
        who + " + modal Esc returns to the pane exactly");
  const Decision pop_pane = reduce(pop_modal.next, Intent::Escape);
  check(pop_pane.next == empty_state(),
        who + " + modal second Esc empties the stack");
  expect_canonical(pop_pane.next, who + " + modal fully closed");
}

// --- Section 3: stacked close priority -------------------------------------

void test_stacked_close_priority() {
  // Build Gear -> Character -> Modal by hand through the sanctioned push
  // path and close it strictly top-down.
  State s = empty_state();
  s = push_surface(s, Surface::Gear).next;
  s = push_surface(s, Surface::Character).next;
  s = push_surface(s, Surface::Modal).next;
  expect_canonical(s, "deep stack built");
  check(s.depth == 3, "deep stack depth is 3");
  check(s.stack[0] == Surface::Gear && s.stack[1] == Surface::Character &&
            s.stack[2] == Surface::Modal,
        "deep stack ordering is bottom-up Gear, Character, Modal");

  const Decision step1 = reduce(s, Intent::Escape);
  check(step1.disposition == Disposition::Consumed,
        "deep stack Esc 1 consumed (closes modal)");
  check(step1.next.focused() == Surface::Character,
        "deep stack Esc 1 reveals Character");
  const Decision step2 = reduce(step1.next, Intent::Escape);
  check(step2.disposition == Disposition::Consumed,
        "deep stack Esc 2 consumed (closes character)");
  check(step2.next.focused() == Surface::Gear,
        "deep stack Esc 2 reveals Gear");
  const Decision step3 = reduce(step2.next, Intent::Escape);
  check(step3.disposition == Disposition::Consumed,
        "deep stack Esc 3 consumed (closes gear)");
  check(step3.next.depth == 0, "deep stack Esc 3 empties the stack");
  const Decision step4 = reduce(step3.next, Intent::Escape);
  expect_decision(step4, Status::Ok, Disposition::RequestQuit, empty_state(),
                  "deep stack Esc 4 requests quit");

  // Text over a nonmodal pane behaves the same way.
  State t = push_surface(with_open(Surface::Passive), Surface::Text).next;
  check(t.focused() == Surface::Text, "passive + text focuses the text field");
  const Decision t1 = reduce(t, Intent::Escape);
  check(t1.next == with_open(Surface::Passive),
        "text Esc returns to the passive pane");
  const Decision t2 = reduce(t1.next, Intent::TextInput);
  expect_decision(t2, Status::Ok, Disposition::Consumed,
                  with_open(Surface::Passive),
                  "passive pane swallows text input");
  const Decision t3 = reduce(t2.next, Intent::Escape);
  check(t3.next.depth == 0, "passive Esc empties the stack");

  // Modal can stack over a text field; dismissal still peels one at a time.
  State m = push_surface(with_open(Surface::Text), Surface::Modal).next;
  check(m.depth == 2 && m.focused() == Surface::Modal,
        "text + modal focuses the modal");
  const Decision m1 = reduce(m, Intent::Cancel);
  check(m1.next.focused() == Surface::Text,
        "modal Cancel reveals the text field");
  const Decision m2 = reduce(m1.next, Intent::Escape);
  check(m2.next.depth == 0, "text Esc then empties the stack");
}

// --- Section 4: hard modality over a nonmodal pane -------------------------

void test_modality(Surface top) {
  const std::string who = std::string("gear + ") + name(top);
  const State base = push_surface(with_open(Surface::Gear), top).next;
  check(base.focused() == top, who + " holds focus");

  for (const Intent intent :
       {Intent::Move, Intent::Attack, Intent::Interact,
        Intent::NavigatePrevious, Intent::NavigateNext, Intent::Confirm,
        Intent::TextInput, Intent::OpenGear, Intent::OpenCharacter,
        Intent::OpenPassive}) {
    const Decision d = reduce(base, intent);
    expect_decision(d, Status::Ok, Disposition::Consumed, base,
                    who + " suppresses " + name(intent));
  }

  const Decision esc = reduce(base, Intent::Escape);
  check(esc.next == with_open(Surface::Gear),
        who + " Esc peels exactly one surface");
  const Decision cancel = reduce(base, Intent::Cancel);
  check(cancel.next == with_open(Surface::Gear),
        who + " Cancel peels exactly one surface");
}

// --- Section 5: capacity and push validation -------------------------------

void test_capacity() {
  State s = empty_state();
  for (std::uint8_t i = 0; i < kMaxOpenPanes; ++i) {
    const Surface pane = static_cast<Surface>(
        static_cast<std::uint8_t>(Surface::Gear) +
        (i % static_cast<std::uint8_t>(Surface::Text)));
    const Decision d = push_surface(s, pane);
    check(d.status == Status::Ok,
          "push " + std::to_string(i + 1) + " of " +
              std::to_string(kMaxOpenPanes) + " succeeds");
    s = d.next;
    expect_canonical(s, "push " + std::to_string(i + 1));
  }
  check(s.depth == kMaxOpenPanes, "stack fills exactly to capacity");

  const Decision overflow = push_surface(s, Surface::Modal);
  expect_decision(overflow, Status::StackFull, Disposition::Consumed, s,
                  "push past capacity is refused with the state intact");

  // Open hotkeys over a full stack replace the nonmodal top instead of
  // growing: depth stays pinned at capacity.
  const Decision switched = reduce(s, Intent::OpenCharacter);
  check(switched.status == Status::Ok,
        "open hotkey over a full stack succeeds by replacing the top");
  check(switched.next.depth == kMaxOpenPanes,
        "open hotkey over a full stack keeps depth at capacity");
  check(switched.next.focused() == Surface::Character,
        "open hotkey over a full stack switches the top pane");

  // push_surface validates the surface argument and the base state.
  expect_decision(push_surface(s, Surface::None), Status::InvalidState,
                  Disposition::Consumed, s, "pushing None is invalid");
  expect_decision(push_surface(s, static_cast<Surface>(77)),
                  Status::InvalidState, Disposition::Consumed, s,
                  "pushing an unknown surface is invalid");
  State corrupt = s;
  corrupt.depth = static_cast<std::uint8_t>(kMaxOpenPanes + 1);
  expect_decision(push_surface(corrupt, Surface::Gear), Status::InvalidState,
                  Disposition::Consumed, corrupt,
                  "pushing onto a corrupt state is invalid");
}

// --- Section 6: invalid states fail closed, never undefined ----------------

void test_invalid_states() {
  // Depth beyond capacity.
  State deep = empty_state();
  deep.depth = static_cast<std::uint8_t>(kMaxOpenPanes + 1);
  check(!deep.valid(), "over-capacity depth is invalid");
  for (const Intent intent :
       {Intent::Escape, Intent::Move, Intent::OpenGear}) {
    expect_decision(reduce(deep, intent), Status::InvalidState,
                    Disposition::Consumed, deep,
                    std::string("over-capacity state refuses ") +
                        name(intent));
  }

  // None buried inside the stack.
  State buried = with_open(Surface::Gear);
  buried.depth = 2;
  buried.stack[1] = Surface::None;
  check(!buried.valid(), "None inside the stack is invalid");
  expect_decision(reduce(buried, Intent::Escape), Status::InvalidState,
                  Disposition::Consumed, buried,
                  "buried-None state refuses Escape");

  // Unknown surface inside the stack.
  State alien = with_open(static_cast<Surface>(42));
  check(!alien.valid(), "unknown surface inside the stack is invalid");
  expect_decision(reduce(alien, Intent::Move), Status::InvalidState,
                  Disposition::Consumed, alien,
                  "unknown-surface state refuses Move");

  // Stale tail beyond depth is tolerated and ignored (canonical form is a
  // producer invariant, not a validity requirement).
  State stale = with_open(Surface::Gear);
  stale.stack[3] = Surface::Passive;
  check(stale.valid(), "stale zero-tail violation is still valid");
  const Decision d = reduce(stale, Intent::Escape);
  check(d.next.depth == 0, "stale-tail state closes to empty normally");
}

// --- Section 7: determinism -------------------------------------------------

void test_determinism() {
  // A fixed script replayed from the same seed state must produce a
  // bitwise-identical decision trace.
  const std::vector<Intent> script = {
      Intent::OpenGear,     Intent::NavigateNext,  Intent::Move,
      Intent::OpenCharacter, Intent::Escape,       Intent::TextInput,
      Intent::Attack,       Intent::Escape,        Intent::OpenPassive,
      Intent::NavigatePrevious, Intent::Confirm,   Intent::Escape,
      Intent::Move,         Intent::Escape};

  auto run_script = [&script]() {
    std::vector<Decision> trace;
    State s = empty_state();
    for (const Intent intent : script) {
      trace.push_back(reduce(s, intent));
      s = trace.back().next;
    }
    return trace;
  };

  const std::vector<Decision> first = run_script();
  const std::vector<Decision> second = run_script();
  check(first.size() == script.size() && second.size() == script.size(),
        "script traces are complete");
  for (std::size_t i = 0; i < script.size(); ++i) {
    check(first[i] == second[i],
          "determinism: trace step " + std::to_string(i) + " (" +
              name(script[i]) + ") replays identically");
  }
  for (const Decision& d : first) {
    expect_canonical(d.next, "script trace state");
  }

  // The quit request is reachable at the end of the script: every pane the
  // script opened was Esc-closed, so the final Move passes to gameplay and a
  // final Escape requests quit.
  State final_state = empty_state();
  for (const Decision& d : first) final_state = d.next;
  check(final_state.depth == 0, "script ends with an empty stack");
  const Decision final_move = reduce(final_state, Intent::Move);
  check(final_move.disposition == Disposition::PassToGameplay,
        "after the script, movement passes to gameplay again");
  const Decision final_esc = reduce(final_state, Intent::Escape);
  check(final_esc.disposition == Disposition::RequestQuit,
        "after the script, bare Esc requests quit");

  // Repeating one fixed call many times never drifts.
  const State stacked =
      push_surface(push_surface(with_open(Surface::Gear),
                                Surface::Character)
                       .next,
                   Surface::Modal)
          .next;
  const Decision fixed = reduce(stacked, Intent::Escape);
  for (int i = 0; i < 100; ++i) {
    check(reduce(stacked, Intent::Escape) == fixed,
          "determinism: repeated Esc over a deep stack is stable");
  }
}

}  // namespace

int main() {
  test_no_focus();
  test_nonmodal_pane(Surface::Gear);
  test_nonmodal_pane(Surface::Character);
  test_nonmodal_pane(Surface::Passive);
  test_stacked_close_priority();
  test_modality(Surface::Modal);
  test_modality(Surface::Text);
  test_capacity();
  test_invalid_states();
  test_determinism();

  std::cout << "TASK-0165 input focus acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
