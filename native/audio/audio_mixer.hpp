#pragma once

// TASK-0157: backend-neutral audio mixer seam. The mixer schedules data
// only; audible playback is a future device-backend concern and is never
// claimed here. The injectable Sink is the only output contract: the test
// recorder implements it to capture schedules, a future device backend
// implements it against an owner-selected audio device (backend choice is
// owner-only; none is selected by this packet).

#include "audio_cues.hpp"

#include <cstdint>
#include <vector>

namespace verdigris::audio {

// Named default table. Voice caps are deliberately small so elite-pack
// fights cannot fan out unbounded voices (TASK-0117 finding 3).
namespace mixer_defaults {
inline constexpr uint32_t kSfxVoiceCap = 8;
inline constexpr uint32_t kMusicVoiceCap = 2;
inline constexpr uint32_t kMinVoiceCap = 1;
inline constexpr uint32_t kVolumeMaxMilli = 1000;
inline constexpr uint32_t kDefaultVolumeMilli = 1000;
}  // namespace mixer_defaults

enum class Admission : uint8_t {
  Admitted = 0,
  RejectedMutedBus = 1,
  RejectedVoiceCap = 2,
};

struct SubmitResult {
  Admission status = Admission::RejectedVoiceCap;
  uint64_t sequence = 0;
  uint64_t evicted_sequence = 0;  // nonzero exactly when a steal occurred.
};

// Backend-neutral output seam.
class Sink {
 public:
  virtual ~Sink() = default;
  virtual void schedule(const ScheduledCue& cue) = 0;
  virtual void retire(uint64_t sequence, uint64_t stolen_by_sequence) = 0;
};

// Injectable recording sink: captures every admission and retirement in
// arrival order without any device, asset, or dependency.
class RecordingSink final : public Sink {
 public:
  void schedule(const ScheduledCue& cue) override { admitted_.push_back(cue); }
  void retire(uint64_t sequence, uint64_t stolen_by_sequence) override {
    retired_.push_back({sequence, stolen_by_sequence});
  }

  const std::vector<ScheduledCue>& admitted() const { return admitted_; }
  const std::vector<std::pair<uint64_t, uint64_t>>& retired() const {
    return retired_;
  }
  size_t retire_count() const { return retired_.size(); }
  void clear() {
    admitted_.clear();
    retired_.clear();
  }

 private:
  std::vector<ScheduledCue> admitted_;
  std::vector<std::pair<uint64_t, uint64_t>> retired_;
};

// Deterministic scheduler. Two buses carry independent volume/mute state,
// voice caps, and active-voice sets. Admission snapshots bus volume into
// the scheduled cue; mute gates the mixer outright. When a bus is at cap
// the incoming cue steals the victim with the lowest priority (ties broken
// oldest-first) if the incoming priority is greater than or equal to the
// victim's; a strictly lower-priority incoming cue is dropped instead.
class AudioMixer {
 public:
  explicit AudioMixer(Sink& sink);

  void set_bus_volume(Bus bus, uint32_t milli);
  void set_bus_muted(Bus bus, bool muted);
  void set_voice_cap(Bus bus, uint32_t cap);

  uint32_t bus_volume(Bus bus) const;
  bool bus_muted(Bus bus) const;
  uint32_t voice_cap(Bus bus) const;

  SubmitResult submit(Bus bus, Priority priority, const CueSpec& spec,
                      uint64_t frame);

  // Active voices for the bus in canonical order.
  std::vector<ScheduledCue> active_voices(Bus bus) const;

 private:
  struct RetiredRecord {
    uint64_t sequence;
    uint64_t stolen_by_sequence;
  };

  Sink& sink_;
  std::vector<ScheduledCue> voices_[2];
  uint32_t volumes_[2] = {mixer_defaults::kDefaultVolumeMilli,
                          mixer_defaults::kDefaultVolumeMilli};
  bool muted_[2] = {false, false};
  uint32_t caps_[2] = {mixer_defaults::kMusicVoiceCap,
                       mixer_defaults::kSfxVoiceCap};
  uint64_t next_sequence_ = 1;
};

}  // namespace verdigris::audio
