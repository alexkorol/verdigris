# TASK-0102 — Skill system and binding gap audit (REPORT)

- Lane: `ox-pc-bb` · Model: `openrouter/stealth/ox-alpha`
- Branch/worktree: `worker/verdigris/pc/ox-pc-bb` @ `Z:/Code/.worktrees/verdigris/ox-pc-bb`
- Task base: `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of the audited head; audit reflects the integrated branch tip)
- Claim commit: `33a381b8` ("TASK-0102: claim (ox-pc-bb)", STATUS CLAIMED, pushed to origin)

## Executive summary

Mechanical, read-only audit of the native skill/action surface delivered as
`FINDINGS.md` plus machine-readable `captures/skill-matrix.json`. Headlines:

- Two authority paths exist. Path A (headless core,
  `Simulation::dispatch(Command::action_use)` →
  `resolve_actor_action`, `native/src/core.cpp:299-302,330,336-421`) gives
  full end-to-end authority to LMB melee, Space/RMB dash, and Q/E/R
  Thrust/Sweep/WarCry with catalog-exposed costs (10/15/20), shared/scaled
  cooldowns, cone/area targeting, telegraphed elite variants through the same
  pipeline, and exhaustive deterministic tests.
- Path B (native protocol server: `player:skill:trigger` →
  `WorldSimulation::start_player_attack` + `advance_combat`) has **no** skill
  resolution; every trigger reduces to a primary attack.
- **Negative control (required by SPEC): Q/E/R lack end-to-end authority on
  the protocol path.** Chain of evidence with citations is FINDINGS §9:
  client sends key `"skill"` (`native/client/remote_session.cpp:440-445`),
  server reads `"skillId"` (`native/src/networking.cpp:2507`); even when set,
  `active_skill_id_` only labels hits and toggles a legacy `"ability*"`
  damage branch (`networking.cpp:2071,2090-2093`) while resolution stamps
  every hit `skill_id="primary-attack"` (`core.cpp:1967`) and discards the
  passed power (`core.cpp:1881`); no wire feedback exists (no player
  resource/cooldown in protocol snapshots; `client_model.hpp:23-24` hardcodes
  50/50; no cooldown field), so remote quickbar availability renders from
  defaults. Secondary deviation: D-007 assigns RMB "weapon skill"
  (`orchestration/DECISIONS.md:33`), but RMB is bound to Dash identical to
  Space (`native/client/main.cpp:4104-4107`) and no seventh ActionType exists.
- Physical/action infrastructure that can proceed is separated from magic
  content blocked by OD-003 in FINDINGS §11 / matrix fields
  `can_proceed_content_neutral` vs `owner_blocked_OD_003`; successors must
  restate which paths stay owner-blocked.

No code or product files were modified; nothing was invented (no names,
effects, costs, or balance beyond quoting existing code). Resource capsule
respected: read-only survey, no ports opened (never 6500), no servers or
watch processes started.

## Approach

1. Product authority first: constitution control reference, D-007 decision
   text, OD-003 open decision, WIZARD Arcane Lattice boundary.
2. Full-text sweep (`rg "skill|primary|secondary|cooldown|cost|mana|LMB|RMB|
   Quickbar|keybind"`) over `native/include native/src native/client
   native/tests docs/product`, then line-level reads of every hit that bears
   on slots, definitions, costs/cooldowns, targeting, effects, wire events,
   client model, HUD, persistence, tests.
3. Findings written with path:line citations for every claim; JSON mirror
   captures the same content structurally; negative-control chain asserted
   step-by-step against source lines.

## Changed files

```
orchestration/tasks/TASK-0102-skill-system-gap-audit/FINDINGS.md           new
orchestration/tasks/TASK-0102-skill-system-gap-audit/captures/skill-matrix.json  new
orchestration/tasks/TASK-0102-skill-system-gap-audit/REPORT.md             new (this file)
orchestration/tasks/TASK-0102-skill-system-gap-audit/STATUS.md             CLAIMED -> REVIEW_REQUESTED
```

Nothing outside `orchestration/tasks/TASK-0102-skill-system-gap-audit/**`
was touched (owned_paths honored; forbidden_paths untouched).

## Public interfaces added/changed

None. Documentation/evidence only; no API, wire, schema, or behavior change.

## Test commands + outcomes

The four SPEC acceptance commands were executed literally from the repo root,
in order. Preparation disclosure: the three evidence files were newly
created, so `git add -N orchestration/tasks/TASK-0102-skill-system-gap-audit`
(intent-to-add) was applied before the diff commands so that `git diff` can
report new files; intent-to-add stages no content and was not part of the
acceptance set.

### 1) rg sweep — exit code 0 (202 matching lines)

Command:

```text
rg -n "skill|primary|secondary|cooldown|cost|mana|LMB|RMB|Quickbar|keybind" native/include native/src native/client native/tests docs/product
```

Literal transcript (complete, unedited):

```text
<<<RG_TRANSCRIPT>>>
```

### 2) JSON parse gate — exit code 0

Command:

```text
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0102-skill-system-gap-audit/captures/skill-matrix.json','utf8')); console.log('skill matrix: PASS')"
```

Output:

```text
skill matrix: PASS
```

### 3) whitespace gate — exit code 0

Command: `git diff --check`

Output: (empty — no whitespace/conflict-marker errors)

### 4) change-set gate — exit code 0

Command: `git diff --name-only`

Output:

```text
orchestration/tasks/TASK-0102-skill-system-gap-audit/FINDINGS.md
orchestration/tasks/TASK-0102-skill-system-gap-audit/captures/skill-matrix.json
```

Expected result met: only task evidence changes. (REPORT.md and the STATUS
flip are created by the closing REVIEW_REQUESTED commit itself, so they
cannot appear in a pre-commit working-tree diff; at acceptance time
`git status --short` listed exclusively
`?? orchestration/tasks/TASK-0102-skill-system-gap-audit/...` entries.)

## Manual verification

Not applicable to runtime behavior (no code changed). Performed instead:
every citation spot-read at the audited head; negative-control chain re-read
line-by-line; JSON validated mechanically (command 2 above).

## Commit SHAs

- Claim: `33a381b8` — pushed to `origin/worker/verdigris/pc/ox-pc-bb`.
- Review head: this closing commit (single commit containing FINDINGS.md,
  captures/skill-matrix.json, REPORT.md, STATUS → REVIEW_REQUESTED), pushed
  to the same worker branch. Branch history is append-only from `a5078d32`;
  no force-push, no program-branch writes.

## Deviations

- None from owned_paths/forbidden_paths or stop rules.
- `git add -N` used solely to make new evidence files visible to the literal
  `git diff` acceptance commands; disclosed above.

## Unresolved questions / risks

- Wire-contract direction (standardize on sender's `"skill"` vs reader's/
  browser-parity `"skillId"`) is an integration decision deliberately left
  to a successor task; this audit changes no contract.
- Incidental observation recorded in FINDINGS §12: production debug trace
  `fprintf(stderr,"[swing] …")` at `native/src/core.cpp:1942`; not repaired
  (outside owned paths).
- Risk: reviewers may treat Path B's primary-attack degradation as a bug to
  fix immediately; per SPEC stop rule ("stop before magic/content decisions")
  it is documented, not fixed here.

## Follow-ups (content-neutral successor candidates)

1. Unify `player:skill:trigger` payload key and route named actions into the
   existing resolver on the protocol server, reusing only frozen
   Thrust/Sweep/WarCry definitions.
2. Emit authoritative resource/cooldown/buff state on the protocol path;
   delete the client's hardcoded 50/50 default (`client_model.hpp:23-24`);
   add a copy-only `ClientPlayer.cooldown_ticks`.
3. Give RMB its distinct D-007 binding using existing ActionTypes only.
4. Protocol-path tests driving `"thrust"/"sweep"/"war-cry"` through
   `ProtocolSession` (zero coverage today).
5. Fold the quickbar cooldown-sweep normalizer (hardcoded max_ticks=30,
   `main.cpp:2462-2465`) into the presentation catalog.

Successor rule restated: all NEW skill content (names, effects, mana
semantics, Arcane Lattice integration) stays owner-blocked under OD-003;
only routing/feedback/binding infrastructure for the six existing ActionTypes
is cleared to proceed.
