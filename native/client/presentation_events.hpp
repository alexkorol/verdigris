#pragma once

// C3 session seam (D-122): transient presentation events. Renderers consume
// these for effects that must not be inferred by diffing snapshots. Events
// carry no gameplay authority — the session that emitted them already
// resolved the consequences.

#include <string>

namespace verdigris::client {

enum class PresentationEventType {
  ConnectionEstablished,
  ConnectionLost,
  SessionReady,        // login accepted; model.player is authoritative
  AttackStarted,
  DamageApplied,
  ActorDied,
  ItemDropped,
  ItemPickedUp,
  ItemEquipped,
  ExtractionCompleted,
  ScionDied,
  Telegraph,
  Message,             // human-readable server/system line in `text`
  ProtocolError,       // malformed or unexpected envelope in `text`
};

struct PresentationEvent {
  PresentationEventType type;
  std::string actor_id;
  std::string item_id;
  std::string text;
  int value = 0;
};

}  // namespace verdigris::client
