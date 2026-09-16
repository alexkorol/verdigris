#pragma once

// TASK-0157: backend-neutral procedural audio scheduler foundation.
//
// This header carries pure scheduling data only. It must never include,
// link, or call into device APIs, assets, third-party dependencies, the
// simulation core, or any transport. Absolute frequencies are deliberately
// absent: pitch fields are ratios relative to a runtime reference pitch
// that a future device backend resolves. Choosing that reference, the
// final sound design, and any music direction remain owner-only decisions.

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace verdigris::audio {

// Buses are fixed by contract so serialized schedules stay stable.
enum class Bus : uint8_t {
  Music = 0,
  Sfx = 1,
};

// Priority classes: UI > player feedback > world (TASK-0117 finding 6.4).
enum class Priority : uint8_t {
  World = 0,
  Feedback = 1,
  Ui = 2,
};

// Content-neutral procedural waveform families. A waveform family names a
// synthesis shape; it pins no frequency, no authored sound, and no license.
enum class Waveform : uint8_t {
  Tone = 0,
  Noise = 1,
};

// Semantic trigger taxonomy for the representative TASK-0157 event set.
// Unknown is the explicit silence path: unmapped presentation events never
// become cues.
enum class CueTrigger : uint8_t {
  OrdinaryHit = 0,
  CriticalHit = 1,
  EnemyDefeat = 2,
  ScionLoss = 3,
  WarCryExpire = 4,
  Unknown = 5,
};

inline constexpr size_t kCueLabelBytes = 16;
using CueLabel = std::array<char, kCueLabelBytes>;

// Fixed-point convention: gains are milli-units of full scale (1000 = full),
// pitch ratios are permilli-units of the runtime reference pitch
// (1000 = reference). Integer fields keep serialization byte-stable on
// every host without floating-point representation questions.
struct CueSpec {
  CueLabel label{};
  Waveform waveform = Waveform::Tone;
  uint32_t duration_ms = 0;
  uint32_t attack_ms = 0;
  uint32_t release_ms = 0;
  uint32_t gain_milli = 500;
  int32_t pitch_start_permil = 1000;
  int32_t pitch_end_permil = 1000;

  bool operator==(const CueSpec&) const = default;
};

// One admitted voice. `sequence` is a mixer-assigned monotonic admission
// counter starting at 1; `frame` is the caller-supplied presentation tick.
// `effective_gain_milli` snapshots spec.gain_milli scaled by the bus volume
// at admission time; later volume changes never rewrite scheduled voices.
struct ScheduledCue {
  uint64_t sequence = 0;
  uint64_t frame = 0;
  Bus bus = Bus::Sfx;
  Priority priority = Priority::World;
  uint32_t effective_gain_milli = 0;
  CueSpec spec{};

  bool operator==(const ScheduledCue&) const = default;
};

// Canonical schedule order: frame ascending, then priority descending
// (UI first), then admission sequence ascending. Every serialized schedule
// and every ordered view passes through this comparator.
bool canonical_order(const ScheduledCue& lhs, const ScheduledCue& rhs);

// Byte-stable little-endian serialization. The input is copied and sorted
// canonically first, so equal logical schedules serialize identically
// regardless of insertion order. Layout: magic "VGA1", u32 cue count, then
// per cue: sequence u64, frame u64, bus u8, priority u8, effective gain u32,
// label 16 bytes, waveform u8, duration u32, attack u32, release u32,
// gain u32, pitch start i32, pitch end i32.
std::vector<uint8_t> serialize_schedule(const std::vector<ScheduledCue>& cues);

// Stable 64-bit FNV-1a digest hex string over the serialized schedule, for
// transcript-level comparison across process runs.
std::string schedule_digest_hex(const std::vector<ScheduledCue>& cues);

}  // namespace verdigris::audio
