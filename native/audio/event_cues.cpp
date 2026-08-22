#include "event_cues.hpp"

namespace verdigris::audio {
namespace {

// Provisional placeholder parameters. Final sounds, frequencies, and music
// remain an owner-only decision (TASK-0157 owner_input_dependency); these
// values exist so the scheduler's behavior is pinned and testable.
constexpr CueParams kHitParams{Waveform::Sine, 220, 110, 90, 480};
constexpr CueParams kCritParams{Waveform::Square, 440, 110, 150, 640};
constexpr CueParams kKillParams{Waveform::Sawtooth, 196, 49, 240, 560};
constexpr CueParams kScionLostParams{Waveform::Sine, 165, 41, 900, 700};
constexpr CueParams kWarcryExpireParams{Waveform::Sine, 392, 262, 300, 420};

}  // namespace

bool cue_for_event(const verdigris::client::PresentationEvent& event,
                   CueSpec* out) {
  if (out == nullptr) return false;
  *out = CueSpec{};
  switch (event.type) {
    case verdigris::client::PresentationEventType::DamageApplied:
      if (event.critical) {
        out->cue_id = "crit";
        out->params = kCritParams;
      } else {
        out->cue_id = "hit";
        out->params = kHitParams;
      }
      out->bus = Bus::Sfx;
      out->priority = PriorityClass::PlayerFeedback;
      return true;
    case verdigris::client::PresentationEventType::ActorDied:
      // Keyed by (type, text discriminator), restoring the accepted
      // TASK-0117 contract: core emits ActorDied with "monster" for enemy
      // death but also ActorDied with "scion" immediately before ScionLost
      // for player death. Only "monster" maps; every other discriminator
      // stays silent so a Scion death never schedules a kill cue.
      if (event.text != "monster") return false;
      out->cue_id = "kill";
      out->params = kKillParams;
      out->bus = Bus::Sfx;
      out->priority = PriorityClass::World;
      return true;
    case verdigris::client::PresentationEventType::ScionLost:
      out->cue_id = "scion-lost";
      out->params = kScionLostParams;
      out->bus = Bus::Sfx;
      out->priority = PriorityClass::PlayerFeedback;
      return true;
    case verdigris::client::PresentationEventType::BuffExpired:
      // The cue table is keyed by (type, text discriminator), mirroring the
      // existing event.text switches at the presentation seam.
      if (event.text != "war-cry") return false;
      out->cue_id = "warcry-expire";
      out->params = kWarcryExpireParams;
      out->bus = Bus::Sfx;
      out->priority = PriorityClass::PlayerFeedback;
      return true;
    default:
      return false;
  }
}

}  // namespace verdigris::audio
