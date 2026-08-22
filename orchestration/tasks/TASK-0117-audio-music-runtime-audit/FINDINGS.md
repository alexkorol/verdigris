# TASK-0117 — Native sound and music runtime audit (FINDINGS)

- Worker: ox-pc-t (provider `openrouter`, model `stealth/ox-alpha`)
- Base: `9fe673b66ffc082e865e0f0fb66f454ec1984949` (routing-pinned; SPEC frontmatter
  names `9bd689b4…`, see REPORT deviations — audit is read-only so tree currency is
  the operative requirement)
- Machine-readable companion: [`captures/audio-surfaces.json`](captures/audio-surfaces.json)
- Method: literal acceptance sweep (`rg -n "audio|sound|music|volume|mute|device|spatial|voice|ambience" native src server docs …`)
  plus targeted follow-up reads. Every claim below cites `path:line`. Read-only:
  no assets downloaded/generated/played, no ports, no source changes.

## 1. Headline: there is no native audio path at all

The native workspace contains **zero** audio code and **zero** audio
dependencies. The acceptance sweep over `native/` hits only `std::mutex`,
the "12x7 spatial backpack" comment (`native/include/verdigris/core.hpp:639`),
and an unrelated "at this volume" test comment
(`native/tests/session_tests.cpp:532`). `native/CMakeLists.txt` links no audio
library; `rg -i 'audio|openal|sdl|miniaudio|wasapi|coreaudio' native/CMakeLists.txt`
returns nothing. Combat feedback today is exclusively visual: impact flashes,
target tints, damage numbers, screen pulse
(`native/client/main.cpp:1774-1801`). The constitution demands "strong impact
feedback" for readable combat (`docs/product/VERDIGRIS_CONSTITUTION.md:100-101`);
sound is the missing channel.

## 2. The authoritative event inputs already exist — audio needs no simulation changes

A successor runtime can be wired entirely presentation-side:

- Core event log: `EventType` enum + `Event{type, actor_id, item_id,
  trophy_id, text, value, tick}` with `Simulation::emit(...)`
  (`native/include/verdigris/core.hpp:221-262`, `:329`, `:373-374`). Emission
  sites cover every cue family a Bronze Age action RPG needs: `AttackStarted`
  (`native/src/core.cpp:405`), `DamageApplied` (`core.cpp:417`, enemy melee
  `:714`), `AttackTelegraphed` thrust/sweep with windup ticks (`:694`, `:700`),
  `BuffApplied/BuffExpired` war-cry (`:352`, `:734`), `ActorDied` monster/scion
  (`:824`, `:892`), item/trophy drop & pickup & equip (`:749`, `:784`, `:506`,
  `:522`), `InstanceEntered` (`:559`), `ExpeditionPhaseChanged` (`:858`),
  `RouteUnlocked` (`:798`), `RelicResurfaced` (`:761`).
- Client seam: C3 `PresentationEvent` stream, explicitly "transient
  presentation events … carry no gameplay authority"
  (`native/client/presentation_events.hpp:3-6`), drained via
  `Session::drain_events()` (`native/client/session.hpp:94`;
  `local_session.cpp:158`, `remote_session.cpp:525`) and consumed by
  cursor-based ingest loops (`native/client/main.cpp:1737` local,
  `:737` remote). An audio mixer belongs exactly beside this switch.
- Remote wire: `combat:hit` envelopes already carry
  `amount/critical/attackStyle/targetType/died/health`
  (`native/src/networking.cpp:1998-2005`) — enough to differentiate hit vs crit
  vs kill cues without protocol changes; `monster:telegraph` carries
  `radius/durationMs` for windup risers (`networking.cpp:1991-1995`).

**Seam gaps to close in the successor** (all additive): map
`InstanceEntered`/`ExpeditionPhaseChanged`/`RouteUnlocked`/`RelicResurfaced`,
`BuffApplied/BuffExpired`, and trophy events into `PresentationEventType`
(today absent — `presentation_events.hpp:12-27`); consider adding `tick` to
`PresentationEvent` (core `Event` has it at `core.hpp:261`) so cues can be
ordered/coalesced deterministically.

## 3. Browser reference: a working pattern wrapped around dead seams

The historical client proves the *shape* of the solution but almost none of it
is live:

- `src/core/audio/sound-system.js` maps five bus events to synthesized
  oscillator cues (`sound-system.js:3-9`, synthesis `:41-74`) and solves
  autoplay/gesture unlock by dropping cues while suspended rather than queuing
  them (`:31-45`). But **`SoundSystem` is never instantiated in app code** — its
  only importer is its own spec (`tests/unit/sound-system.spec.js:4`).
- Of the five mapped cues only `sound:loot` has producers
  (`src/core/player/events/item.js:25` from inventory-diff;
  `src/core/player/events/loot-moment.js:87`). `sound:combat-hit`,
  `sound:monster-kill`, `sound:zone`, `sound:final-death` have zero producers.
- Menu music is a single looped mp3 in the auth shell
  (`src/components/sub/AudioMainMenu.vue:35-39`, mounted via
  `AuthContainer.vue:10`). `music:start` has **no producer anywhere**; music
  starts only by manual button click. `music:stop` fires once, on game start
  (`src/Delaford.vue:1735`).
- Settings promise "Combat, loot, and world cues"
  (`src/components/slots/Settings.vue:52`) and persist a `soundEffects` boolean
  (`src/stores/ui.js:22,80`), but the toggle's bus broadcast `SETTINGS:SOUND`
  (`Settings.vue:109`) has **no listener** — it gates nothing because nothing
  plays.
- The server emits no audio events; the loot chime is derived client-side from
  an inventory diff (`server/` sweep matches only the word "sounder" in flavor
  text, `server/core/items/vesselforge/verdigris-pack.js:292`).

Lesson carried forward: the browser built consumers before producers and never
wired the consumer. The successor should land **event mapping + mixer +
settings gating as one vertical slice**, or the same dead-seam pattern repeats.

## 4. Negative control (SPEC-required)

**Load-bearing combat event with no audio consumer:** `combat:hit` /
core `DamageApplied` — emitted on every strike with amount, critical, and
attack-style fields (`native/src/networking.cpp:2005`; `native/src/core.cpp:417`,
`:714`) and consumed solely for visual effects
(`native/client/main.cpp:1774-1801`). No audio consumer exists in any codebase.
Secondary dead seams: `SETTINGS:SOUND` (no listener),
`music:start` (listener without producer), and the four unmapped SFX cues above.

## 5. Current surface inventory (what exists per SPEC axis)

| Axis | Native | Browser reference |
|---|---|---|
| Sound hooks/assets | none | synth-only SFX seam (unwired) + one menu mp3 (`src/assets/audio/music/main_menu.mp3`, 5,068,686 bytes; archived bundle-bloat flag `docs/archive/code-review.md:229,282`) |
| Authoritative event inputs | rich: core `Event` log + `PresentationEvent` stream + `combat:hit`/`monster:telegraph` envelopes | bus events derived client-side from server state |
| Spatial/2D buses | none | none (tones connect straight to destination, `sound-system.js:56`) |
| Priorities / voice limits | none | none (unbounded oscillator overlap; mitigated only by short durations) |
| Music states/transitions | none | start/stop loop only |
| Ambience | none | none |
| UI cues | none | promised by Settings copy, only loot delivered |
| Device lifecycle | none | lazy context + gesture resume + drop-while-suspended (`sound-system.js:31-45`) |
| Volume/mute/accessibility | none | sfx toggle persisted (`ui.js:22,80`); aria-labeled mute button (`AudioMainMenu.vue:7`); no volume sliders |
| Deterministic test seams | scenario harness + recorded ops precedent (`native/README.md:90-118`); `drain_events()` tests (`session_tests.cpp:71…`) | fake-AudioContext oscillator assertions (`sound-system.spec.js:7-60`); `$emit` spies (`loot-first-find.spec.js:241-254`) |
| Packaging | no audio targets/deps in CMake | mp3 shipped as static asset |
| Windows/macOS backend | platform seam reserved, SDL3 named as direction (`native/platform/README.md`) | n/a |

## 6. Concrete recommendations for the synthetic-placeholder successor

Backend-neutral interface first (constitution: audio consumes events, never
gameplay authority):

1. **`verdigris_client_audio` static library** beside `client_session`
   (`native/CMakeLists.txt` target layout), owning three seams:
   - `audio::Sink` (pure virtual): `schedule(CueSpec)` + `set_bus_volume` +
     `drain_scheduled()` returning what was voiced — the test recorder
     implements this; the real backend implements it against a device.
   - `AudioMixer` consuming the existing `PresentationEvent` drain loop next to
     `ingest_events` (`main.cpp:1737`) — pure translation Event→CueSpec, unit
     -testable headless.
   - Cue table keyed by `(PresentationEventType, text discriminator, value)`
     mirroring how ingest already switches on `event.text` ("war-cry",
     "thrust", "sweep") — e.g. `AttackTelegraphed{text:"sweep"}` → rising
     windup for `event.value` ticks.
2. **Placeholder synthesis mirrors the browser trick**: procedural
     oscillator/noise cues generated at runtime (hit thud, kill sting, loot
     chime pair, zone swell, death drone — parameters reusable from
     `sound-system.js:63-73`), zero asset files, zero licensing exposure.
     Music stays owner-input-blocked (SPEC `owner_input_dependency`); reserve a
     music state machine stub (`menu/expedition/safehouse` states + fade
     transitions) behind the same sink.
3. **Backend choice**: default candidate **miniaudio** (single vendored header;
     WASAPI on Windows, CoreAudio on macOS; matches the dependency-free ethos)
     with SDL3-audio as the alternative if the platform seam lands SDL3 first
     (`native/platform/README.md`). Defer raw WASAPI/CoreAudio — two hand-
     maintained backends exceed proof-slice scope. Lifecycle requirements:
     default-output-device change notification, suspend/resume parity with the
     browser's gesture-unlock behavior (drop, don't queue, while locked), silent
     no-op when no device.
4. **Mix architecture now, even for placeholders**: two buses minimum (UI/SFX +
     music), priority classes UI > player-feedback > world, and a per-class
     voice cap with steal-the-oldest policy — the browser's unbounded
     oscillator fan-out (`loot` alone schedules 2 voices per emit) is the anti-
     pattern to avoid under elite-pack fights.
5. **Settings**: persist `audio.sfxVolume/musicVolume/muted` in the client
     settings store equivalent to `ui.js` semantics (default-on, tri-state not
     required); the toggle must gate the mixer, not just persist.
6. **Tests**: headless assertions against a recording `Sink` (assert scheduled
     cue sequence per scripted command stream, mirroring `render_list` op
     assertions per `native/README.md:104-117`), plus one new client scenario
     per the every-wave rule; keep the simulation byte-identical (no core
     changes needed — evidence in §2).

Stop conditions respected: no dependency selected/vendored, no assets touched,
no music invented — those are successor-task decisions requiring owner input on
licensing and direction.
