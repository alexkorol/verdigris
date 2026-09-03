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
  ScionLost,           // TASK-0122 Phase A: permanent Scion loss beat
  BuffApplied,         // authoritative temporary player effect began
  BuffExpired,         // TASK-0122 Phase A: authoritative buff end (war cry)
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
  // TASK-0122 Phase A: already-authoritative combat:hit parity data. The
  // remote session copies these from the envelope; it never computes them.
  bool critical = false;
  std::string style;
  int combo_step = 0;
  int combo_window_ms = 0;
  int stagger_ms = 0;
};

// TASK-0122 Phase A: the single named table for every new animation/VFX TTL,
// pulse, color, and style value introduced by this packet. One 50 ms tick
// matches the fixed simulation cadence; presentation lifetimes are
// presentation-owned interpolation and never alter authoritative timing.
namespace phase_a {

inline constexpr int kTickMs = 50;

// Critical-hit treatment (consumes shipped combat:hit critical/attackStyle).
inline constexpr int kCriticalNumberTtlTicks = 16;   // 800 ms, vs 600 ms normal
inline constexpr int kCriticalFlashTtlTicks = 6;     // 300 ms vs 200 ms normal
struct Rgb {
  int r = 0;
  int g = 0;
  int b = 0;
};
inline constexpr Rgb kCriticalNumberColor{255, 176, 64};   // white-hot orange
inline constexpr Rgb kCriticalFlashColor{255, 246, 214};   // near-white burst
inline constexpr const char* kCriticalDamageLabel = "critical";  // + ":" + style

// Authoritative third-beat melee finisher treatment.
inline constexpr int kComboFinisherTtlTicks = 10;          // 500 ms
inline constexpr Rgb kComboFinisherColor{104, 232, 204};   // verdigris flare
inline constexpr const char* kComboFinisherLabel = "combo-finisher";

// Deterministic spawn/materialization beat (first client sighting of a foe).
inline constexpr int kMaterializeTtlTicks = 8;             // 400 ms
inline constexpr Rgb kMaterializeColor{120, 220, 190};     // verdigris teal
inline constexpr const char* kSpawnRenderLabel = "spawn";

// War-cry expiration beat (BuffExpired "war-cry").
inline constexpr int kWarcryFadeTtlTicks = 10;             // 500 ms
inline constexpr Rgb kWarcryFadeColor{214, 168, 72};       // dimmed gold
inline constexpr const char* kWarcryFadeLabel = "warcry-fade";

// ScionLost beat: the longest, most somber beat in the packet.
inline constexpr int kScionLostRingTtlTicks = 40;          // 2000 ms
inline constexpr int kScionLostPulseTicks = 12;            // 600 ms edge pulse
inline constexpr Rgb kScionLostColor{186, 74, 62};         // deep rust red
inline constexpr const char* kScionLostLabel = "scion-lost";

}  // namespace phase_a

}  // namespace verdigris::client
