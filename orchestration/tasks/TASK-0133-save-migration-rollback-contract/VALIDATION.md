# TASK-0133 validation

Validation record for `save-migration-contract.json` and
`fixtures/negative-cases.json`. Every acceptance command from `SPEC.md` was
run literally from the repository root on worker branch
`codex/TASK-0133-save-migration-rollback-contract-ox-pc-h`.

## How to evaluate

Run the five commands in `SPEC.md` § Acceptance commands verbatim. Gates 1–4
are deterministic file/git checks; gate 3 doubles as the seam-inventory
evidence source for the contract's `seam_map`.

## Negative-case coverage matrix

| Expected error | Case | Injected fault | Detection point | Contract clause |
| --- | --- | --- | --- | --- |
| UNKNOWN_SOURCE_VERSION | NEG-01 | Store matches no declared version or shape probe | preflight P-04 | `source_version.detection_order` rule 3 |
| BACKUP_FAILED | NEG-02 | Copy corruption / sha256 mismatch / parse-count mismatch | backup B-02/B-03 | `backup.steps`, `backup.fail_error` |
| MIGRATION_FAILED | NEG-03 | Transformer throws mid-step | migration step execution | `migration.ordering_rules` |
| VERIFY_FAILED | NEG-04 | Migrated output drops item + regresses level | verification V-02/V-03 | `verification.checks_per_store` |
| NON_IDEMPOTENT | NEG-05 | Re-run over matching migration token | idempotence token check | `idempotence.rules` |
| ROLLBACK_FAILED | NEG-06 | Backup corrupts after verification | rollback R-02 | `rollback.escalation` |
| DATA_LOSS | NEG-07 | Undeclared durable-item drop (heirloom UUID vanishes) | data-loss counters | `data_loss_detection.verdict` |
| LOCK_HELD (extra) | NEG-08 | Foreign lock present | preflight P-02 | `preflight.mandatory_steps` |

The seven SPEC-required errors are covered by NEG-01..NEG-07; NEG-08 is an
extra guard referenced by preflight step P-02. All fixtures are synthetic and
disposable; none references real profile data.

## Gate execution record

Executed 2026-08-22 UTC from repository root; outputs verbatim.

1. Contract key check:

   ```text
   save migration contract: PASS
   GATE1-EXIT:0
   ```

2. Negative-case check:

   ```text
   save migration negatives: PASS
   GATE2-EXIT:0
   ```

3. Seam inventory rg (unabridged stdout preserved at
   `captures/gate3-seam-inventory.txt`, 914 matched lines):

   ```text
   GATE3-EXIT:0
   ```

4. Whitespace check (`git diff --check` with all task files staged):

   ```text
   (no output)
   GATE4-EXIT:0
   ```

5. Ownership scope (`git diff --name-only <base>..HEAD`) is recorded in
   `REPORT.md`; every listed path lies under
   `orchestration/tasks/TASK-0133-save-migration-rollback-contract/**`
   per `owned_paths`.

## Revision r2 gate rerun (post-review owner-authority correction)

Architect REVIEW verdict `REVISE` (reviewed head `b44ab0ab`) required exactly
one semantic correction: `target_version.current_target` must be
null/`OWNER_PENDING`, dropping the claim that native-snapshot-v1 is "the
single ratified target format today"; native-snapshot-v1 is preserved as an
observed candidate with citations, not a chosen cross-estate destination.
Applied to `save-migration-contract.json` § `target_version`; no other
section, fixture id, or source path changed. All five literal SPEC commands
were rerun from the repository root on this branch:

```text
save migration contract: PASS
GATE1-EXIT:0
```

```text
save migration negatives: PASS
GATE2-EXIT:0
```

Gate 3 rg exit 0 (959 matched lines at this head; the r1 transcript at
`captures/gate3-seam-inventory.txt` remains the unabridged r1 evidence
capture and was not regenerated for this prose-only-plus-contract-field
revision). Gate 4 `git diff --check` clean, exit 0. Gate 5 base-to-head path
list exit 0 — unchanged except this task folder's own files; the non-folder
entries remain the architect routing commit `b3599c80`'s introductions, not
worker-authored.

## Manual verification

- No persistent user data was read, copied, mutated, or created. The only
  writes made by this task live inside the task folder. No server, browser,
  or native process was started; port 6500 untouched.
- Seam facts cited in `save-migration-contract.json` were taken from files
  read directly at routed HEAD `b3599c80122d09cd0685ae96830990cc5bada5cf`
  (guest-save-store.js, player-persistence.js, chronicles-persistence.md,
  verdigris-authority.js, native/persistence/README.md) plus prior accepted
  audit tasks (TASK-0005, TASK-0032, TASK-0105, TASK-0081).
- Unresolved mappings (U-01..U-04) are explicitly marked OPEN and no owner
  compatibility policy is selected anywhere in this packet.

## Honest limitations

- This is an architecture/evidence contract: no transformer code exists, so
  the negative cases define required behavior rather than execute it. Any
  future implementation task must cite these case ids in its tests.
- Gate 3 matches the word `migration` inside orchestration prose (including
  this folder); its value here is the seam transcript, not a pass/fail signal
  beyond exit code 0.
