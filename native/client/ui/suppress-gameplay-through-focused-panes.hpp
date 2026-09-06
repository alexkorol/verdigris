#pragma once

// VG-MOVE-005: TASK-0165 input_focus is the production gate. A helper
// without ClientState ticks cannot prove the journey.

#include "../input_focus.hpp"

namespace verdigris::client::ui {

struct PaneFocusView {
  bool gear = false;
  bool character = false;
  bool tree = false;
  bool trade = false;
  bool drag = false;
  bool text = false;
};

inline input_focus::State stack_from(const PaneFocusView& view) {
  input_focus::State stack;
  const auto push = [&](input_focus::Surface surface) {
    const input_focus::Decision decision = input_focus::push_surface(stack, surface);
    if (decision.status == input_focus::Status::Ok) stack = decision.next;
  };
  // Escape order is tree, character, gear: gear sits at the bottom.
  if (view.gear) push(input_focus::Surface::Gear);
  if (view.character) push(input_focus::Surface::Character);
  if (view.tree) push(input_focus::Surface::Passive);
  if (view.trade || view.drag) push(input_focus::Surface::Modal);
  if (view.text) push(input_focus::Surface::Text);
  return stack;
}

inline bool passes_gameplay(const PaneFocusView& view, input_focus::Intent intent) {
  return input_focus::reduce(stack_from(view), intent).disposition ==
         input_focus::Disposition::PassToGameplay;
}

inline const char* focus_hud_label(const PaneFocusView& view) {
  switch (stack_from(view).focused()) {
    case input_focus::Surface::Gear:
      return "focus:gear";
    case input_focus::Surface::Character:
      return "focus:character";
    case input_focus::Surface::Passive:
      return "focus:passive";
    case input_focus::Surface::Modal:
      return "focus:modal";
    case input_focus::Surface::Text:
      return "focus:text";
    case input_focus::Surface::None:
      return "focus:none";
  }
  return "focus:unknown";
}

}  // namespace verdigris::client::ui
