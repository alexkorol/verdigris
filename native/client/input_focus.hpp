// input_focus.hpp — TASK-0165 pure input-focus and pane-close foundation.
//
// Header-only, dependency-free reducer for the future native client's pane
// focus. Given the currently focused surface stack and one abstract input
// intent, reduce() returns the next stack plus an explicit disposition:
// Consumed (the UI ate the input), PassToGameplay (route to the simulation),
// or RequestQuit (ask the owner loop to exit). The model encodes the accepted
// close contract: the first Esc closes the topmost open surface, and a bare
// Esc with nothing left open requests quit — never a global instant exit
// while any pane stands. Movement/combat intents never leak through focused
// UI, and navigation inside nonmodal panes consumes deterministically.
// Fixed-capacity and pure: no heap, no clocks, no global mutable state, no
// Win32/GDI, no key codes or binding choices, and zero integration with
// main.cpp or any production runtime in this packet.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace input_focus {

// A focusable UI surface. None means "no pane is open": gameplay owns input.
enum class Surface : std::uint8_t {
  None = 0,
  Gear,
  Character,
  Passive,
  Modal,  // blocking dialog: everything except dismissal is swallowed
  Text,   // text entry: swallows movement/combat/open hotkeys entirely
};

// Abstract input intent. Deliberately key-agnostic: this packet freezes no
// keycode and no final binding; callers translate real keys into intents.
enum class Intent : std::uint8_t {
  Move,              // locomotion intent (WASD/arrows equivalent)
  Attack,            // combat activation intent
  Interact,          // context interaction intent (click target, pickup)
  NavigatePrevious,  // move selection up/back within the focused pane
  NavigateNext,      // move selection down/forward within the focused pane
  Confirm,           // activate current selection / submit a text line
  Cancel,            // semantic alias of Escape: close the topmost surface
  Escape,            // the bare Esc key intent
  OpenGear,          // open-or-switch hotkey: gear pane
  OpenCharacter,     // open-or-switch hotkey: character pane
  OpenPassive,       // open-or-switch hotkey: passive pane
  TextInput,         // printable text destined for a focused text field
};

// One past the last valid Intent enumerator; values at or above this are
// unknown (e.g. produced by casting raw integers) and fail closed.
inline constexpr std::uint8_t kIntentCount =
    static_cast<std::uint8_t>(Intent::TextInput) + 1;

enum class Disposition : std::uint8_t {
  Consumed,        // handled by the focus model/UI; forward nowhere
  PassToGameplay,  // no UI holds focus; route the intent to the simulation
  RequestQuit,     // bare Esc with nothing left to close; consult the owner
};

enum class Status : std::uint8_t {
  Ok,
  InvalidState,  // corrupt stack, unknown surface/intent; input echoed intact
  StackFull,     // open capacity exhausted; request refused, state untouched
};

// Fixed maximum of simultaneously open surfaces. Bounding depth by a
// constant keeps the model allocation-free and lets any incoming state be
// validated completely before it is ever indexed.
inline constexpr std::size_t kMaxOpenPanes = 8;

struct State {
  // 0 => empty stack => focus is None. stack[depth-1] is the focused/top
  // surface. Canonical form: every slot at or beyond depth is None.
  std::uint8_t depth = 0;
  std::array<Surface, kMaxOpenPanes> stack{};

  [[nodiscard]] constexpr Surface focused() const {
    return depth == 0
               ? Surface::None
               : stack[static_cast<std::size_t>(depth) - 1];
  }

  // A state is valid iff the depth fits the capacity and every occupied slot
  // holds a known open surface (None may only appear in an empty stack or in
  // the canonical zero tail). Invalid states never cause undefined behavior:
  // reduce() detects and refuses them.
  [[nodiscard]] constexpr bool valid() const {
    if (depth > kMaxOpenPanes) return false;
    for (std::size_t i = 0; i < static_cast<std::size_t>(depth); ++i) {
      const auto raw = static_cast<std::uint8_t>(stack[i]);
      if (raw == static_cast<std::uint8_t>(Surface::None) ||
          raw > static_cast<std::uint8_t>(Surface::Text)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

struct Decision {
  Status status = Status::Ok;
  Disposition disposition = Disposition::Consumed;
  State next{};

  [[nodiscard]] constexpr bool operator==(const Decision&) const = default;
};

[[nodiscard]] constexpr const char* name(Surface surface) {
  switch (surface) {
    case Surface::None:
      return "none";
    case Surface::Gear:
      return "gear";
    case Surface::Character:
      return "character";
    case Surface::Passive:
      return "passive";
    case Surface::Modal:
      return "modal";
    case Surface::Text:
      return "text";
  }
  return "unknown-surface";
}

[[nodiscard]] constexpr const char* name(Intent intent) {
  switch (intent) {
    case Intent::Move:
      return "move";
    case Intent::Attack:
      return "attack";
    case Intent::Interact:
      return "interact";
    case Intent::NavigatePrevious:
      return "navigate-previous";
    case Intent::NavigateNext:
      return "navigate-next";
    case Intent::Confirm:
      return "confirm";
    case Intent::Cancel:
      return "cancel";
    case Intent::Escape:
      return "escape";
    case Intent::OpenGear:
      return "open-gear";
    case Intent::OpenCharacter:
      return "open-character";
    case Intent::OpenPassive:
      return "open-passive";
    case Intent::TextInput:
      return "text-input";
  }
  return "unknown-intent";
}

[[nodiscard]] constexpr const char* name(Disposition disposition) {
  switch (disposition) {
    case Disposition::Consumed:
      return "consumed";
    case Disposition::PassToGameplay:
      return "pass-to-gameplay";
    case Disposition::RequestQuit:
      return "request-quit";
  }
  return "unknown-disposition";
}

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "ok";
    case Status::InvalidState:
      return "invalid-state";
    case Status::StackFull:
      return "stack-full";
  }
  return "unknown-status";
}

// Maps an open-hotkey intent to its pane, or None for non-open intents.
[[nodiscard]] constexpr Surface open_target(Intent intent) {
  switch (intent) {
    case Intent::OpenGear:
      return Surface::Gear;
    case Intent::OpenCharacter:
      return Surface::Character;
    case Intent::OpenPassive:
      return Surface::Passive;
    default:
      return Surface::None;
  }
}

// Builds the canonical refused decision: the offending input state is echoed
// back untouched so callers can never observe corrupted focus, and the
// disposition fails closed to Consumed (never gameplay dispatch, never quit).
[[nodiscard]] constexpr Decision refused(Status status, const State& in) {
  Decision out;
  out.status = status;
  out.disposition = Disposition::Consumed;
  out.next = in;
  return out;
}

// Opens `pane` on top of the stack through the exact validation path reduce()
// uses, so future dialog/text owners cannot bypass capacity or state checks.
// This is the only sanctioned way to introduce Modal/Text surfaces, which no
// intent in this packet opens.
[[nodiscard]] constexpr Decision push_surface(const State& in, Surface pane) {
  if (!in.valid()) return refused(Status::InvalidState, in);
  const auto raw = static_cast<std::uint8_t>(pane);
  if (raw == static_cast<std::uint8_t>(Surface::None) ||
      raw > static_cast<std::uint8_t>(Surface::Text)) {
    return refused(Status::InvalidState, in);
  }
  if (in.depth >= kMaxOpenPanes) return refused(Status::StackFull, in);
  Decision out;
  out.next = in;
  out.next.stack[out.next.depth] = pane;
  ++out.next.depth;
  return out;
}

// The reducer. Pure: the input state is never modified, the same (state,
// intent) pair always yields the identical decision, and every branch below
// preserves the canonical zero-tail form of State.
[[nodiscard]] constexpr Decision reduce(const State& in, Intent intent) {
  if (!in.valid()) return refused(Status::InvalidState, in);
  if (static_cast<std::uint8_t>(intent) >= kIntentCount) {
    return refused(Status::InvalidState, in);
  }

  // Open hotkeys share one funnel: from an empty stack they push, over a
  // nonmodal pane they replace the top (switch), under Modal/Text focus they
  // are suppressed along with everything else.
  const Surface target = open_target(intent);

  Decision out;
  out.next = in;
  const std::size_t d = static_cast<std::size_t>(in.depth);

  switch (in.focused()) {
    case Surface::None:
      switch (intent) {
        case Intent::Escape:
        case Intent::Cancel:
          // Nothing left to close: the accepted second-bare-Esc contract
          // asks the owner loop to quit instead of exiting unconditionally.
          out.disposition = Disposition::RequestQuit;
          return out;
        case Intent::Move:
        case Intent::Attack:
        case Intent::Interact:
          // No pane holds focus; gameplay receives the input untouched.
          out.disposition = Disposition::PassToGameplay;
          return out;
        case Intent::NavigatePrevious:
        case Intent::NavigateNext:
        case Intent::Confirm:
        case Intent::TextInput:
          // UI-only intents with no focused UI are safely swallowed.
          return out;
        case Intent::OpenGear:
        case Intent::OpenCharacter:
        case Intent::OpenPassive:
          return push_surface(in, target);
      }
      break;

    case Surface::Gear:
    case Surface::Character:
    case Surface::Passive:
      switch (intent) {
        case Intent::Escape:
        case Intent::Cancel:
          // Close-topmost: pop the focused pane; the surface beneath (or an
          // empty stack) becomes the next focus.
          out.next.stack[d - 1] = Surface::None;
          --out.next.depth;
          return out;
        case Intent::Move:
        case Intent::Attack:
        case Intent::Interact:
          // Focused UI suppresses gameplay input: nothing leaks through.
          return out;
        case Intent::NavigatePrevious:
        case Intent::NavigateNext:
        case Intent::Confirm:
        case Intent::TextInput:
          // Deterministic in-pane navigation consumption; a nonmodal pane
          // has no text field, so typed input is swallowed too.
          return out;
        case Intent::OpenGear:
        case Intent::OpenCharacter:
        case Intent::OpenPassive:
          // Open-or-switch between nonmodal panes keeps the depth stable.
          out.next.stack[d - 1] = target;
          return out;
      }
      break;

    case Surface::Modal:
    case Surface::Text:
      switch (intent) {
        case Intent::Escape:
        case Intent::Cancel:
          // Modal dialogs and text fields dismiss exactly like any other
          // topmost surface; Esc is never a global exit beneath them.
          out.next.stack[d - 1] = Surface::None;
          --out.next.depth;
          return out;
        case Intent::Move:
        case Intent::Attack:
        case Intent::Interact:
          // Hard modality: gameplay input cannot leak through.
          return out;
        case Intent::NavigatePrevious:
        case Intent::NavigateNext:
          return out;
        case Intent::Confirm:
          // Confirmation effects belong to the owning dialog/field outside
          // this model; the model only records that input was consumed.
          return out;
        case Intent::TextInput:
          // Under Text focus this feeds the field; under Modal focus it is
          // swallowed. Either way it never reaches gameplay.
          return out;
        case Intent::OpenGear:
        case Intent::OpenCharacter:
        case Intent::OpenPassive:
          // Hotkeys cannot lift focus away from a modal dialog or a text
          // field; the surface must be dismissed explicitly first.
          return out;
      }
      break;
  }

  // Unreachable: every enumerator of both switches is handled above, and
  // unknown values were rejected before the dispatch. Kept as a fail-closed
  // guard so even a future enum extension degrades to an explicit refusal.
  return refused(Status::InvalidState, in);
}

}  // namespace input_focus
