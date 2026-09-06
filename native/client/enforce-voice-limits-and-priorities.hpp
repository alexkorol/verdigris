#pragma once

// VG-SOUND-004: SFX voice budget plus reserved warning priority. Cosmetic
// World cues cannot starve a PlayerFeedback scion-lost warning. Mixer
// steal policy stays in native/audio (TASK-0157); this is the owner HUD.
// VG-PERF-002 tick budget stays Kimi.

#include <cstdint>
#include <string>

#include "../audio/audio_mixer.hpp"

namespace verdigris::client::voices {

inline constexpr std::uint32_t kSfxVoiceBudget =
    verdigris::audio::AudioMixer::kDefaultSfxVoices;

inline const char* owner_warning_held_label() { return "Warning held"; }

inline std::string owner_budget_line() {
  return std::string("Voices ") + std::to_string(kSfxVoiceBudget);
}

inline bool cosmetics_starve_warning(bool heard_warning) {
  return !heard_warning;
}

inline bool over_budget_cosmetics_fail_review(int submitted, std::uint32_t cap) {
  return submitted > static_cast<int>(cap);
}

}  // namespace verdigris::client::voices
