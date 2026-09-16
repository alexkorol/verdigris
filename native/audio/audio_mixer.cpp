#include "audio_mixer.hpp"

#include <algorithm>

namespace verdigris::audio {

namespace {

constexpr size_t bus_index(Bus bus) { return static_cast<size_t>(bus); }

}  // namespace

AudioMixer::AudioMixer(Sink& sink) : sink_(sink) {}

void AudioMixer::set_bus_volume(Bus bus, uint32_t milli) {
  volumes_[bus_index(bus)] =
      std::min(milli, mixer_defaults::kVolumeMaxMilli);
}

void AudioMixer::set_bus_muted(Bus bus, bool muted) {
  muted_[bus_index(bus)] = muted;
}

void AudioMixer::set_voice_cap(Bus bus, uint32_t cap) {
  caps_[bus_index(bus)] = std::max(cap, mixer_defaults::kMinVoiceCap);
}

uint32_t AudioMixer::bus_volume(Bus bus) const {
  return volumes_[bus_index(bus)];
}

bool AudioMixer::bus_muted(Bus bus) const { return muted_[bus_index(bus)]; }

uint32_t AudioMixer::voice_cap(Bus bus) const { return caps_[bus_index(bus)]; }

SubmitResult AudioMixer::submit(Bus bus, Priority priority, const CueSpec& spec,
                                uint64_t frame) {
  SubmitResult result;
  if (muted_[bus_index(bus)]) {
    result.status = Admission::RejectedMutedBus;
    return result;
  }

  std::vector<ScheduledCue>& active = voices_[bus_index(bus)];
  const uint32_t cap = caps_[bus_index(bus)];
  if (active.size() >= cap) {
    // Steal victim: lowest priority, ties broken oldest (lowest sequence).
    auto victim = std::min_element(
        active.begin(), active.end(), [](const ScheduledCue& lhs,
                                          const ScheduledCue& rhs) {
          if (lhs.priority != rhs.priority)
            return static_cast<uint8_t>(lhs.priority) <
                   static_cast<uint8_t>(rhs.priority);
          return lhs.sequence < rhs.sequence;
        });
    const bool may_steal =
        static_cast<uint8_t>(priority) >= static_cast<uint8_t>(victim->priority);
    if (!may_steal) {
      result.status = Admission::RejectedVoiceCap;
      return result;
    }
    result.evicted_sequence = victim->sequence;
    sink_.retire(victim->sequence, next_sequence_);
    active.erase(victim);
  }

  ScheduledCue cue;
  cue.sequence = next_sequence_++;
  cue.frame = frame;
  cue.bus = bus;
  cue.priority = priority;
  // Integer snapshot: spec gain scaled by the bus volume at admission.
  cue.effective_gain_milli =
      static_cast<uint32_t>(static_cast<uint64_t>(spec.gain_milli) *
                            volumes_[bus_index(bus)] /
                            mixer_defaults::kVolumeMaxMilli);
  cue.spec = spec;
  active.push_back(cue);
  sink_.schedule(cue);

  result.status = Admission::Admitted;
  result.sequence = cue.sequence;
  return result;
}

std::vector<ScheduledCue> AudioMixer::active_voices(Bus bus) const {
  std::vector<ScheduledCue> ordered = voices_[bus_index(bus)];
  std::sort(ordered.begin(), ordered.end(), canonical_order);
  return ordered;
}

}  // namespace verdigris::audio
