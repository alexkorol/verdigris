// TASK-0157: deterministic, headless audio mixer. Schedules CueSpec data
// against an injectable Sink; applies bus mute/volume, priority classes, and
// bounded per-bus voice caps with steal-oldest eviction. This packet
// schedules data only — it never opens a device, plays audio, or links an
// audio backend.

#pragma once

#include <cstdint>
#include <vector>

#include "cue_spec.hpp"
#include "presentation_events.hpp"

namespace verdigris::audio {

// Backend seam: the only way scheduled cues leave the mixer. Production
// backends and the headless test recorder both implement this.
class Sink {
 public:
  virtual ~Sink() = default;
  virtual void schedule(const CueSpec& cue) = 0;
};

// Headless recording sink: captures every scheduled cue in arrival order.
class RecordingSink final : public Sink {
 public:
  void schedule(const CueSpec& cue) override;
  const std::vector<CueSpec>& cues() const { return cues_; }
  void clear() { cues_.clear(); }

 private:
  std::vector<CueSpec> cues_;
};

class AudioMixer {
 public:
  static constexpr std::uint32_t kDefaultSfxVoices = 8;
  static constexpr std::uint32_t kDefaultMusicVoices = 2;

  explicit AudioMixer(Sink& sink,
                      std::uint32_t sfx_voice_cap = kDefaultSfxVoices,
                      std::uint32_t music_voice_cap = kDefaultMusicVoices);

  // Translates one presentation event into a pending cue. Events unknown to
  // the cue table are dropped silently. Returns true when a cue was queued.
  bool ingest(const verdigris::client::PresentationEvent& event,
              std::uint64_t tick);

  // Direct cue submission for callers that already hold a CueSpec (future
  // UI/music systems); used by tests to exercise non-SFX buses.
  void submit(CueSpec spec);

  // Bus state. Volumes are permille (clamped to 0..1000); a muted bus or a
  // zero effective gain silences its cues at drain time.
  void set_bus_volume(Bus bus, int volume_permille);
  void set_bus_muted(Bus bus, bool muted);
  int bus_volume(Bus bus) const;
  bool bus_muted(Bus bus) const;

  // Applies gating, voice caps, and steal-oldest eviction, then hands the
  // surviving cues to the sink in deterministic (tick, sequence) order.
  // Returns the voiced cues; pending state is cleared.
  std::vector<CueSpec> drain_scheduled();

  const std::vector<CueSpec>& pending() const { return pending_; }
  std::uint64_t last_sequence() const { return next_sequence_ - 1; }

 private:
  struct BusState {
    int volume_permille = 1000;
    bool muted = false;
    std::uint32_t voice_cap = 0;
  };

  BusState& bus_state(Bus bus);
  const BusState& bus_state(Bus bus) const;

  Sink& sink_;
  std::vector<CueSpec> pending_;
  BusState buses_[2];
  std::uint64_t next_sequence_ = 1;
};

}  // namespace verdigris::audio
