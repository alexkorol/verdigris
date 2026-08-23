---
task: TASK-0085
title: Live denylist-exception evidence packet
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 4474b54e
reviewed_at: 2026-08-23T22:45:00Z
revision: 1
---

# Review — TASK-0085 (live denylist-exception evidence packet)

## Verdict: ACCEPTED

Frozen head `4474b54e` (worker branch `worker/verdigris/pc/ox-pc-bb`), evidence
head `576d325d`, reviewed in detached worktree `review-task0085-4474b54e`.

## Scope

Worker-only delta `224a0b7c..4474b54e` touches only
`orchestration/tasks/TASK-0085-denylist-exception-audit/**` (FINDINGS.md,
REPORT.md, STATUS.md). Evidence-only packet: no disposition chosen, nothing
renamed, no denylist/config/server/src/native/playtest/docs-product file
modified. `git diff --check` clean.

## Acceptance gates

1. `rg -n -F 'legacyRelicId' --glob '!orchestration/tasks/TASK-0085...' .` → 23 lines, exit 0.
2. `rg -n -F 'bronze-dagger' --glob '!...' .` → 103 lines, exit 0.
3. `rg -n 'legacyRelicId|bronze-dagger' config/legacy-denylist.json` → both
   documented exceptions present, exit 0.
4. `git diff --check` → clean, exit 0.
5. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is outstanding and exhaustive. For each of the two exceptions it
  lists every current occurrence with its contract role (wire-only for
  `legacyRelicId`; both data and canon for `bronze-dagger`), visibility, and
  three dispositions with named breakage — without choosing any.
- **Gate-mechanics finding verified genuine (high value):** the exceptions were
  implemented by **deleting** the canonical terms from the denylist
  `identifiers` (legacyRelicId in `7ab99b65`, bronze-dagger in `f33cc15a`)
  rather than using the purpose-built per-path scoped allowlist. Confirmed
  live:
  - `python check_legacy_denylist.py --self-test` → **exit 1** ("expected denied
    variant was missed: bronzeDagger") — self-test is RED.
  - `python check_legacy_denylist.py` (scan) → **exit 0** ("native legacy
    denylist: PASS") — CI stays green because CI (`ci-native.ps1:27`) runs only
    the scan form.
  - `config/legacy-denylist.json:35` `"allowlist": []` — the scoped allowlist is
    unused, so no spelling of either token is policed anywhere under `native/`.
- Also documented: the live JS/native `bronze-dagger` price divergence (9 vs
  hardcoded 10, networking.cpp:2395) with no harness assertion pinning it.
- The owner-ruling surface is precise: any disposition can be encoded (keep =
  restore nothing + optionally convert notes to scoped allowlist entries;
  migrate = versioned wave; remove = named breakage lists). TASK-0121 G-03 and
  RUN_STATUS are waiting on this packet.

## Capsule

Evidence-only audit respected throughout: no disposition/rename/denylist change,
no forbidden path touched, only owned task-folder paths changed.

## Follow-up

The owner should rule on the two exceptions using this packet. Also, the
gate-mechanics contradiction (self-test RED while CI green, exceptions by
deletion) is a real hardening gap worth a successor task: either convert the
exceptions to scoped allowlist entries or restore the terms and add `--self-test`
to CI.
