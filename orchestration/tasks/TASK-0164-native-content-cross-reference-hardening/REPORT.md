# REPORT — TASK-0164 native content seed cross-reference hardening

- Worker lane: `ox-pc-ag` (coordinator `codex`, `openrouter`/`stealth/ox-alpha`,
  OpenCode CLI 1.18.21, variant max)
- Claim commit: `dfd40ee2a4e0bdbaf7b5b6354ef5b51b841b036c` (clean replacement
  claim after one owner-directed same-session activation repair of the
  `coordinator` frontmatter field; original claim commit `4c6d09ff`)
- Implementation commit: `342ed4f5d80e165c3464829ac7717baf708ac0b5`
- Base commit: `282d62a06244fb7304f12443f162f5661b701780`
- Predecessor released claim (not copied, not consulted):
  `949508f5c2d1cb74353f57eed61b1a5c2dd392d9`
- Branch: `codex/TASK-0164-native-content-cross-reference-hardening-ox-pc-ag-r2`
- Clone/worktree: `Z:\Code\.worktrees\verdigris\ox-pc-ag`
- Temp evidence: `Z:\Code\.fleet\tmp\ox-pc-ag` only
- Status at write time: REVIEW_REQUESTED

## Executive summary

Hardened the accepted TASK-0151 content validator so the zone/encounter seed
corpus is checked as a closed, deterministic reference graph, and extended the
negative suite from 23 to 27 checks with isolated fixtures for every SPEC
category. New hardening: reference targets that resolve to an id of the wrong
entity collection now fail with a targeted `E_REFERENCE_TYPE_MISMATCH`
diagnostic instead of a generic unknown-id error; seeded encounters anchored to
zones unreachable from the graph root now fail with a targeted
`E_UNREACHABLE_ENCOUNTER` error (zone-level unreachability remains the accepted
non-fatal `W_UNREACHABLE_ZONE` warning — nothing was downgraded). The CLI also
accepts the SPEC-literal positional invocation
`validate_content.py <zones.json> <encounters.json>` additively; the previous
`--root` interface is unchanged and remains the default. All acceptance gates
pass twice with byte-identical output; positive seeds still validate clean.

## Approach

- Implemented independently from `native/content/schema.json` v1,
  `native/content/seeds/**`, and the TASK-0164 SPEC. The quarantined ox-pc-ae
  worktree was never opened or read.
- "Compatible tier/type fields" was mapped onto what accepted schema v1 can
  express without an owner decision: schema-v1 carries no numeric tier concept,
  so type compatibility is enforced as entity-kind compatibility of reference
  targets (a `reference:zone`/exit target must either exist in the declared
  target collection or be reported as a type mismatch when it exists in another
  collection). No content rule, enum, or relation outside schema v1 was
  invented; see Interpretation note below.
- Type-mismatch detection uses a deterministic first-definition ownership map
  (`collect_id_owners`: first item in index order wins, pattern-valid ids only),
  so `E_REFERENCE_TYPE_MISMATCH` names both the offending id and the collection
  that actually defines it.
- Encounter reachability reuses one shared BFS (`reachable_zones`, sorted
  neighbor iteration, lexicographically smallest zone id as root) with the
  existing zone warning; an encounter errors only when its zone reference
  itself resolved to a valid but unreachable zone, avoiding duplicate
  diagnostics for already-reported dangling references.
- Determinism preserved by construction: all diagnostic tuples are sorted at
  emit time, dict iteration in output paths is `sorted()`, no timestamps or
  environment dependence; the suite byte-compares every case across two runs.
- No accepted seed, schema, README, runtime, client, server, CMake, or
  dependency file was touched; scope verified via `git diff --name-only`.

## Changed files

All inside owned paths:

```text
native/content/validate_content.py          hardened + positional seed-file args
native/content/tests/run_negative_tests.py  23 -> 27 isolated negative cases
orchestration/tasks/TASK-0164-.../STATUS.md claim + REVIEW_REQUESTED transitions
orchestration/tasks/TASK-0164-.../REPORT.md this report
```

## Public interfaces added/changed

- Added CLI positionals: `seed_files...` (optional, repeatable); each must be a
  declared `seed_files` path under `--root`; undeclared paths exit 2 with a
  usage error on stderr. With both seeds supplied, output is identical to the
  argument-less full validation.
- Added stable diagnostic codes (additive; consumers match codes, not prose):
  - `E_REFERENCE_TYPE_MISMATCH` — reference resolves to an id defined in a
    different entity collection.
  - `E_UNREACHABLE_ENCOUNTER` — encounter anchors a valid but unreachable zone
    (fatal; exit 1).
- Exit contract unchanged: `0` valid, `1` validation failures, `2` usage.
- Summary line format unchanged.

## Test commands and outcomes

Environment: Windows, PowerShell 7, Python 3.12.6. Literal transcripts
(evidence copies in `Z:\Code\.fleet\tmp\ox-pc-ag\`):

Positive gate, run twice, byte-compared:

```text
=== run 1 / run 2 ===
OK schema=1 errors=0 warnings=0 encounter=3 zone=5
exit=0 / exit=0 / identical=True
```

Negative gate, run twice, exit 0 both, stdout byte-identical:

```text
PASS positive_control
PASS determinism_double_run
PASS unknown_visual_role (E_UNKNOWN_ROLE)
PASS unknown_visual_slot (E_UNKNOWN_SLOT)
PASS duplicate_zone_id (E_DUPLICATE_ID)
PASS duplicate_encounter_id (E_DUPLICATE_ID)
PASS cross_collection_id_collision (E_DUPLICATE_ID)
PASS exit_to_unknown_zone (E_UNKNOWN_ZONE_REF)
PASS encounter_references_unknown_zone (E_UNKNOWN_ZONE_REF)
PASS exit_to_encounter_id_type_mismatch (E_REFERENCE_TYPE_MISMATCH)
PASS encounter_zone_type_mismatch (E_REFERENCE_TYPE_MISMATCH)
PASS unreachable_encounter_zone (E_UNREACHABLE_ENCOUNTER)
PASS string_schema_version_linkage (E_SCHEMA_VERSION)
PASS unknown_zone_template (E_UNKNOWN_TEMPLATE)
PASS unknown_zone_layout (E_UNKNOWN_LAYOUT)
PASS unknown_exit_kind (E_UNKNOWN_EXIT_KIND)
PASS unknown_encounter_family (E_UNKNOWN_FAMILY)
PASS bad_seed_schema_version (E_SCHEMA_VERSION)
PASS wrong_envelope_kind (E_FILE_KIND)
PASS missing_required_field (E_MISSING_FIELD)
PASS unknown_item_field (E_UNKNOWN_FIELD)
PASS malformed_identifier (E_ID_FORMAT)
PASS empty_display_name (E_NAME_LENGTH)
PASS duplicate_exit_edge (E_DUPLICATE_EXIT)
PASS unknown_envelope_field (E_UNKNOWN_FIELD)
PASS malformed_json (E_JSON_PARSE)
PASS missing_seed_file (E_FILE_MISSING)
checks=27 failures=0
NEGATIVE SUITE PASS
```

Targeted-diagnostic samples (isolated fixture roots):

```text
ERROR seeds/encounters.json:items[0].zone E_REFERENCE_TYPE_MISMATCH: 'zone' references 'example-encounter-two' which is defined as a encounter id, not a 'zone' id
FAIL errors=1 warnings=0

ERROR seeds/encounters.json:items[1].zone E_UNREACHABLE_ENCOUNTER: encounter 'example-encounter-two' anchors zone 'example-hall-two' which is not reachable from graph root 'example-field-one'
ERROR seeds/encounters.json:items[2].zone E_UNREACHABLE_ENCOUNTER: encounter 'example-encounter-three' anchors zone 'example-mire-one' which is not reachable from graph root 'example-field-one'
FAIL errors=2 warnings=3
```

Whitespace/scope gate:

```text
git diff --check      -> exit 0 (no output)
git diff --name-only  -> native/content/tests/run_negative_tests.py, native/content/validate_content.py (implementation commit staging)
```

## Manual verification

- Ran the SPEC-literal command exactly:
  `python native/content/validate_content.py native/content/seeds/zones.json native/content/seeds/encounters.json`
  → exit 0, summary byte-for-byte stable across runs (this invocation required
  the additive positional-args support described above).
- Confirmed the unreachable-zone warning contract is intact: zone warnings
  remain non-fatal; only encounter anchoring became fatal.
- Confirmed each new fixture fails for exactly its targeted code and that the
  suite's internal double-run comparison passes for every case.
- No listener started; ports 7240-7259 unused; port 6500 untouched; transient
  files confined to `Z:\Code\.fleet\tmp\ox-pc-ag`.

## Deviations

- None from SPEC acceptance commands; the literal positional invocation is now
  supported natively rather than rewritten.
- Implementation debugging note: two intermediate defects (an inverted
  selection map and a lost indent level muting `check_reference_fields`) were
  introduced and caught by the negative gate before any commit; the committed
  head is the green state.

## Interpretation note for the architect

The SPEC phrase "compatible tier/type fields" has no numeric-tier analogue in
accepted schema v1 (forbidden to edit). It was implemented strictly within the
accepted schema as entity-kind/type compatibility of every seed reference plus
enum membership of kind fields, per RUN_STATUS's contribution line ("rejects
dangling zone/encounter seed references"). If the architect intended a
family↔template compatibility rule (e.g., which encounter families may anchor
in which zone templates), that relation is not expressible in schema v1 and
would require an owner/architect schema decision; flagging rather than
inventing it.

## Unresolved questions

- None blocking; the interpretation note above is advisory.

## Risks

- `E_UNREACHABLE_ENCOUNTER` makes previously-warning-only situations fatal once
  encounters anchor orphaned zones; this is intentional hardening and matches
  "no unreachable seeded encounter".
- First-definition-wins ownership reporting may name a later-corrected item if
  authors duplicate ids across collections; `E_DUPLICATE_ID` fires alongside so
  the pair is always visible together.

## Follow-ups

- Future loader task maps validated seeds into the simulation seam unchanged.
- If the architect defines tier semantics, extend schema v2 deliberately per
  the versioning policy; validator changes remain schema-driven.
