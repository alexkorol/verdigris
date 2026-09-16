#pragma once

// TASK-0157: read-only translation seam from the existing presentation
// event stream into the cue taxonomy. This header only classifies; the
// later client-integration wave (outside this packet's ownership) wires
// the mixer beside the drain loop. The included client header is consumed
// read-only and is never modified by the audio library.

#include "../client/presentation_events.hpp"

#include "audio_cues.hpp"

namespace verdigris::audio {

// Total, pure classification over PresentationEvent values. Anything the
// representative TASK-0157 set does not cover returns Unknown, which maps
// to explicit silence downstream.
inline CueTrigger classify_presentation_event(
    const verdigris::client::PresentationEvent& event) {
  using verdigris::client::PresentationEventType;
  switch (event.type) {
    case PresentationEventType::DamageApplied:
      return event.critical ? CueTrigger::CriticalHit : CueTrigger::OrdinaryHit;
    case PresentationEventType::ActorDied:
      return CueTrigger::EnemyDefeat;
    case PresentationEventType::ScionLost:
      return CueTrigger::ScionLoss;
    case PresentationEventType::BuffExpired:
      return event.text == "war-cry" ? CueTrigger::WarCryExpire
                                     : CueTrigger::Unknown;
    default:
      return CueTrigger::Unknown;
  }
}

}  // namespace verdigris::audio
