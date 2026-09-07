#pragma once

// VG-SOUND-008: exploration / combat / recovery themes. An unloaded or
// paused scene must mute the music bus so a prior combat loop cannot keep
// voicing. Core STORY phase authority stays with Kimi.

#include <cstring>
#include <string>

#include "presentation_state.hpp"

namespace verdigris::client::music {

inline const char* theme_for(ExpeditionPhaseView phase, bool scene_loaded,
                             bool living_foes) {
  if (!scene_loaded) return "music:none";
  if (phase == ExpeditionPhaseView::ExtractCarriedValue) return "music:recovery";
  if (phase == ExpeditionPhaseView::SlayWardens && living_foes) return "music:combat";
  return "music:explore";
}

inline bool mute_music_bus(const char* want) {
  return want == nullptr || std::string(want) == "music:none";
}

inline const char* owner_theme_label(const std::string& want) {
  if (want == "music:combat") return "Theme Combat";
  if (want == "music:recovery") return "Theme Recovery";
  if (want == "music:explore") return "Theme Explore";
  return "Theme None";
}

inline const char* owner_music_none_label() { return "Music none"; }
inline bool music_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}

inline bool leftover_theme_fails_review(bool scene_loaded,
                                        const std::string& sent) {
  return !scene_loaded && sent != "music:none" && !sent.empty();
}

}  // namespace verdigris::client::music
