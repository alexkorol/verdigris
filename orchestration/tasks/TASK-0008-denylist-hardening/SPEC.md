---
id: TASK-0008
title: Legacy denylist hardening from audit evidence
state: READY
track: tooling
priority: medium
base_commit: wave-2 integration tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0005]
parallel_safe: true
owned_paths:
  - native/tools/**
  - config/legacy-denylist.json
forbidden_paths:
  - native/src/**
  - native/include/**
  - native/tests/**
  - native/client/**
  - src/**
  - server/**
  - prototypes/**
  - docs/**
acceptance_commands:
  - python native/tools/check_legacy_denylist.py
  - powershell -File native/build.ps1 -RunTests
---

## Goal

The denylist gate catches the evasion classes the TASK-0005 audit proved it
misses, without flagging legitimate native code or the historical browser
reference.

## Why this task exists

TASK-0005 §4 documented concrete gaps: identifiers slip through via
camelCase/PascalCase and hyphenated variants, Delaford-derived zone/guest
identifiers that omit the literal name, and denied content appearing in
non-C++ native data files that the checker never scans.

## Product and architectural invariants

- The gate governs NATIVE production code (D-005); historical `src/` and
  `server/` stay exempt while the migration is active.
- Zero false positives on the current native tree: the gate must pass at
  base_commit before and after the hardening (prove both in REPORT.md).
- Pure stdlib Python; no new dependencies.

## Inputs and references

TASK-0005 REPORT.md §4 (`orchestration/tasks/TASK-0005-legacy-archaeology-audit/`),
`native/tools/check_legacy_denylist.py`, `config/legacy-denylist.json`.

## Scope

1. Normalize matching: case-fold and split identifiers (camelCase,
   PascalCase, snake_case, kebab-case) before comparison so a denied term
   is caught in any casing/joining, while allowing an explicit allowlist
   for justified exceptions.
2. Extend the scan set to native data/config files (`native/**` json,
   text, cmake, ps1 — pick the concrete extension list and document it),
   not just C++ sources.
3. Add the audit's specific evaded identifiers to the denylist where they
   are genuinely Delaford residue; keep the file sorted and commented by
   category. Do not add terms the audit tagged as Verdigris-era.
4. Self-test: a small fixture-based test mode (`--self-test`) proving the
   matcher catches representative variants and honors the allowlist; wire
   nothing into the C++ test binary.

## Non-goals

No purges of legacy code, no docs edits (the architect updates
LEGACY_MATRIX separately), no CI changes.

## Deliverables

Updated checker + denylist, one coherent commit.

## Acceptance criteria

- Both acceptance commands exit 0 at the hardened state.
- `--self-test` exits 0 and demonstrably fails when a fixture variant is
  injected (show one negative run in REPORT.md).
- REPORT.md lists every denylist term added, each tied to audit evidence.

## Required verification

The two acceptance commands plus the self-test, outputs pasted in
REPORT.md.

## File ownership

Only `native/tools/**` and `config/legacy-denylist.json`.

## Dependencies

TASK-0005 (evidence source; already integrated).

## Parallel-safety assessment

Disjoint from TASK-0007 (core sources). The build gate runs the checker,
so coordinate integration order with 0007 normally (no shared files).

## Review focus

False-positive discipline (the allowlist mechanism), the extension list
chosen for scanning, and that added terms match audit evidence rather than
guesswork.

## Stop conditions

A term's provenance is ambiguous (audit tagged it `mixed`) → list it in
REPORT.md as proposed-but-not-added and continue; do not decide product
questions in a tooling task.
