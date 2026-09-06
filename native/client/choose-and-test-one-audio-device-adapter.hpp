#pragma once

// VG-SOUND-001: portable software tone adapter. A scheduled cue with no
// PCM (zero duration or zero gain) is not audible output. Unknown backends
// cannot pretend to be portable. Device mute still silences waveOut.

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "../audio/cue_spec.hpp"

namespace verdigris::audio {

enum class ToneBackend { Software };

struct ToneAdapter {
  static constexpr int kSampleRate = 22050;
  static constexpr const char* kBackendName = "software";

  bool alive = false;
  std::vector<std::int16_t> pcm;

  bool init(ToneBackend backend) {
    shutdown();
    if (backend != ToneBackend::Software) return false;
    alive = true;
    return true;
  }

  bool play_generated_tone(int hz, int duration_ms, int gain_permille) {
    pcm.clear();
    if (!alive || hz <= 0 || duration_ms <= 0 || gain_permille <= 0) return false;
    const int total = kSampleRate * duration_ms / 1000;
    if (total <= 0) return false;
    pcm.resize(static_cast<std::size_t>(total));
    const double amp = (gain_permille / 1000.0) * 0.35 * 32767.0;
    for (int i = 0; i < total; ++i) {
      const double phase =
          (static_cast<double>(i) * hz) / static_cast<double>(kSampleRate);
      const double env = 1.0 - static_cast<double>(i) / total;
      pcm[static_cast<std::size_t>(i)] =
          static_cast<std::int16_t>(std::sin(phase * 6.283185307179586) * env * amp);
    }
    return audible();
  }

  bool audible() const { return peak_abs() > 256; }

  int peak_abs() const {
    int peak = 0;
    for (std::int16_t s : pcm) {
      const int a = s < 0 ? -s : s;
      if (a > peak) peak = a;
    }
    return peak;
  }

  void shutdown() {
    pcm.clear();
    alive = false;
  }
};

inline bool cue_has_audible_output(const CueSpec& cue) {
  const int gain = cue.effective_gain_permille > 0 ? cue.effective_gain_permille
                                                   : cue.params.gain_permille;
  return cue.params.duration_ms > 0 && gain > 0 && cue.params.start_hz > 0;
}

inline const char* owner_adapter_label() { return "Adapter software"; }

inline std::string owner_tone_label(int hz) {
  return std::string("Tone ") + std::to_string(hz) + " Hz";
}

inline bool unknown_backend_fails_review(bool software_backend) {
  return !software_backend;
}

inline bool zero_duration_cue_fails_review(const CueSpec& cue) {
  return !cue_has_audible_output(cue);
}

}  // namespace verdigris::audio
