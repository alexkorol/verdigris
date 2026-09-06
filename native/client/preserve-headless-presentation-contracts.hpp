#pragma once

#include "presentation_events.hpp"
#include "verdigris/core.hpp"

// VG-QA-002: one sim event maps to a render/audio intent. A mocked
// PresentationEvent that the simulation never emitted cannot prove the
// journey. native/tests stays with Kimi; this fixture is the client lease.

namespace verdigris::client::qa {

struct SemanticIntent {
  PresentationEventType voiced = PresentationEventType::Message;
  const char* visual = "intent:swing";
  const char* audio_cue = "attack-anticipate";
};

inline bool intent_for_attack_started(SemanticIntent* out) {
  if (!out) return false;
  out->voiced = PresentationEventType::AttackStarted;
  out->visual = "intent:swing";
  out->audio_cue = "attack-anticipate";
  return true;
}

template <typename Events>
inline bool sim_emitted(const Events& events, verdigris::EventType type) {
  for (const auto& event : events)
    if (event.type == type) return true;
  return false;
}

inline bool journey_proved(bool sim_emitted_attack, bool bridge_mapped,
                           bool saw_visual, bool saw_audio) {
  return sim_emitted_attack && bridge_mapped && saw_visual && saw_audio;
}

inline bool mock_without_sim_rejected(bool sim_emitted_attack, bool mock_voiced) {
  return mock_voiced && !sim_emitted_attack;
}

}  // namespace verdigris::client::qa
