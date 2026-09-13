// owner_menu_input.hpp — TASK-0183 prep: compose menu_scene + input_focus.
//
// Header-only bridge for the integrator lane. While playing, pane focus
// (gear/inventory) consumes Esc before the pause menu opens; bare Esc never
// requests quit — it opens pause via menu_scene. Title/pause roots are owned
// entirely by menu_scene. No main.cpp, Win32, or heap in this packet.
#pragma once

#include "input_focus.hpp"
#include "menu_scene.hpp"

namespace owner_menu_input {

struct State {
  menu_scene::State menu{};
  input_focus::State focus{};

  [[nodiscard]] constexpr bool valid() const {
    if (!menu.valid() || !focus.valid()) return false;
    if (menu.root == menu_scene::Root::Playing) return true;
    return focus.depth == 0;
  }

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

struct Decision {
  menu_scene::Status menu_status = menu_scene::Status::Ok;
  input_focus::Status focus_status = input_focus::Status::Ok;
  menu_scene::Disposition disposition = menu_scene::Disposition::Consumed;
  State next{};

  [[nodiscard]] constexpr bool operator==(const Decision&) const = default;
};

[[nodiscard]] constexpr input_focus::Intent map_menu_intent(
    menu_scene::Intent intent) {
  switch (intent) {
    case menu_scene::Intent::Move:
      return input_focus::Intent::Move;
    case menu_scene::Intent::Attack:
      return input_focus::Intent::Attack;
    case menu_scene::Intent::Interact:
      return input_focus::Intent::Interact;
    case menu_scene::Intent::NavigatePrevious:
      return input_focus::Intent::NavigatePrevious;
    case menu_scene::Intent::NavigateNext:
      return input_focus::Intent::NavigateNext;
    case menu_scene::Intent::Confirm:
      return input_focus::Intent::Confirm;
    case menu_scene::Intent::Cancel:
      return input_focus::Intent::Cancel;
    case menu_scene::Intent::Escape:
      return input_focus::Intent::Escape;
    case menu_scene::Intent::Start:
    case menu_scene::Intent::Resume:
    case menu_scene::Intent::Quit:
    case menu_scene::Intent::OpenSettings:
    case menu_scene::Intent::OpenHelp:
      return input_focus::Intent::Move;
  }
  return input_focus::Intent::Move;
}

[[nodiscard]] constexpr Decision refused_menu(menu_scene::Status status,
                                              const State& in) {
  Decision out;
  out.menu_status = status;
  out.focus_status = input_focus::Status::Ok;
  out.disposition = menu_scene::Disposition::Consumed;
  out.next = in;
  return out;
}

[[nodiscard]] constexpr Decision from_menu(const State& in,
                                           const menu_scene::Decision& md) {
  Decision out;
  out.menu_status = md.status;
  out.focus_status = input_focus::Status::Ok;
  out.disposition = md.disposition;
  out.next = in;
  out.next.menu = md.next;
  return out;
}

[[nodiscard]] constexpr Decision from_focus(const State& in,
                                            const input_focus::Decision& fd) {
  Decision out;
  out.menu_status = menu_scene::Status::Ok;
  out.focus_status = fd.status;
  out.disposition =
      fd.disposition == input_focus::Disposition::PassToGameplay
          ? menu_scene::Disposition::PassToGameplay
          : fd.disposition == input_focus::Disposition::RequestQuit
                ? menu_scene::Disposition::RequestQuit
                : menu_scene::Disposition::Consumed;
  out.next = in;
  out.next.focus = fd.next;
  return out;
}

[[nodiscard]] constexpr Decision reduce(const State& in,
                                        menu_scene::Intent intent) {
  if (!in.valid()) return refused_menu(menu_scene::Status::InvalidState, in);
  if (static_cast<std::uint8_t>(intent) >= menu_scene::kIntentCount) {
    return refused_menu(menu_scene::Status::InvalidState, in);
  }

  if (in.menu.root != menu_scene::Root::Playing) {
    return from_menu(in, menu_scene::reduce(in.menu, intent));
  }

  switch (intent) {
    case menu_scene::Intent::Escape:
    case menu_scene::Intent::Cancel:
      if (in.focus.depth > 0) {
        return from_focus(
            in, input_focus::reduce(in.focus, input_focus::Intent::Escape));
      }
      return from_menu(in, menu_scene::reduce(in.menu, intent));

    case menu_scene::Intent::Move:
    case menu_scene::Intent::Attack:
    case menu_scene::Intent::Interact:
      if (in.focus.depth > 0) {
        const input_focus::Intent fi = map_menu_intent(intent);
        return from_focus(in, input_focus::reduce(in.focus, fi));
      }
      return from_menu(in, menu_scene::reduce(in.menu, intent));

    case menu_scene::Intent::NavigatePrevious:
    case menu_scene::Intent::NavigateNext:
    case menu_scene::Intent::Confirm:
      if (in.focus.depth > 0) {
        const input_focus::Intent fi = map_menu_intent(intent);
        return from_focus(in, input_focus::reduce(in.focus, fi));
      }
      return from_menu(in, menu_scene::reduce(in.menu, intent));

    case menu_scene::Intent::Start:
    case menu_scene::Intent::Resume:
    case menu_scene::Intent::Quit:
    case menu_scene::Intent::OpenSettings:
    case menu_scene::Intent::OpenHelp:
      return from_menu(in, menu_scene::reduce(in.menu, intent));
  }

  return refused_menu(menu_scene::Status::InvalidState, in);
}

[[nodiscard]] constexpr Decision reduce_pane(const State& in,
                                           input_focus::Intent intent) {
  if (!in.valid()) {
    Decision out;
    out.menu_status = menu_scene::Status::InvalidState;
    out.focus_status = input_focus::Status::InvalidState;
    out.next = in;
    return out;
  }
  if (in.menu.root != menu_scene::Root::Playing) {
    return refused_menu(menu_scene::Status::InvalidState, in);
  }
  if (static_cast<std::uint8_t>(intent) >= input_focus::kIntentCount) {
    Decision out;
    out.focus_status = input_focus::Status::InvalidState;
    out.next = in;
    return out;
  }
  return from_focus(in, input_focus::reduce(in.focus, intent));
}

}  // namespace owner_menu_input
