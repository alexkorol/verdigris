#include "cue_library.hpp"

#include <cstring>

namespace verdigris::audio {

namespace {

CueLabel make_label(const char* text) {
  CueLabel label{};
  for (size_t i = 0; text[i] != '\0' && i + 1 < kCueLabelBytes; ++i)
    label[i] = text[i];
  return label;
}

}  // namespace

// Pitch fields are permilli ratios of a runtime reference pitch (1000 =
// reference). The reference itself is resolved by the future device backend
// and is an owner-only decision; no absolute Hz appears anywhere in this
// packet. Contours are relative: e.g. 1400 -> 700 falls one octave-neutral
// ratio regardless of where the reference lands.
CueMapping cue_for_trigger(CueTrigger trigger) {
  CueMapping mapping;
  switch (trigger) {
    case CueTrigger::OrdinaryHit:
      mapping.mapped = true;
      mapping.bus = Bus::Sfx;
      mapping.priority = Priority::Feedback;
      mapping.spec.label = make_label("hit");
      mapping.spec.waveform = Waveform::Noise;
      mapping.spec.duration_ms = 90;
      mapping.spec.attack_ms = 2;
      mapping.spec.release_ms = 60;
      mapping.spec.gain_milli = 550;
      mapping.spec.pitch_start_permil = 1000;
      mapping.spec.pitch_end_permil = 1000;
      break;
    case CueTrigger::CriticalHit:
      mapping.mapped = true;
      mapping.bus = Bus::Sfx;
      mapping.priority = Priority::Feedback;
      mapping.spec.label = make_label("crit-hit");
      mapping.spec.waveform = Waveform::Noise;
      mapping.spec.duration_ms = 160;
      mapping.spec.attack_ms = 2;
      mapping.spec.release_ms = 110;
      mapping.spec.gain_milli = 750;
      // Falling contour, louder and longer than the ordinary hit.
      mapping.spec.pitch_start_permil = 1400;
      mapping.spec.pitch_end_permil = 700;
      break;
    case CueTrigger::EnemyDefeat:
      mapping.mapped = true;
      mapping.bus = Bus::Sfx;
      mapping.priority = Priority::World;
      mapping.spec.label = make_label("enemy-defeat");
      mapping.spec.waveform = Waveform::Tone;
      mapping.spec.duration_ms = 320;
      mapping.spec.attack_ms = 4;
      mapping.spec.release_ms = 240;
      mapping.spec.gain_milli = 600;
      mapping.spec.pitch_start_permil = 1200;
      mapping.spec.pitch_end_permil = 400;
      break;
    case CueTrigger::ScionLoss:
      mapping.mapped = true;
      mapping.bus = Bus::Sfx;
      mapping.priority = Priority::Feedback;
      mapping.spec.label = make_label("scion-lost");
      mapping.spec.waveform = Waveform::Tone;
      mapping.spec.duration_ms = 900;
      mapping.spec.attack_ms = 20;
      mapping.spec.release_ms = 600;
      mapping.spec.gain_milli = 800;
      // Slow, somber fall; the longest cue in the representative set.
      mapping.spec.pitch_start_permil = 500;
      mapping.spec.pitch_end_permil = 250;
      break;
    case CueTrigger::WarCryExpire:
      mapping.mapped = true;
      mapping.bus = Bus::Sfx;
      mapping.priority = Priority::Ui;
      mapping.spec.label = make_label("warcry-end");
      mapping.spec.waveform = Waveform::Tone;
      mapping.spec.duration_ms = 240;
      mapping.spec.attack_ms = 10;
      mapping.spec.release_ms = 180;
      mapping.spec.gain_milli = 450;
      mapping.spec.pitch_start_permil = 800;
      mapping.spec.pitch_end_permil = 640;
      break;
    case CueTrigger::Unknown:
      // Explicit silence: unknown events never become cues.
      mapping.mapped = false;
      break;
  }
  return mapping;
}

}  // namespace verdigris::audio
