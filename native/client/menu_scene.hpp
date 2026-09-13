// menu_scene.hpp — TASK-0170 pure title/pause/menu scene reducer.
//
// Header-only, dependency-free model for the Owner Demo splash/menu flow.
// Given the current scene (title, playing, paused) and one abstract input
// intent, reduce() returns the next scene plus an explicit disposition:
// Consumed, PassToGameplay, or RequestQuit. Escape never directly exits the
// application: during play it opens pause, atop pause it closes the top
// nested surface or resumes, and on the title screen it is swallowed. Only an
// explicit Quit menu command (after the confirm-quit surface when required)
// yields RequestQuit. Fixed-capacity pause nesting, no heap, no clocks, no
// Win32/GDI, no key codes, and zero integration with main.cpp in this packet.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace menu_scene {

// Top-level owner-facing scene. Expedition gameplay maps to Playing.
enum class Root : std::uint8_t {
  Title = 0,
  Playing,
  Paused,
};

// Nested pause-menu surfaces. None is canonical tail padding only.
enum class PausePane : std::uint8_t {
  None = 0,
  Main,         // root pause list (resume / settings / quit)
  Settings,
  Help,
  ConfirmQuit,  // explicit quit confirmation dialog
};

// Abstract input intent. Key-agnostic: callers translate hardware into intents.
enum class Intent : std::uint8_t {
  Move,
  Attack,
  Interact,
  NavigatePrevious,
  NavigateNext,
  Confirm,
  Cancel,
  Escape,
  Start,          // begin play from title / affirm a menu row
  Resume,         // explicit resume command from pause root
  Quit,           // explicit quit command from pause menu
  OpenSettings,
  OpenHelp,
};

inline constexpr std::uint8_t kIntentCount =
    static_cast<std::uint8_t>(Intent::OpenHelp) + 1;

enum class Disposition : std::uint8_t {
  Consumed,
  PassToGameplay,
  RequestQuit,
};

enum class Status : std::uint8_t {
  Ok,
  InvalidState,
  StackFull,
};

inline constexpr std::size_t kMaxPausePanes = 4;

struct State {
  Root root = Root::Title;
  std::uint8_t pause_depth = 0;
  std::array<PausePane, kMaxPausePanes> pause_stack{};

  [[nodiscard]] constexpr PausePane focused_pause() const {
    return pause_depth == 0 ? PausePane::None
                            : pause_stack[static_cast<std::size_t>(pause_depth) - 1];
  }

  [[nodiscard]] constexpr bool valid() const {
    if (root == Root::Paused) {
      if (pause_depth == 0 || pause_depth > kMaxPausePanes) return false;
    } else {
      if (pause_depth != 0) return false;
    }
    for (std::size_t i = 0; i < static_cast<std::size_t>(pause_depth); ++i) {
      const auto raw = static_cast<std::uint8_t>(pause_stack[i]);
      if (raw == static_cast<std::uint8_t>(PausePane::None) ||
          raw > static_cast<std::uint8_t>(PausePane::ConfirmQuit)) {
        return false;
      }
    }
    for (std::size_t i = static_cast<std::size_t>(pause_depth);
         i < kMaxPausePanes; ++i) {
      if (pause_stack[i] != PausePane::None) return false;
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

[[nodiscard]] constexpr const char* name(Root root) {
  switch (root) {
    case Root::Title:
      return "title";
    case Root::Playing:
      return "playing";
    case Root::Paused:
      return "paused";
  }
  return "unknown-root";
}

[[nodiscard]] constexpr const char* name(PausePane pane) {
  switch (pane) {
    case PausePane::None:
      return "none";
    case PausePane::Main:
      return "main";
    case PausePane::Settings:
      return "settings";
    case PausePane::Help:
      return "help";
    case PausePane::ConfirmQuit:
      return "confirm-quit";
  }
  return "unknown-pane";
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
    case Intent::Start:
      return "start";
    case Intent::Resume:
      return "resume";
    case Intent::Quit:
      return "quit";
    case Intent::OpenSettings:
      return "open-settings";
    case Intent::OpenHelp:
      return "open-help";
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

[[nodiscard]] constexpr Decision refused(Status status, const State& in) {
  Decision out;
  out.status = status;
  out.disposition = Disposition::Consumed;
  out.next = in;
  return out;
}

[[nodiscard]] constexpr State playing_state() {
  State s;
  s.root = Root::Playing;
  return s;
}

[[nodiscard]] constexpr State paused_with(PausePane top) {
  State s;
  s.root = Root::Paused;
  s.pause_depth = 1;
  s.pause_stack[0] = top;
  return s;
}

[[nodiscard]] constexpr Decision push_pause_pane(const State& in,
                                                 PausePane pane) {
  if (!in.valid() || in.root != Root::Paused) {
    return refused(Status::InvalidState, in);
  }
  const auto raw = static_cast<std::uint8_t>(pane);
  if (raw == static_cast<std::uint8_t>(PausePane::None) ||
      raw > static_cast<std::uint8_t>(PausePane::ConfirmQuit)) {
    return refused(Status::InvalidState, in);
  }
  if (in.pause_depth >= kMaxPausePanes) {
    return refused(Status::StackFull, in);
  }
  Decision out;
  out.next = in;
  out.next.pause_stack[out.next.pause_depth] = pane;
  ++out.next.pause_depth;
  return out;
}

[[nodiscard]] constexpr Decision pop_pause_pane(const State& in) {
  if (!in.valid() || in.root != Root::Paused || in.pause_depth == 0) {
    return refused(Status::InvalidState, in);
  }
  Decision out;
  out.next = in;
  out.next.pause_stack[static_cast<std::size_t>(out.next.pause_depth) - 1] =
      PausePane::None;
  --out.next.pause_depth;
  if (out.next.pause_depth == 0) {
    out.next.root = Root::Playing;
  }
  return out;
}

[[nodiscard]] constexpr Decision open_pause(const State& in) {
  if (!in.valid() || in.root != Root::Playing) {
    return refused(Status::InvalidState, in);
  }
  Decision out;
  out.next.root = Root::Paused;
  out.next.pause_depth = 1;
  out.next.pause_stack[0] = PausePane::Main;
  return out;
}

[[nodiscard]] constexpr Decision reduce(const State& in, Intent intent) {
  if (!in.valid()) return refused(Status::InvalidState, in);
  if (static_cast<std::uint8_t>(intent) >= kIntentCount) {
    return refused(Status::InvalidState, in);
  }

  Decision out;
  out.next = in;

  switch (in.root) {
    case Root::Title:
      switch (intent) {
        case Intent::Escape:
        case Intent::Cancel:
          // Title screen: Esc never exits; owner must choose Start or Quit.
          return out;
        case Intent::Start:
        case Intent::Confirm:
          out.next = playing_state();
          return out;
        case Intent::Quit:
          out.disposition = Disposition::RequestQuit;
          return out;
        case Intent::Resume:
        case Intent::OpenSettings:
        case Intent::OpenHelp:
          return out;
        case Intent::Move:
        case Intent::Attack:
        case Intent::Interact:
        case Intent::NavigatePrevious:
        case Intent::NavigateNext:
          return out;
        default:
          break;
      }
      break;

    case Root::Playing:
      switch (intent) {
        case Intent::Escape:
        case Intent::Cancel:
          return open_pause(in);
        case Intent::Move:
        case Intent::Attack:
        case Intent::Interact:
          out.disposition = Disposition::PassToGameplay;
          return out;
        case Intent::Start:
        case Intent::Resume:
        case Intent::Quit:
        case Intent::OpenSettings:
        case Intent::OpenHelp:
        case Intent::NavigatePrevious:
        case Intent::NavigateNext:
        case Intent::Confirm:
          return out;
      }
      break;

    case Root::Paused:
      switch (intent) {
        case Intent::Escape:
        case Intent::Cancel:
          return pop_pause_pane(in);
        case Intent::Resume:
          out.next = playing_state();
          return out;
        case Intent::Quit:
          if (in.focused_pause() == PausePane::ConfirmQuit) {
            out.disposition = Disposition::RequestQuit;
            return out;
          }
          if (in.focused_pause() == PausePane::Main) {
            return push_pause_pane(in, PausePane::ConfirmQuit);
          }
          return out;
        case Intent::Confirm:
          if (in.focused_pause() == PausePane::ConfirmQuit) {
            out.disposition = Disposition::RequestQuit;
            return out;
          }
          return out;
        case Intent::OpenSettings:
          if (in.focused_pause() == PausePane::Main) {
            return push_pause_pane(in, PausePane::Settings);
          }
          return out;
        case Intent::OpenHelp:
          if (in.focused_pause() == PausePane::Main) {
            return push_pause_pane(in, PausePane::Help);
          }
          return out;
        case Intent::Start:
          if (in.focused_pause() == PausePane::Main) {
            out.next = playing_state();
            return out;
          }
          return out;
        case Intent::Move:
        case Intent::Attack:
        case Intent::Interact:
        case Intent::NavigatePrevious:
        case Intent::NavigateNext:
          return out;
      }
      break;
  }

  return refused(Status::InvalidState, in);
}

}  // namespace menu_scene
