# VG-SOUND-002 — legal combat sound family (planning ID)

Cooked bank is the procedural mixer table. Provenance lives in
`native/client/sound_family.hpp` (SPDX CC0-1.0, synth source tags).

Roles: impact (`hit`/`crit`/`kill`), warning (`scion-lost`/`warcry-expire`),
swing placeholder (`cosmetic`).

Negative: a cue that plays without license/source fails `legal-sounds`.

Acceptance: `native/build/verdigris_client.exe --scenario legal-sounds`
