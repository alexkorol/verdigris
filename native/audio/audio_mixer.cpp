#include "audio_mixer.hpp"

#include <algorithm>
#include <utility>

#include "event_cues.hpp"

namespace verdigris::audio {

void RecordingSink::schedule(const CueSpec& cue) { cues_.push_back(cue); }

AudioMixer::AudioMixer(Sink& sink, std::uint32_t sfx_voice_cap,
                       std::uint32_t music_voice_cap)
    : sink_(sink) {
  buses_[0].voice_cap = sfx_voice_cap;
  buses_[1].voice_cap = music_voice_cap;
}

AudioMixer::BusState& AudioMixer::bus_state(Bus bus) {
  return buses_[static_cast<int>(bus)];
}

const AudioMixer::BusState& AudioMixer::bus_state(Bus bus) const {
  return buses_[static_cast<int>(bus)];
}

bool AudioMixer::ingest(const verdigris::client::PresentationEvent& event,
                        std::uint64_t tick) {
  CueSpec spec;
  if (!cue_for_event(event, &spec)) return false;
  spec.scheduled_tick = tick;
  spec.sequence = next_sequence_++;
  pending_.push_back(std::move(spec));
  return true;
}

void AudioMixer::submit(CueSpec spec) {
  spec.sequence = next_sequence_++;
  pending_.push_back(std::move(spec));
}

void AudioMixer::set_bus_volume(Bus bus, int volume_permille) {
  if (volume_permille < 0) volume_permille = 0;
  if (volume_permille > 1000) volume_permille = 1000;
  bus_state(bus).volume_permille = volume_permille;
}

void AudioMixer::set_bus_muted(Bus bus, bool muted) {
  bus_state(bus).muted = muted;
}

int AudioMixer::bus_volume(Bus bus) const {
  return bus_state(bus).volume_permille;
}

bool AudioMixer::bus_muted(Bus bus) const { return bus_state(bus).muted; }

namespace {

bool voice_order(const CueSpec& a, const CueSpec& b) {
  if (a.scheduled_tick != b.scheduled_tick) {
    return a.scheduled_tick < b.scheduled_tick;
  }
  return a.sequence < b.sequence;
}

// Steal-oldest victim inside a bus: lowest priority first, then oldest.
bool steal_victim_order(const CueSpec& a, const CueSpec& b) {
  if (a.priority != b.priority) {
    return static_cast<int>(a.priority) < static_cast<int>(b.priority);
  }
  return a.sequence < b.sequence;
}

}  // namespace

std::vector<CueSpec> AudioMixer::drain_scheduled() {
  std::vector<CueSpec> ordered = pending_;
  pending_.clear();
  std::sort(ordered.begin(), ordered.end(), voice_order);

  std::vector<CueSpec> active[2];
  for (CueSpec& cue : ordered) {
    const BusState& state = bus_state(cue.bus);
    if (state.muted || state.volume_permille == 0) continue;
    const int effective =
        cue.params.gain_permille * state.volume_permille / 1000;
    if (effective == 0) continue;
    std::vector<CueSpec>& voices = active[static_cast<int>(cue.bus)];
    if (voices.size() < state.voice_cap) {
      cue.effective_gain_permille = effective;
      voices.push_back(cue);
      continue;
    }
    if (state.voice_cap == 0) continue;
    const auto victim = std::min_element(voices.begin(), voices.end(),
                                         steal_victim_order);
    if (static_cast<int>(victim->priority) >
        static_cast<int>(cue.priority)) {
      continue;  // every active voice outranks the newcomer; drop it
    }
    *victim = cue;
    victim->effective_gain_permille = effective;
  }

  std::vector<CueSpec> voiced;
  voiced.reserve(active[0].size() + active[1].size());
  for (auto& bus_voices : active) {
    voiced.insert(voiced.end(), bus_voices.begin(), bus_voices.end());
  }
  std::sort(voiced.begin(), voiced.end(), voice_order);
  for (const CueSpec& cue : voiced) sink_.schedule(cue);
  return voiced;
}

}  // namespace verdigris::audio
