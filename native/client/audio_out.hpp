#pragma once

// waveOut backend for the TASK-0157 audio mixer: the first real device
// behind the Sink seam. Each scheduled cue is synthesized to a small PCM
// buffer (waveform + linear frequency sweep + attack/decay envelope) and
// written to one of a small pool of waveOut handles; Windows' kernel mixer
// blends overlapping cues, so no software mixing is needed here. All calls
// run on the UI thread at the fixed tick; synthesis of a sub-second mono
// cue is microseconds of work.

#ifdef _WIN32

#include <cmath>
#include <cstdint>
#include <vector>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "../audio/audio_mixer.hpp"

namespace verdigris::audio {

class WaveOutSink final : public Sink {
 public:
  static constexpr int kSampleRate = 44100;
  static constexpr int kHandlePool = 6;
  static constexpr int kMaxCueMs = 2000;

  WaveOutSink() = default;
  WaveOutSink(const WaveOutSink&) = delete;
  WaveOutSink& operator=(const WaveOutSink&) = delete;

  ~WaveOutSink() {
    for (auto& lane : lanes_) {
      if (!lane.handle) continue;
      waveOutReset(lane.handle);
      reclaim(lane, true);
      waveOutClose(lane.handle);
    }
  }

  void set_muted(bool muted) { muted_ = muted; }
  bool muted() const { return muted_; }
  bool device_ok() const { return device_ok_; }

  void schedule(const CueSpec& cue) override {
    if (muted_) return;
    if (cue.effective_gain_permille <= 0 || cue.params.duration_ms <= 0) return;
    Lane* lane = next_lane();
    if (!lane) return;
    play(*lane, cue);
  }

 private:
  struct Voice {
    WAVEHDR header{};
    std::vector<std::int16_t> samples;
  };

  struct Lane {
    HWAVEOUT handle = nullptr;
    std::vector<Voice*> voices;
  };

  bool muted_ = false;
  bool device_ok_ = true;
  bool opened_any_ = false;
  int next_ = 0;
  Lane lanes_[kHandlePool];

  Lane* next_lane() {
    if (!device_ok_) return nullptr;
    for (int attempt = 0; attempt < kHandlePool; ++attempt) {
      Lane& lane = lanes_[next_];
      next_ = (next_ + 1) % kHandlePool;
      if (!lane.handle) {
        WAVEFORMATEX format{};
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 1;
        format.nSamplesPerSec = kSampleRate;
        format.wBitsPerSample = 16;
        format.nBlockAlign = 2;
        format.nAvgBytesPerSec = kSampleRate * 2;
        if (waveOutOpen(&lane.handle, WAVE_MAPPER, &format, 0, 0,
                        CALLBACK_NULL) != MMSYSERR_NOERROR) {
          lane.handle = nullptr;
          // No usable device (headless CI, remote desktop): stay silent
          // forever rather than erroring per cue.
          if (!opened_any_) device_ok_ = false;
          continue;
        }
        opened_any_ = true;
      }
      reclaim(lane, false);
      return &lane;
    }
    return nullptr;
  }

  void reclaim(Lane& lane, bool force) {
    for (std::size_t i = 0; i < lane.voices.size();) {
      Voice* voice = lane.voices[i];
      if (force || (voice->header.dwFlags & WHDR_DONE)) {
        waveOutUnprepareHeader(lane.handle, &voice->header, sizeof(WAVEHDR));
        delete voice;
        lane.voices.erase(lane.voices.begin() + static_cast<long long>(i));
      } else {
        ++i;
      }
    }
  }

  void play(Lane& lane, const CueSpec& cue) {
    const int clamped_ms =
        cue.params.duration_ms > kMaxCueMs ? kMaxCueMs : cue.params.duration_ms;
    const int total = kSampleRate * clamped_ms / 1000;
    if (total <= 0) return;
    auto* voice = new Voice();
    voice->samples.resize(static_cast<std::size_t>(total));
    // Master headroom keeps six overlapping synth cues out of clipping.
    const double gain =
        (cue.effective_gain_permille / 1000.0) * 0.30 * 32767.0;
    const double start_hz = cue.params.start_hz;
    const double end_hz = cue.params.end_hz;
    double phase = 0.0;
    std::uint32_t noise = 0x9E3779B9u ^ static_cast<std::uint32_t>(cue.sequence);
    const int attack = kSampleRate * 5 / 1000;  // 5 ms
    for (int i = 0; i < total; ++i) {
      const double t = static_cast<double>(i) / total;
      const double hz = start_hz + (end_hz - start_hz) * t;
      phase += hz / kSampleRate;
      if (phase >= 1.0) phase -= std::floor(phase);
      double value = 0.0;
      switch (cue.params.waveform) {
        case Waveform::Sine:
          value = std::sin(phase * 6.283185307179586);
          break;
        case Waveform::Square:
          value = phase < 0.5 ? 0.6 : -0.6;  // softened square
          break;
        case Waveform::Sawtooth:
          value = (phase * 2.0 - 1.0) * 0.7;
          break;
        case Waveform::Noise:
          noise = noise * 1664525u + 1013904223u;
          value = (static_cast<double>(noise >> 8) / 8388607.5) - 1.0;
          break;
      }
      double envelope = 1.0 - t;  // linear decay
      if (i < attack)
        envelope *= static_cast<double>(i) / (attack > 0 ? attack : 1);
      voice->samples[static_cast<std::size_t>(i)] =
          static_cast<std::int16_t>(value * envelope * gain);
    }
    voice->header.lpData = reinterpret_cast<LPSTR>(voice->samples.data());
    voice->header.dwBufferLength =
        static_cast<DWORD>(voice->samples.size() * sizeof(std::int16_t));
    if (waveOutPrepareHeader(lane.handle, &voice->header, sizeof(WAVEHDR)) !=
            MMSYSERR_NOERROR ||
        waveOutWrite(lane.handle, &voice->header, sizeof(WAVEHDR)) !=
            MMSYSERR_NOERROR) {
      waveOutUnprepareHeader(lane.handle, &voice->header, sizeof(WAVEHDR));
      delete voice;
      return;
    }
    lane.voices.push_back(voice);
  }
};

class TeeSink final : public Sink {
 public:
  TeeSink(Sink& live, RecordingSink& tape) : live_(live), tape_(tape) {}
  void schedule(const CueSpec& cue) override {
    tape_.schedule(cue);
    live_.schedule(cue);
  }

 private:
  Sink& live_;
  RecordingSink& tape_;
};

}  // namespace verdigris::audio

#endif  // _WIN32
