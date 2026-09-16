#pragma once

// TASK-0157: content-neutral cue table. Translates the representative
// TASK-0157 trigger set into procedural cue parameters. Every value here is
// structural (waveform family, duration, envelope, relative pitch contour):
// it pins no absolute frequency, no authored sound, no license, and no
// music direction, which remain owner-only decisions.

#include "audio_cues.hpp"

namespace verdigris::audio {

struct CueMapping {
  bool mapped = false;  // false = explicit silence for unknown triggers.
  Bus bus = Bus::Sfx;
  Priority priority = Priority::World;
  CueSpec spec{};
};

// Pure, total function over the trigger taxonomy. Same input, same output,
// forever: the mapping is contract, not tuning.
CueMapping cue_for_trigger(CueTrigger trigger);

}  // namespace verdigris::audio
