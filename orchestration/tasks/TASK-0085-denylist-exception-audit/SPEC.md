---
task: TASK-0085
title: Live denylist-exception evidence packet
state: SUPERSEDED
superseded_by: integrated (reviewed head 4474b54e, 2026-08-23)
packet: MECHANICAL
topology: INDEPENDENT
priority: medium (owner ruling prep)
lane: luna-mac; Qwen may build tables from verified grep output
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owned_paths:
  - orchestration/tasks/TASK-0085-denylist-exception-audit/**
forbidden_paths:
  - config/legacy-denylist.json
  - native/**
  - server/**
  - src/**
  - playtest/**
  - docs/product/**
---

# Outcome

Produce `FINDINGS.md` for the two owner-pending exceptions named in the Sol
handoff: wire key `legacyRelicId` and item id `bronze-dagger`. For each, list
every current source/test/data occurrence, the exact protocol or harness
contract that requires it, whether the name is player-visible or wire-only,
and three dispositions: keep documented exception, migrate compatibly, or
remove with named breakage.

This task gathers evidence only. It does not choose a disposition, rename
anything, change the denylist, or make a lore/item decision.

# Acceptance commands

Paste literal commands, output, and exit codes in `REPORT.md`:

```bash
rg -n -F 'legacyRelicId' --glob '!orchestration/tasks/TASK-0085-denylist-exception-audit/**' .
rg -n -F 'bronze-dagger' --glob '!orchestration/tasks/TASK-0085-denylist-exception-audit/**' .
rg -n 'legacyRelicId|bronze-dagger' config/legacy-denylist.json
git diff --check
git diff --name-only
```

# Stop conditions

STOP at the evidence boundary. Any proposed canonical replacement name,
compatibility window, or item redesign is owner-only.
