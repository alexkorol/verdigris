# VG-GOV-002 — Resolve orchestration precedence (DRAFT, not owner-stamped)

Proposed 2026-09-06 by Cursor Grok for owner stamp. This is **not** a
ruling. Until the owner signs it, `orchestration/PROTOCOL.md` plus the
standing "commit locally; the owner pushes" workspace rule remain in
force, and `orchestration/CURSOR_KIMI_LANES.md` is only an on-machine
lease map.

## Conflict (pack DRAFT-D01 / D02)

The pack's first-push-wins claim model disagrees with this repository:

- PROTOCOL (commit `e3ad60da`, 2026-09-03): do not push by default;
  push when the owner explicitly asks.
- Workspace rule: commit locally; the owner pushes.
- Pack: exclusive claim via first successful push of a worker branch.

Result: Kimi's TASK-0108 commits are real on this machine and invisible
on origin until the owner pushes. Separate successful lane-branch pushes
must never count as exclusive claims.

## Proposed policy table (for owner stamp)

| State change | Authoritative writer | Target | Approval |
|---|---|---|---|
| Product / constitution / DECISIONS | Owner | `docs/product/**`, `orchestration/DECISIONS.md` | Owner only |
| TASK packet STATUS | First STATUS writer, then that coordinator | `orchestration/tasks/<task>/STATUS.md` | PROTOCOL; architect may revoke |
| Path lease (this machine) | First writer of the path in `CURSOR_KIMI_LANES.md` | that file | Released only by that writer or owner |
| Worker branch commits | Claiming agent | `kimiwork/*` or `codex/*` in that agent's clone | No origin claim until owner push |
| Origin update | Owner (or owner-explicit push request) | `origin` | Owner |
| VG planning ID | Pack registry only | `docs/execution/pack/` | Never becomes a TASK number |
| Architect checkout branch | Architect / Cursor in `delaford_game` | `codex/native-reconstitution` | Coordinators do not switch this checkout |

## Negative control (must remain true after stamp)

- A pushed `kimiwork/TASK-0108-ranged-rev3` does **not** by itself lock
  `native/client/main.cpp`.
- Two agents pushing different branches that touch the same owned path
  is a collision, not two valid claims.
- Cursor publishing `docs/execution/**` does not mint TASK-0108.

## Working rule until stamp

Cursor holds `native/client/**`, `native/renderer/gpu/**`,
`docs/execution/**`, and the additive `state.xp` block. Kimi Work holds
`native/src/**`, `native/include/**`, `native/tests/**` (minus frozen
gate-b / Cursor scenario regions), `native/tools/**`, and claimed task
folders. Owner pushes.
