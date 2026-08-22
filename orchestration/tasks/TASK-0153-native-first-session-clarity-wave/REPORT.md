---
task: TASK-0153
type: REPORT
worker: ox-pc-v
provider: openrouter
model: stealth/ox-alpha
base_commit: 4dfa4f1eac8853fcf82393e41abcf14419cff7b4
claim_commit: 8474ac5125d3725c7fd119ac907e907c14da75d6
implementation_commit: 8d386e24
branch: codex/TASK-0153-native-first-session-clarity-wave-ox-pc-v
completed_at: 2026-08-22 07:52 PDT
---

# TASK-0153 report — native first-session clarity wave (ox-pc-v)

## Executive summary

The first native expedition now tells the owner the truth on the normal HUD,
without F3 and without a second tutorial system:

1. **Authoritative expedition phase.** The objective strip names the slay
   objective (`objective: slay the wardens (N remain)`) while wardens remain
   and flips to the existing EXIT guidance only when the core itself reports
   `ExtractCarriedValue`. Locally this reads `Simulation::instance().phase`
   (the state behind the previously-dropped `ExpeditionPhaseChanged` event;
   the event now also reaches the event log via a new `event_label` case).
   Remotely — where this wire carries no phase event — the strip mirrors the
   equivalent already-authoritative session state (the server's living-foe
   snapshot); no client-side authority is invented, and scenes without an
   authoritative instance show no phase at all.
2. **Mode-aware extraction contract.** The EXIT instruction comes from one
   shared function (`extraction_action_hint`, presentation_state.cpp): local
   play says "press F there"; the remote owner path says "walk onto it" and
   can never render "press F" again. This kills accepted-TASK-0119 F-3, the
   active lie at the moment of maximum investment.
3. **Restrained always-available controls line.** A single dim reference
   line on the normal HUD — "WASD move | mouse aim | LMB attack |
   RMB/Space dash | Q E R skills | X take | Z names | I gear" — carries the
   essential combat/loot/gear answers including dash, the response to every
   enemy telegraph. It coexists with the scene at 960x600 and 1366x768
   (placed under the identity/objective row, clear of art/connection chips)
   and is asserted visible with the debug overlay disabled.
4. **Owner Esc contract (owner-reported defect addendum).** Escape closes an
   open gear/inventory pane first and keeps the client and session alive; a
   bare Escape requests application exit. The Win32 path and the scenario
   harness call the same production seams (`toggle_gear_overlay`,
   `handle_escape_key`, `ClientState::quit_requested`) — no test-only
   behavior exists.
5. **README bindings corrected.** Stale P/E/X documentation replaced with
   the implemented client bindings; the extraction line in the first-playable
   proof is mode-aware; the harness section names the new scenario.

Loose-guidance character preserved: the objective/status-chip language of
TASK-0142 was extended, no quest checklist, no narrative copy, no invented
names or rules, no server changes, no port 6500 use.

## Approach

- Extended the owned presentation seam rather than inventing new surfaces:
  `WorldView::expedition_phase` (`ExpeditionPhaseView`) populated from core
  phase state locally and from the authoritative foe snapshot remotely.
- One source of truth for the mode-aware exit phrase so strip copy and
  scenario assertions cannot drift apart.
- Input handling refactor kept minimal: the 'I' inline block became
  `toggle_gear_overlay`; Esc decision-making moved into `handle_escape_key`;
  the window procedure posts the quit only when `quit_requested` is set.
- New deterministic scenario `first-session-clarity` proves all contracts
  through the real dispatch/ingest/present pipeline. Local phase transition:
  `drive_to_extraction_phase` mirrors the accepted core-test discipline
  (core_tests.cpp drive_expedition) by bringing each living warden into
  forward reach for a real pipeline kill; remote determinism uses the same
  accepted dev-envelope shortcut class as chronicles-gate-b (`dev:clear-floor`)
  to reach the cleared floor, after which the production strip logic runs on
  real snapshots.

## Changed files

- `native/client/main.cpp` — phase/mode-aware objective strip; controls
  hint; quit-request flag + shared input seams; window_proc Esc/I wiring;
  `event_label` case for `ExpeditionPhaseChanged`; loot-to-bank reaches the
  genuine flip before its preserved strip assertions; new
  `first-session-clarity` scenario registered in `run_scenarios`.
- `native/client/presentation_state.hpp` — `ExpeditionPhaseView`,
  `WorldView::expedition_phase`, `extraction_action_hint` declaration.
- `native/client/presentation_state.cpp` — phase sync from
  `Simulation::instance().phase`; remote derivation from the authoritative
  model snapshot; `extraction_action_hint` implementation.
- `native/README.md` — binding corrections as above.
- `orchestration/tasks/TASK-0153-native-first-session-clarity-wave/**` —
  STATUS.md, REPORT.md.

## Public interfaces added/changed

- Added (client-local, presentation-only): `ExpeditionPhaseView`,
  `WorldView::expedition_phase`, `extraction_action_hint(bool)`.
- Changed (behavioral, owner-visible): objective-strip text contract
  (phase-aware, mode-aware); Esc dismissal order; gear toggle is a named
  production function.
- No simulation, networking-wire, persistence, or server interface changed.
  Forbidden paths untouched (`native/src/**`, `native/include/**`,
  `native/tests/**`, server/src/docs-product/docs-rebuild, TASK-0119 folder).

## Test commands + outcomes (literal SPEC gates)

1. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1
   -RunTests -RunClientScenarios`
   → build OK (pre-existing warnings only); legacy denylist PASS; core /
   networking / camera2d tests PASS; session tests PASS (local,
   remote-negative, remote, journey, reconnect, replaced, render-list);
   all nine scenarios PASS with 0 failures: move-and-camera, first-fight,
   loot-to-bank, telegraph-dodge, combat-juice, remote-render-list,
   zoom-invariance, chronicles-gate-b, first-session-clarity.
2. `native/build/verdigris_client.exe --scenario first-session-clarity`
   → exit 0; 20 checks ok, incl. both negative controls ("remote HUD never
   says press F" before and after clearing) and the four Esc-contract checks.
3. `git diff --check` → clean (exit 0).
4. `git diff --name-only` → only owned paths (one unowned TASK-0145 evidence
   PNG regenerated by the gate-b run was restored before commit).

Preservation evidence: every pre-existing scenario passed unchanged in the
same gate run; loot-to-bank's original assertion labels are byte-identical —
the journey now honestly finishes its slay leg before asserting the
carry-to-exit strip it always meant to prove.

## Manual verification

Interactive manual play was not driven by this worker (resource capsule
discipline; deterministic scenarios are this repo's accepted proof standard
per TASK-0145/TASK-0119 reviews). All behavioral claims are backed by the
in-tree scenario run quoted above, which drives the real Win32-independent
paint path, real protocol sessions on the shared 6580–6599 capsule, and the
real simulation. Port 6500 was never touched; worker ports 7040-7059 unused
(no manual servers launched).

## Process notes (required disclosures)

- **Activation SLA breach:** the routed claim initially exceeded the five-
  minute push deadline during mandated document reading and codebase study;
  a supervisor recovery ordered an immediate STATUS-only claim commit, which
  landed as `8474ac51` and was verified on origin before implementation began.
- **Hook override:** this isolated worktree intentionally has no
  node_modules, so the shared Yorkie hook cannot run. Per explicit owner
  authorization, every commit used the per-command nonpersistent override
  `git -c core.hooksPath=Z:/Code/.fleet/no-hooks commit ...`. Git config was
  not modified and no browser dependencies were installed.

## Deviations

- None material. Two implementation notes for the reviewer:
  1. loot-to-bank gained a single `drive_to_extraction_phase(state)` call so
     its unchanged assertions execute against the genuinely-authoritative
     post-flip state (its old expectation encoded the pre-phase strip).
  2. The remote phase view derives from the server's authoritative living-
     foe snapshot because this wire has no phase event; between ~4Hz
     snapshots a just-killed last warden may leave the slay line up briefly.
     Truthful per-snapshot; flagged as a future wire-enhancement candidate
     (owner decision), not worked around client-side.

## Unresolved questions

- Should the server eventually publish an explicit expedition-phase field
  (e.g., alongside `wardenDead` metadata) so the remote strip can mirror a
  dedicated signal instead of the foe snapshot? Owner/architect decision;
  current behavior remains truthful either way.

## Risks

- Low. Strip copy changes could surprise downstream string-matching tools;
  all in-repo consumers (scenarios) pass. The slay wording reuses established
  core vocabulary ("slay-wardens" tokens, Warden naming already canonical in
  core/networking), so no lore was introduced.

## Follow-ups

- TASK-0119 items still open and unowned here: progression surfacing
  (level-ups/passive tree read-only), front-door reconnect action, stake-
  naming quit confirm, awaiting-respawn explanation, placeholder loot labels,
  name-input seam (owner decision required).
