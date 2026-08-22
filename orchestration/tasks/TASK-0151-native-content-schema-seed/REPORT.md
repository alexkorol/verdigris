# REPORT — TASK-0151 native content schema seed

- Worker lane: `ox-pc-ab` (ox-alpha, `openrouter`/`stealth/ox-alpha`, OpenCode
  CLI 1.18.21, variant max)
- Claim commit: `57627ca3130064330e1ff89672c61632fb8a71c0`
- Implementation commit: `0707b819e16d1996ca29b933b61f337d4c37c323`
- Base commit: `c2b814488278f4f093e754cf695ea9ed749d81fb`
- Branch: `codex/TASK-0151-native-content-schema-seed-ox-pc-ab`
- Clone/worktree: `Z:\Code\.worktrees\verdigris\ox-pc-ab`
- Status at write time: REVIEW_REQUESTED

## Executive summary

Replaced the prose-only `native/content/README.md` placeholder with a working,
deterministic, versioned, content-neutral content seam: a JSON schema
descriptor (`schema.json`, version 1), two synthetic seed files (5 example
zones, 3 example encounters), a dependency-free Python 3 stdlib validator CLI,
and an automated negative-test suite with 23 checks. All acceptance gates pass;
no production lore or balance values were introduced; all writes stayed inside
owned paths.

## Approach

- Schema-driven validation: the validator reads enums, entity field
  definitions, composite types, identifier/display-name rules, reference
  targets, and the seed-file manifest from `schema.json`. Adding fields or
  enum members is a schema-only change; validator code does not hardcode the
  content shape.
- Determinism by construction: diagnostics are tuples sorted by
  `(file, path, code, message)`; all dict iteration in output paths goes
  through `sorted()`; messages embed values via stable JSON serialization; no
  timestamps, locale, environment, or filesystem-order dependence. The
  negative suite re-runs every case twice and byte-compares output.
- Content neutrality: seeds use `example-*` identifiers and generic display
  names ("Example Hall One" ...). Zone templates/layouts reuse only the
  identifier families already accepted by `verdigris::is_zone_template` /
  `is_zone_layout` (`crypt|dungeon|grove|marsh|wilds`,
  `clearings|gauntlet|warren`). Encounter families are structural only
  (`skirmish|elite|warden`) with no numbers anywhere.
- Graph integrity: exit targets must resolve to committed zone ids
  (`E_UNKNOWN_ZONE_REF`), duplicate edges within one zone are rejected
  (`E_DUPLICATE_EXIT`), and unreachable zones produce a deterministic
  non-fatal warning (`W_UNREACHABLE_ZONE`) from a BFS rooted at the
  lexicographically smallest zone id.

## Changed files

All inside owned paths:

```text
native/content/README.md                  rewritten: seam documentation + commands
native/content/schema.json                new: versioned schema descriptor (v1)
native/content/seeds/zones.json           new: 5 synthetic example zones
native/content/seeds/encounters.json      new: 3 synthetic example encounters
native/content/validate_content.py        new: dependency-free validator CLI
native/content/tests/run_negative_tests.py new: positive+determinism+negative suite
```

`git status` after staging showed exactly those six paths and nothing else.
No file outside owned paths was created, modified, or reverted.

## Public interfaces added

- Command: `python native/content/validate_content.py [--root DIR] [--quiet]`
  (documented in `native/content/README.md`; Python 3 stdlib only).
- Command: `python native/content/tests/run_negative_tests.py`.
- Stable diagnostic codes: `E_DUPLICATE_EXIT`, `E_DUPLICATE_ID`,
  `E_FILE_KIND`, `E_FILE_MISSING`, `E_ID_FORMAT`, `E_JSON_PARSE`,
  `E_MISSING_FIELD`, `E_NAME_LENGTH`, `E_BAD_TYPE`, `E_SCHEMA_INVALID`,
  `E_SCHEMA_VERSION`, `E_UNKNOWN_ENVELOPE` n/a, `E_UNKNOWN_ENUM_MEMBER`
  fallback, `E_UNKNOWN_FAMILY`, `E_UNKNOWN_FIELD`, `E_UNKNOWN_EXIT_KIND`,
  `E_UNKNOWN_LAYOUT`, `E_UNKNOWN_REFERENCE` (reserved), `E_UNKNOWN_ROLE`,
  `E_UNKNOWN_SLOT`, `E_UNKNOWN_TEMPLATE`, `E_UNKNOWN_ZONE_REF`,
  `W_UNREACHABLE_ZONE`.
- Exit contract: `0` valid, `1` validation failures, `2` argparse usage.
- Summary line format: `OK schema=<v> errors=0 warnings=<n> <kind>=<count>...`
  or `FAIL errors=<n> warnings=<m>`.

No C++ or other production code was touched; nothing loads this directory yet.

## Test commands and outcomes

Environment: Windows, PowerShell 7, Python 3.12.6. Literal transcripts:

Positive gate (accepts committed seed), run three times plus quiet mode:

```text
=== GATE positive run 1 ===
OK schema=1 errors=0 warnings=0 encounter=3 zone=5
exit=0
=== GATE positive run 2 ===
OK schema=1 errors=0 warnings=0 encounter=3 zone=5
exit=0
=== GATE determinism byte-compare of repeated runs ===
identical=True
=== GATE quiet mode ===
OK schema=1 errors=0 warnings=0 encounter=3 zone=5
exit=0
=== interpreter ===
Python 3.12.6
```

Negative gate (rejects unknown roles/duplicate IDs/invalid graph references;
23/23 checks, each case also byte-compared across two runs):

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
checks=23 failures=0
NEGATIVE SUITE PASS
exit=0
```

Whitespace/scope gate:

```text
git diff --cached --check   -> exit 0
git diff --check            -> exit 0
```

## Manual verification

- Ran the validator from the repository root via exactly the README-documented
  command; confirmed the documented expected summary line byte-for-byte.
- Confirmed `--quiet` prints only the summary line.
- Inspected the staged diff scope (`git status --short`): only
  `native/content/**` entries; task-folder files are untracked at that point
  and committed separately in this folder.
- No listener was started; no port was bound; port 6500 untouched.

## Deviations

- None from SPEC acceptance. Note: the SPEC frontmatter lists
  `base_commit 060c1151...` while the launch packet and RUN_STATUS route this
  lane at `c2b81448...` ("Queue restock commit"); I followed the routed base as
  instructed by the packet ("prove ... exact routed base"), which is also the
  provisioned HEAD. Flagging for the architect in case the SPEC header should
  be restamped on integration.
- Early in implementation the first validator draft had ordering defects
  (counts used before initialization, a module-global file map). Both were
  fixed before any commit; the committed head is the clean rewrite. The gates
  themselves surfaced two schema/validator contract mismatches
  (`slot_role_map` composite shape, plural vs singular seed-file kinds) which
  were corrected and are covered by the passing transcripts above.

## Unresolved questions

- None blocking. The base-commit discrepancy above is recorded for review.

## Risks

- The validator intentionally rejects unknown fields everywhere
  (strict forward compatibility); future schema extensions must bump
  deliberately per the versioning policy in `native/content/README.md`.
- Diagnostics messages are human-readable and may be reworded; codes are the
  stable contract. Consumers should match codes, not prose.

## Follow-ups

- Future loader task can map validated seeds into the simulation seam without
  changing this directory's contract.
- When owner-approved real content exists, replace seeds via the approval
  matrix; the validator needs no changes for data swaps that fit schema v1.
