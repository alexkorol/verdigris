#pragma once

// VG-SOUND-006: master mute plus category volumes. Changing mute cannot
// reset SFX/music preferences. Zero-volume categories stay silent.

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#include "../audio/audio_mixer.hpp"

namespace verdigris::audio {

struct AudioPrefs {
  bool muted = false;
  int sfx_permille = 1000;
  int music_permille = 1000;
};

inline AudioPrefs apply_mute_only(AudioPrefs prefs, bool muted) {
  prefs.muted = muted;
  return prefs;
}

inline bool save_audio_prefs(const std::string& path, const AudioPrefs& prefs) {
  if (path.empty()) return false;
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "wb") != 0 || !file) return false;
#else
  file = std::fopen(path.c_str(), "wb");
  if (!file) return false;
#endif
  std::fprintf(file, "muted=%d\nsfx=%d\nmusic=%d\n", prefs.muted ? 1 : 0,
               prefs.sfx_permille, prefs.music_permille);
  std::fclose(file);
  return true;
}

inline AudioPrefs load_audio_prefs(const std::string& path) {
  AudioPrefs prefs;
  if (path.empty()) return prefs;
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "rb") != 0 || !file) return prefs;
#else
  file = std::fopen(path.c_str(), "rb");
  if (!file) return prefs;
#endif
  char buf[256];
  const std::size_t n = std::fread(buf, 1, sizeof(buf) - 1, file);
  std::fclose(file);
  buf[n] = '\0';
  const std::string text(buf);
  auto take = [&](const char* key, int fallback) {
    const std::string needle = std::string(key) + "=";
    const auto at = text.find(needle);
    if (at == std::string::npos) return fallback;
    return std::atoi(text.c_str() + at + needle.size());
  };
  prefs.muted = take("muted", 0) != 0;
  prefs.sfx_permille = take("sfx", 1000);
  prefs.music_permille = take("music", 1000);
  if (prefs.sfx_permille < 0) prefs.sfx_permille = 0;
  if (prefs.sfx_permille > 1000) prefs.sfx_permille = 1000;
  if (prefs.music_permille < 0) prefs.music_permille = 0;
  if (prefs.music_permille > 1000) prefs.music_permille = 1000;
  return prefs;
}

inline void apply_audio_prefs(AudioMixer& mixer, const AudioPrefs& prefs) {
  mixer.set_bus_volume(Bus::Sfx, prefs.muted ? 0 : prefs.sfx_permille);
  mixer.set_bus_volume(Bus::Music, prefs.muted ? 0 : prefs.music_permille);
  mixer.set_bus_muted(Bus::Sfx, false);
  mixer.set_bus_muted(Bus::Music, false);
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
