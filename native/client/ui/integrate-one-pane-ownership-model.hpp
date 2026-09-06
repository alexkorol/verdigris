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

}  // namespace verdigris::client::ui
