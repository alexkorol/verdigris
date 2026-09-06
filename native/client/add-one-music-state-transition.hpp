#pragma once

// VG-SOUND-008: exploration / combat / recovery themes. An unloaded or
// paused scene must mute the music bus so a prior combat loop cannot keep
// voicing. Core STORY phase authority stays with Kimi.

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

}  // namespace verdigris::client::music
