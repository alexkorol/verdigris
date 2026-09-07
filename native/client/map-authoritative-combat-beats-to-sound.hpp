#pragma once

// VG-SOUND-003: presentation events map to one voiced cue per event ID.
// Replaying the same DamageApplied cannot double-play hit. Core event
// minting stays Kimi (VG-CORE-006).

namespace verdigris::client::audio_beats {

inline const char* owner_beats_label() { return "Beats mapped"; }
inline const char* owner_hit_once_label() { return "Hit once"; }
inline bool double_play_fails_review(int hits_for_one_event) {
  return hits_for_one_event != 1;
}

}  // namespace verdigris::client::audio_beats
