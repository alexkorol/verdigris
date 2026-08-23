# TASK-0101 REPORT — Combat depth and feel gap audit

- worker: ox-pc-ai (provider openrouter, model `stealth/ox-alpha`, opencode harness)
- coordinator: codex
- branch: `codex/TASK-0101-combat-depth-gap-audit-ox-pc-ai` (pushed; see SHAs)
- base: `610a240e1e4bdfacfd77bec49e36be945a1ced13` (verified equal to HEAD at
  claim time and an ancestor of pushed `origin/codex/native-reconstitution`)
- resource capsule honored: read-only analysis, no play server started, port
  6500 never touched

## Executive summary

Audited the constitution combat vocabulary (VERDIGRIS_CONSTITUTION.md 87–123)
against both authoritative native surfaces. Delivered `FINDINGS.md` (narrative,
source-cited) and `captures/combat-matrix.json` (deterministic matrix, parses
PASS). Result: swings/thrusts/war cries present and test-locked; slams exist
only as an enemy-only boss mechanic on the tile-space path; leaps, guarded
actions, combos, and magic are absent (grep-proved); ranged attacks are
authored data with zero behavior; telegraphs and damage resolution diverge
between the two surfaces; equipment effects are mostly decorative data;
gold auto-pickup is the one missing control contract. Six ranked successors
are specified with exact paths, dependencies, negative controls, locking
tests, and owner-visible outcomes; all balance/design choices stop at the
owner boundary.

## Approach

Read the constitution, SPEC, PROTOCOL, RUN_STATUS top; ran the repository
preflight; read both authoritative simulation surfaces (`Simulation`,
`WorldSimulation`), their tests (`core_tests`, `session_tests`,
`presentation_events_tests`, client scenarios), client presentation
(`main.cpp`, `presentation_state.cpp`, `render_list.hpp`,
`presentation_events.hpp`), the audio cue seam (`event_cues.*`), and wire
serialization (`networking.cpp`, `remote_session.cpp`). Ran literal greps for
each constitution family including proof-of-absence controls. No file outside
the owned task folder was modified.

## Changed files (scope proof)

```
orchestration/tasks/TASK-0101-combat-depth-gap-audit/STATUS.md   (claim + review-request states)
orchestration/tasks/TASK-0101-combat-depth-gap-audit/FINDINGS.md (new)
orchestration/tasks/TASK-0101-combat-depth-gap-audit/REPORT.md   (new, this file)
orchestration/tasks/TASK-0101-combat-depth-gap-audit/captures/combat-matrix.json (new)
```

`git diff --name-only` / `git status --short` before each commit showed only
these owned paths. No runtime/source files touched; no public interfaces added
or changed (pure audit packet).

## Acceptance commands and outcomes (exact)

```powershell
rg -n "attack|damage|hit|telegraph|dodge|dash|guard|slam|thrust|combo|war cry|effect" native/include native/src native/client native/tests docs/product/VERDIGRIS_CONSTITUTION.md
# exit 0; full output preserved at Z:\Code\.fleet\tmp\ox-pc-ai\acceptance_rg.txt

node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0101-combat-depth-gap-audit/captures/combat-matrix.json','utf8')); console.log('combat matrix: PASS')"
# output: "combat matrix: PASS"; exit 0

git diff --check        # exit 0 at first submission (evidence still untracked)
git diff --name-only    # only owned task paths (untracked evidence at run time)
```

Revision 1 (architect REVISE in program commit
`1a434371b281494d3f5aa6bdc3e50447e1814855`) added the final committed-range
gate and the readable-successor correction:

```powershell
git diff --check 610a240e..HEAD   # exit 0 over the committed artifact range
```

The two trailing-whitespace defects the review found (FINDINGS.md W2/W4
owned-path lines, committed lines 128 and 156) are removed in this revision.
W1 no longer routes invisible ranged damage: it now owns the readable
telegraph/hit-beat seam (`core.hpp` event fields,
`presentation_state.cpp`, `render_list.hpp`, `main.cpp`,
`session_tests.cpp`), reuses only shipped render ops and constants, and adds
a deterministic session-level lock requiring every ranged hit to be preceded
by a Telegraph op and land as a Damage/Impact op; if readability is split out,
GAP-TELEGRAPH-CATALOG routes first as an explicit dependency.

Negative control (SPEC): designated absent family **combos** —

```powershell
rg -n -i "combo" native/include native/src native/client native/tests
# exit 1 (zero matches) — recorded in matrix negative_control block
rg -n -i "\bleap(s|ing)?\b" native/include native/src
# exit 1 (zero matches) — supporting absence
```

The matrix therefore marks combos/leaps/guarded-actions/magic as ABSENT rather
than treating generic attack parity as coverage.

## Manual verification

None runnable under the capsule: building or driving a play session would
exceed read-only scope. Visual/audio claims cite shipped scenario assertions
and test locks instead of fresh captures; this is stated honestly in
FINDINGS.md ("Honest gaps").

## Commit SHAs (this lane)

| commit | meaning |
|---|---|
| `9ca61eb7` | claim (STATUS.md, Markdown bullets) |
| `b083b58b` | supervisor-directed YAML STATUS correction |
| `aeab40fe` | FINDINGS.md + captures/combat-matrix.json evidence |
| (head) | REPORT.md + REVIEW_REQUESTED STATUS flip (this commit) |

Local HEAD was verified equal to `origin/codex/TASK-0101-...` after every push
(`git ls-remote` comparison).

## Deviations

- The first claim commit used Markdown bullets that the machine board could
  not parse; corrected in `b083b58b` per supervisor correction memo (new
  commit, no amend).
- Three JSON objects in the matrix initially lacked a `"note"` key and failed
  the parse acceptance command twice; repaired and re-run to PASS before any
  commit of the captures file. Caught by the literal acceptance command, not
  silently fixed.
- Session stopped once after the claim with analysis uncommitted; resumed in
  the same session per supervisor recovery memo; no work was redone or lost.
- Revision 1: first submission ran `git diff --check` while evidence files
  were still untracked (they were committed moments later in `aeab40fe`), so
  the gate had not inspected the final artifact; the review found two
  trailing-whitespace defects in FINDINGS.md. Fixed here; the corrected gate
  is `git diff --check 610a240e..HEAD`, exit 0, recorded above. W1 was also
  rewritten to carry its readable presentation contract per the REVISE.

## Risks / unresolved questions

- Line-number citations are exact at base `610a240e`; they will drift if the
  cited files change (matrix carries the frozen base_commit for this reason).
- Successors W1/W4 eventually need owner-supplied values; both are routable
  with placeholder reuse of already-authored constants, flagged in the matrix.
- The two-surface coherence gaps (telegraph catalog, damage formula) touch
  files other lanes may own; integration sequencing is the coordinator's call.

## Follow-ups (immediate highest-value successor)

**W1 — realize ranged behaviour in tile-space combat, readably**: own
`native/src/core.cpp` (advance_combat monster branch) +
`native/tests/core_tests.cpp` + the readable-beat seam (`core.hpp`,
`presentation_state.cpp`, `render_list.hpp`, `main.cpp`,
`session_tests.cpp`); negative control keeps melee streams byte-identical;
locks prove a `ranged` monster damages from beyond contact while its melee
twin does not, with deterministic replay, AND that every ranged hit is
preceded by a client-visible Telegraph op and lands as a Damage/Impact op
(session-transcript asserted). If readability is split out, W2 routes first
as an explicit dependency. Owner-visible outcome: packs read as composed
encounters and ranged attacks are warned-then-visible. Full W2–W6
specifications in FINDINGS.md.
