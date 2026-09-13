#pragma once

// VG-SOUND-006: master mute plus category volumes. Changing mute cannot
// reset SFX/music preferences. Zero-volume categories stay silent.

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#include "../audio/audio_mixer.hpp"
#include "user_settings.hpp"

namespace verdigris::audio {

inline void apply_audio_prefs(AudioMixer& mixer, const AudioPrefs& prefs) {
  mixer.set_bus_volume(Bus::Sfx, prefs.muted ? 0 : prefs.sfx_permille);
  mixer.set_bus_volume(Bus::Music, prefs.muted ? 0 : prefs.music_permille);
  // Scene-level bus gates (for example unloaded music) survive a volume edit.
}

inline const char* owner_mute_label(bool muted) { return muted ? "Muted" : "Audio on"; }

inline std::string owner_volume_line(const char* name, int permille) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%s %d", name, permille / 10);
  return buf;
}

inline bool mute_chip_alone_fails_prefs_review(bool mixer_painted) {
  return !mixer_painted;
}

inline const char* owner_mixer_prefs_label() { return "Mixer prefs"; }
inline const char* owner_sfx_persist_label() { return "SFX persist"; }

}  // namespace verdigris::audio
