#pragma once

// Constitution HUD: life is the left vessel, mana/resource the right.
// art.png stores the red life crop first and the blue mana crop second.
// Swapping those crops puts blue liquid on the life slot.

namespace verdigris::client::ui {

inline constexpr int kSheetLifeArtLeft = 20;
inline constexpr int kSheetManaArtLeft = 842;

struct ChannelMean {
  int r = 0;
  int b = 0;
  int samples = 0;
};

inline bool life_on_screen_left(int life_cx, int mana_cx) {
  return life_cx < mana_cx;
}

inline bool swapped_sheet_crops_fail(int life_art_left) {
  return life_art_left >= 400;
}

inline bool life_reads_red(const ChannelMean& mean) {
  return mean.samples > 8 && mean.r > mean.b + 8;
}

inline bool mana_reads_blue(const ChannelMean& mean) {
  return mean.samples > 8 && mean.b > mean.r + 8;
}

inline bool mute_on_mana_globe_fails(bool muted, bool glyph_at_resource_cx) {
  return muted && glyph_at_resource_cx;
}

inline const char* owner_life_left_label() { return "Life left"; }
inline const char* owner_mana_right_label() { return "Mana right"; }
inline bool controls_on_tree_fails_review(bool overlap) { return overlap; }
inline bool controls_on_character_fails_review(bool overlap) { return overlap; }
inline bool missing_controls_fails_review(bool present) { return !present; }

}  // namespace verdigris::client::ui
