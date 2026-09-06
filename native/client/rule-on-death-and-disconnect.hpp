#pragma once

// VG-GOV-006: presentation must not treat disconnect, quit, crash, or
// Scion death as an extraction ack. Core death/relics stay with Kimi /
// TASK-0018 (D-106). This table is the HUD/policy lease.

namespace verdigris::client::gov {

enum class EndEvent { Death, Quit, Disconnect, Crash };
enum class Carry { Uncommitted, ExtractCommitted };

inline bool silent_disconnect_ack(EndEvent event, Carry carry) {
  const bool lost = event == EndEvent::Disconnect || event == EndEvent::Crash;
  return lost && carry == Carry::Uncommitted;
}

inline const char* extract_hud(EndEvent event, Carry carry) {
  if (carry == Carry::ExtractCommitted) return "extract:ok";
  if (event == EndEvent::Death) return "extract:uncommitted";
  if (event == EndEvent::Quit) return "extract:uncommitted";
  if (silent_disconnect_ack(event, carry)) return "extract:uncommitted";
  return "extract:pending";
}

inline const char* carried_value(EndEvent event, Carry carry) {
  if (carry == Carry::ExtractCommitted) return "already-banked";
  if (event == EndEvent::Death) return "relic-pool";
  return "uncommitted";
}

inline const char* house_value(Carry carry) {
  return carry == Carry::ExtractCommitted ? "keeps-banked" : "no-new-credit";
}

inline const char* recovery(EndEvent event, Carry carry) {
  if (carry == Carry::ExtractCommitted) return "reconnect-or-successor";
  if (event == EndEvent::Death) return "re-entry-roll";
  if (event == EndEvent::Disconnect || event == EndEvent::Crash)
    return "reconnect-uncommitted";
  return "re-enter-instance";
}

inline bool paints_extract_ok(EndEvent event, Carry carry) {
  (void)event;
  return carry == Carry::ExtractCommitted;
}

}  // namespace verdigris::client::gov
