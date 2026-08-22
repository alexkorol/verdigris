// TASK-0157: backend-neutral procedural audio scheduler foundation.
// Content-neutral cue data only: no audio asset, backend/device API,
// dependency, or final frequency/music decision lives here. Every numeric
// parameter below is a provisional placeholder awaiting owner audio
// direction (SPEC owner_input_dependency).

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace verdigris::audio {

enum class Bus : int {
  Sfx,
  Music,
};

// Lower enum value = lower precedence. UI > player-feedback > world, per the
// accepted TASK-0117 mix architecture.
enum class PriorityClass : int {
  World = 0,
  PlayerFeedback = 1,
  Ui = 2,
};

enum class Waveform : int {
  Sine,
  Square,
  Sawtooth,
  Noise,
};

struct CueParams {
  Waveform waveform = Waveform::Sine;
  int start_hz = 0;
  int end_hz = 0;
  int duration_ms = 0;
  int gain_permille = 0;  // 0..1000
};

struct CueSpec {
  std::string cue_id;
  Bus bus = Bus::Sfx;
  PriorityClass priority = PriorityClass::PlayerFeedback;
  std::uint64_t scheduled_tick = 0;
  std::uint64_t sequence = 0;       // mixer-assigned arrival ordinal
  int effective_gain_permille = 0;  // authored gain scaled by bus volume
  CueParams params;
};

std::string bus_name(Bus bus);
std::string priority_name(PriorityClass priority);
std::string waveform_name(Waveform waveform);

// Canonical, deterministic text form. Pure integer fields only, so the bytes
// are reproducible across runs on the same build. Input order is preserved;
// callers wanting a canonical schedule sort first (the mixer does).
std::string serialize_schedule(const std::vector<CueSpec>& cues);

}  // namespace verdigris::audio
