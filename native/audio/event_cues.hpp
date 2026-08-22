// TASK-0157: stable translation from existing PresentationEvent values into
// content-neutral procedural cue parameters. Exactly the five representative
// beats named by the SPEC are mapped (ordinary hit, critical hit, enemy
// defeat, Scion loss, war-cry expiry); every other event or discriminator is
// deliberately silent. This table is data translation only — it never touches
// the simulation, the wire, or any audio device.

#pragma once

#include "cue_spec.hpp"
#include "presentation_events.hpp"

namespace verdigris::audio {

// Returns true and fills `out` when the event maps to a cue; false means the
// event is unknown to the cue table and must stay silent.
bool cue_for_event(const verdigris::client::PresentationEvent& event,
                   CueSpec* out);

}  // namespace verdigris::audio
