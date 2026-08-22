#include "cue_spec.hpp"

#include <cstdio>

namespace verdigris::audio {

std::string bus_name(Bus bus) {
  switch (bus) {
    case Bus::Sfx:
      return "sfx";
    case Bus::Music:
      return "music";
  }
  return "unknown";
}

std::string priority_name(PriorityClass priority) {
  switch (priority) {
    case PriorityClass::World:
      return "world";
    case PriorityClass::PlayerFeedback:
      return "player";
    case PriorityClass::Ui:
      return "ui";
  }
  return "unknown";
}

std::string waveform_name(Waveform waveform) {
  switch (waveform) {
    case Waveform::Sine:
      return "sine";
    case Waveform::Square:
      return "square";
    case Waveform::Sawtooth:
      return "sawtooth";
    case Waveform::Noise:
      return "noise";
  }
  return "unknown";
}

std::string serialize_schedule(const std::vector<CueSpec>& cues) {
  std::string out;
  out.reserve(cues.size() * 96);
  for (const CueSpec& cue : cues) {
    char line[128];
    std::snprintf(line, sizeof(line),
                  "cue[%06llu] tick=%llu bus=%s prio=%s id=%s wave=%s %d->%dHz "
                  "%dms gain=%d effective=%d\n",
                  static_cast<unsigned long long>(cue.sequence),
                  static_cast<unsigned long long>(cue.scheduled_tick),
                  bus_name(cue.bus).c_str(),
                  priority_name(cue.priority).c_str(), cue.cue_id.c_str(),
                  waveform_name(cue.params.waveform).c_str(),
                  cue.params.start_hz, cue.params.end_hz,
                  cue.params.duration_ms, cue.params.gain_permille,
                  cue.effective_gain_permille);
    out += line;
  }
  return out;
}

}  // namespace verdigris::audio
