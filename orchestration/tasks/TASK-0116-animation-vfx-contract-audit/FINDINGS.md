# TASK-0116 FINDINGS — Native animation and VFX contract audit

- **Worker:** ox-pc-s (OpenCode Ox Alpha), provider `openrouter`, model `stealth/ox-alpha`
- **Base:** `9fe673b66ffc082e865e0f0fb66f454ec1984949` (spec base `9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4`)
- **Capsule:** read-only audit. No game source, SPEC, REVIEW, other tasks, other worktrees, or port 6500 touched. Everything below cites the tree as it exists at the base commit.
- **Machine-readable companion:** `captures/animation-vfx-matrix.json` (18 rows, 5 negative controls, 4 successor phases). Row/NC ids below reference it.

---

## 1. Executive picture

The native presentation already has a **real, testable animation/VFX contract skeleton**: a frozen simulation telegraph timing contract (`kTelegraphTicks`, exact resolution offset, cancel-on-death, replay determinism), a semantic per-frame render list recorded next to every draw (D-119), byte-deterministic staged captures in five scenes at two resolutions, and scenario/test coverage for facing, dash, telegraph dodge, and damage-number lifetime. That skeleton is the successor's foundation and it is sound.

What does **not** exist is anything an ARPG player would call animation: there are **no walk cycles, no attack lunges, no spawn/death body motion, no positional interpolation of actors, no particle layer, no crit distinction, no camera shake** — actors teleport per 20 Hz tick under static mirrored billboards. The gaps are not accidents; they are unbuilt layers. Every gap below is phrased so TASK-0122 can promote directly into it.

**Row scorecard:** 7 COMPLETE, 9 PARTIAL, 4 MISSING, 1 OWNER-ASSET (details in the matrix).

---

## 2. Authority frame (frozen, honored throughout)

| Authority | Where | Audit consequence |
|---|---|---|
| Commands/events authority | constitution "Native architecture invariant"; `presentation_events.hpp:3-6` ("events carry no gameplay authority") | All presentation timing below is classified as presentation-owned interpolation; nothing proposes moving authority into the client except where explicitly flagged as an open decision (hit-stop, R07). |
| D-114 timing coherence | `DECISIONS.md` D-114; `core.hpp:43-73`; N3 table `core.cpp:1463-1474` | Any successor feel retune must re-derive the whole table in one diff. Current table is locked by `test_d114_world_scale_table` (`core_tests.cpp:1693`). |
| Render-list determinism | `render_list.hpp:1-8`; two-run byte-equality gate `main.cpp:4493-4498` | Determinism is proven for staged frames; multi-tick sequences are not yet gated (R12/R17 gap → Phase B). |
| D-115 play verdicts | `DECISIONS.md` D-115 | This audit produces no feel verdicts; it only maps what the architect will later judge. |
| D-113 art direction | `DECISIONS.md` D-113 | Procedural/vector-first; owner approves image assets. Drives the OWNER-ASSET classification (R16) and Phase D. |

---

## 3. The required separation: simulation-authored vs presentation-owned

**Simulation-authored timing/events (authoritative, deterministic, replay-locked):**

- 20 Hz fixed step and step derivation (`core.hpp:34-41`); D-114 distance/time table (`core.hpp:43-73`).
- 8-way integer facing from movement and aim; monster pursuit facing; thrust cone (`core.cpp:61-66, 312-328, 674-677`).
- `ActorMoved` per move; dash = named 10-tick burst (`core.hpp:75-77`, `core.cpp:338-344`).
- `AttackStarted` after cooldown gates (`core.cpp:393-405`); `AttackTelegraphed` with `kTelegraphTicks=3` windup (`core.hpp:29-32`, `core.cpp:692-701`); damage exactly 3 ticks later; cancels on any death (`core.cpp:813-822`).
- N3 tile combat clock: 350 ms swing cadence, 2-tile/1000 ms boss telegraph (`core.cpp:1463-1474, 1986-2011`); N2 tile steps with sequence/startedAtMs/durationMs meta (`core.hpp:836-842`, `networking.cpp:805`).
- `DamageApplied`, `ActorDied`, `BuffApplied/BuffExpired`, `ScionLost` emissions (`core.cpp:416-418, 726-734, 809-899`).

**Presentation-only timing/interpolation (client-owned, no authority):**

- All FX lifetimes in 50 ms ticks: Swing 6, SweepArc 8, WarCryAura 14, Impact 4, TargetFlash 4, DamageNumber 12, DeathRing 12, Dust 8–10, Sparkle 24 (`main.cpp:1762-1832`, aging at `main.cpp:3135-3138`, `presentation_state.cpp:272-279`).
- Telegraph visibility ramp + sine pulse; client-side expiry sweep (`main.cpp:1508-1523, 1855-1868`); remote ms→ticks heuristic `value>20 ? value/50 : value` (`presentation_state.cpp:196`).
- Animated draw parameters (arc sweep, expanding rings, dust spread, number lift/fade): `main.cpp:1548-1671`.
- Camera follow lerp ×0.2/tick — the **only** positional interpolation in the client (`main.cpp:3143-3144`); actor positions snap (`presentation_state.cpp:90-91`).
- Ambient pulses: extraction pad `tick/9 % 2`, low-life orb `tick % 24 < 12`, quickbar cooldown sweep (`main.cpp:2645, 2105-2106, 2182-2191`).
- Remote-mode inventions to flag: monster/telegraph facing synthesized by inverting player facing (`presentation_state.cpp:113-115, 192-194`); kill-drop sparkle placed at the player's last tile (`remote_session.cpp:799-805`).

**Authored-asset needs (OWNER-ASSET, D-113):** character animation frames in any form; any new image plates. The committed procedural kit (TASK-0141/TASK-0144 lineage: `visual_kit.h`, 9 symbols / 375 shapes, data-only) plus the PNG→kit→geometric fallback chain and honest art-status chip (`main.cpp:957-1103, 3474-3508`) mean the successor never blocks on art.

---

## 4. Area findings (condensed; full citations in the matrix)

### 4.1 Actor facing — COMPLETE authority, one remote seam gap
Facing is quantized, replay-locked, cone-gated, and consumed by sprite mirroring, the authoritative facing tick, and the minimap arrow (R01). Gap: remote monsters/telegraphs get **inverted-player facing** because the wire snapshot carries no monster facing field (`presentation_state.cpp:113-115`). Phase A fix; trivial, zero feel risk.

### 4.2 Movement — timing COMPLETE, animation MISSING
Simulation movement timing is fully locked (R02). Presentation has **zero movement animation**: no walk cycle, no smoothing, static mirrored billboards (R03). This is the single largest readability gap versus the constitution's "readable physical space" requirement and the browser side-by-side evidence (`orchestration/benchmarks/side-by-side-2026-08-20/`). Phase C builds interpolation-only procedural motion from the `MovementStepInfo` meta that already ships on `player:movement`.

### 4.3 Attack / telegraph / hit / death / dodge timing
- **Attack start:** event + Swing/Sweep render ops proven in local and remote modes (R04, `session_tests.cpp:520`).
- **Elite telegraph:** the strongest contract in the codebase — exact tick offset, fizzle, cancel, replay, catalog stability, HUD-safe dodge scenario, and a frozen mid-windup capture (R05). One structural wart: local ticks vs remote `durationMs` are two ununified windup systems (F-CLOCK).
- **Boss ground-slam geometry:** MISSING (R06). The wire ships `x/y/radius`; the client keeps only `durationMs` (`remote_session.cpp:730-742`) and draws range-substituted shapes at guessed anchors (`main.cpp:1538-1543`). No capture proves the authored 2-tile radius.
- **Hit feedback:** flash/number/screen-pulse exist and the damage-number lifetime is scenario-pinned (~300 ms visible, gone ~650 ms; `main.cpp:3682-3693`), but `critical`/`beastbane`/`attackStyle` are dropped on the floor (R07, NC-01). No hit-stop or knockback concept exists anywhere.
- **Death:** ring+dust+drop sparkle with scenario locks; body vanishes at ring end; `ScionLost` has no presentation beat (R08, NC-02); remote death arrives only as a `died` flag.
- **Dodge/dash:** sim burst locked; avoidance scenario locked; dust trail and any dodge *motion* unpinned/absent (R09).

### 4.4 Render-list events and determinism — COMPLETE
26-op semantic vocabulary recorded next to each draw, headless recorder with identical vocabulary, stable depth sort with integer tie-break, id-sorted loot iteration, and the two-run byte-equality gate (R11/R12). This is the successor's measurement instrument; extending it follows the documented recipe (`native/README.md:102-116`).

### 4.5 Particles / auras / orbs — PARTIAL
Nine hand-drawn effect kinds aged by tick counter; largest particle multiplicity is Dust's five dots (R13). War cry aura renders on apply only — `BuffExpired` is unmapped and unrendered (NC-03). The `aura:damage` empowered state ships as data and is rendered **nowhere** (R10). Orbs/quickbar/extraction pulses exist with presence assertions but no cadence pins. There is no particle abstraction; everything is switch-case GDI inside a 4,682-line `main.cpp`, and the `renderer/` seam is still documentation-only (`native/renderer/README.md:1-5`).

### 4.6 Layering — COMPLETE
Translation-invariant `wy*1e6+wx` painter key, class-ordered depth collection, effects anchored above their targets, telegraphs on the ground plane beneath billboards, HUD-reserve clamping asserted at 960×600 (R14). Residual risk: no explicit overlap-correctness fixture; determinism gate covers stability, not semantics.

### 4.7 Camera — PARTIAL
D-118 orthographic top-down (2.5D path deleted, `camera2d.hpp:1-21`); project/unproject locked four ways; zoom envelope locked via scenarios. **The shipped follow-lerp path is untested** — the scenario harness snaps the camera (`main.cpp:3340-3344`), so the ×0.2 constant and its feel are pinned by nothing (R15). No shake exists; ScreenPulse substitutes deliberately.

### 4.8 Deterministic capture — COMPLETE for stills
Five staged scenes × {JSON, PNG@1920×1080, PNG@1366×768} with mandatory two-run byte equality, checked in under TASK-0070; Chronicles journey captures; side-by-side browser benchmark explicitly not-a-gate (R17). Gap: all timing evidence for FX is discrete scenario assertions; no multi-tick frame-sequence dump exists. Phase B adds `--ticks N` sequence dumps so TASK-0122 gets real animation goldens.

### 4.9 Tests — strong core, named holes
Inventory in R18. Holes that matter for the successor: camera lerp, Dust trail, orb/extraction pulse cadence, four FX lifetimes, sprite-mirror correctness, HUD clamp at non-960×600 resolutions, and the local session seam dropping `AttackTelegraphed`/`BuffExpired`/`ScionLost` while the local client bypasses the seam by reading raw sim events (`local_session.cpp:238` vs `main.cpp:1748-1759`) — the "renderers consume only PresentationEvents" contract is unproven for telegraphs in local mode.

---

## 5. Negative controls (SPEC requirement: ≥1 combat event with no proved visual timing/capture)

| id | Event | Status | Decisive evidence |
|----|-------|--------|-------------------|
| NC-01 | **critical hit** (`combat:hit.critical`) | computed + shipped, never consumed | `core.cpp:1953-1971` computes; `networking.cpp:1998-2005` ships; `remote_session.cpp:744-808` ignores it; numbers render uniform (`main.cpp:1633-1657`) |
| NC-02 | `ScionLost` | no presentation mapping/beat | emitted `core.cpp:893`; absent from `presentation_events.hpp:12-27`; dropped `local_session.cpp:238` |
| NC-03 | `BuffExpired` (war cry end) | no visual at all | `core.cpp:734`; seam drop `local_session.cpp:230-239`; aura only on apply `main.cpp:1770-1773` |
| NC-04 | pack materialization | no spawn event/FX; only arming is timed | `core.cpp:633-647, 837-842` |
| NC-05 | boss slam area telegraph | authored radius/position never reach pixels | R06 citations |

## 6. Additional audit findings (non-row)

1. **F-DEBUG:** leftover `fprintf(stderr, "[swing] tgt=... now=... next=... range-ok\n")` fires on **every player swing attempt** in tile-space combat — `native/src/core.cpp:1942`. Log noise on the hot path; flagged for the successor's owning task (this capsule may not edit source).
2. **F-AIM:** remote `AimIntent` is presentation-local — no envelope is sent (`remote_session.cpp:409-415`) while local aim is authoritative (`core.cpp:323-328`). Swing-direction parity between modes is unproven.
3. **F-CLOCK:** dual telegraph windup units (local 3 ticks vs remote `durationMs`/50 heuristic).
4. **F-SYNTH:** remote kill drops synthesized at the player's last tile, not the monster's (`remote_session.cpp:799-805`).
5. **F-SEAM:** `renderer/` is documentation-only; all presentation logic lives in `client/main.cpp` (4,682 lines). TASK-0122 should decide extract-now vs annotate-and-defer explicitly.
6. **F-PROV:** committed visual kit is TASK-0141/TASK-0144 work; the TASK-0147 polish wave is double-released and quarantined (`RELEASE.md`) — do not resume or copy it.

---

## 7. Phased successor boundaries (ready for TASK-0122 promotion)

- **Phase A — contracts (zero feel risk, do first):** map `ScionLost`/`BuffExpired`/spawn events through the session seam; carry telegraph `radius/position/facing` and hit `critical/style` end-to-end; unify windup units; delete monster-facing inversion. Each item lands with a session or scenario test. Unblocks: NC-01…NC-05.
- **Phase B — timing pins:** one named presentation-constants table for all FX ttl/pulse values; scenario assertions for Dust trail, orb pulse cadence, camera lerp; `--reference-scene --ticks N` frame-sequence dumps for animation goldens; HUD-clamp assertion at 1920×1080.
- **Phase C — procedural juice (D-113 vector-first, D-114 coherent, D-115 gated):** interpolation-only walk bob/attack lunge/dodge roll from `MovementStepInfo`; crit visual variant; spawn/dissolve beats; explicit decisions on hit-stop authority (sim freeze ticks vs presentation-only) and camera shake boundary.
- **Phase D — owner assets:** owner art decision gate (plates vs approved procedural skeletons); PNG plate pipeline already exists; honest art-status chip polices claims; TASK-0147 quarantine respected.

**Promotion note for the supervisor:** TASK-0122's DRAFT lists dependencies "TASK-0116 ACCEPTED, renderer/text strategy frozen, owner asset policy available or procedural fallback approved." This audit satisfies the first; Phases A–C above are implementable **now** without the owner gate (D-113 procedural-first), so the successor can be promoted immediately with Phase A as its first milestone and Phase D as its terminal gate.

---

## 8. Acceptance evidence

- `rg -n "animation|frame|facing|swing|telegraph|impact|death|dash|effect|particle|aura|orb|camera" native/client native/include native/src native/tests orchestration/benchmarks` — ran verbatim; **719 matching lines**; heaviest clusters: `native/client/main.cpp` 304, `native/tests/core_tests.cpp` 118, `native/src/core.cpp` 71, `presentation_state.cpp` 49, `remote_session.cpp` 38. Full output retained in the session tool log.
- `node -e "JSON.parse(...animation-vfx-matrix.json); console.log('animation/VFX matrix: PASS')"` — see REPORT.md run log; must print `animation/VFX matrix: PASS`.
- `git diff --check` and `git diff --name-only` — see REPORT.md; only this task folder may appear.
