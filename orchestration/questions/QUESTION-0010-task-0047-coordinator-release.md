---
question: QUESTION-0010
related_task: TASK-0047-native-protocol-n4
status: OPEN
owner: Fable / Kimi-work coordination
---

## Decision needed

Should the existing Kimi-work claim on TASK-0047 be resumed or released so a
different worker can implement the N4 parity wave?

## Evidence

- `origin/codex/TASK-0047-native-n4-kimiwork` remains at claim commit
  `d1e30e2b6`; there is no implementation commit or REPORT.
- The Kimi-work clone contains only an uncommitted `core.hpp` declaration
  block, survey notes, and a forge-truth capture script.
- The uncommitted header makes the branch fail at link time because
  `VesselForge` is declared as a live member without definitions.
- Clean committed claim tip `d1e30e2b6` independently passes
  `powershell -File native/build.ps1 -RunTests` (legacy denylist, core tests,
  and networking tests). The failure is isolated to the uncommitted header
  draft, not the accepted N3 baseline.
- `native/src/core.cpp`, `native/src/networking.cpp`, and native tests contain
  no N4 definitions/usages for the declared forge, inventory, depth, or wire
  surfaces.

## Options and consequences

1. Kimi-work resumes immediately and either completes or commits a coherent
   partial implementation; preserves continuity but delays the critical N4
   wave if the claim remains idle.
2. Fable places a `RELEASE.md` in the task folder; Codex can then claim N4 in
   a fresh isolated worktree without touching Kimi-work's uncommitted files.
3. Leave the claim unchanged; no safe second implementation can start under
   the first-STATUS-write-wins protocol.

## Recommendation

Prefer option 1 only if Kimi-work confirms an active implementation window;
otherwise release the claim (option 2). Do not merge the header-only work.

## Blocking scope

This is an orchestration handoff, not a product decision. Native health gates
on the current architect tip pass; only TASK-0047 implementation ownership is
blocked by the stale claim.
