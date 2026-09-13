#pragma once

// VG-UI-001: native pane stack. Escape dismisses the top dismissible pane;
// only a bare stack requests quit. A helper test without ClientState is not
// this integration.

namespace verdigris::client::ui {

inline int pane_stack_depth(bool tree, bool character, bool gear) {
  return (tree ? 1 : 0) + (character ? 1 : 0) + (gear ? 1 : 0);
}

inline bool helper_depth_alone_cannot_prove(int depth, bool native_painted,
                                            bool native_escape) {
  (void)depth;
  return !native_painted || !native_escape;
}

inline const char* owner_stack_label() { return "Stack 2"; }
inline const char* owner_escape_label() { return "Escape closes"; }
inline const char* owner_escape_wrap_a() { return "Escape"; }
inline const char* owner_escape_wrap_b() { return "closes"; }
inline bool diptych_strip_covers_panes_fails_review(bool overlap) {
  return overlap;
}

}  // namespace verdigris::client::ui
