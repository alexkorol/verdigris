---
task: TASK-0149
verdict: ACCEPTED
reviewed_head: a88d307d56460cb67c2f6bf5bc49071d4a51e750
reviewed_at: 2026-08-22 04:22 -07:00
supersedes_reviewed_head: 96f4ccbd1572add96e34ccb230b9935b743d7ff3
---

# TASK-0149 revision 2 review — ACCEPTED

The clean pushed revision head closes the sole numbered REVISE item without
broadening into gameplay, client, server, build-system, or capsule changes.
`Start-OwnerServer` now owns cleanup for every exception after
`Start-Process`, publishes the exact spawned PID before throwable readiness
checks, and distinguishes an already-exited spawn from a live spawn it stops.

Independent verification ran from a detached worktree at the exact reviewed
head:

- PowerShell parser: PASS;
- `-ReadinessFaultControl`: PASS for a live silent readiness timeout and a
  live wrong-port impostor; exact PIDs 24164 and 25296 were proven gone;
- `native/build.ps1 -RunTests`: PASS;
- `play-native.ps1 -Rebuild -LifecycleSelfTest`: PASS with real
  `VerdigrisNativeClient` windows for normal close and forced exit, with exact
  client/server PIDs clean;
- the four preserved fail-fast controls and three new invalid-switch controls
  all returned the expected exit code 1;
- revision diff is exactly `native/tools/play-native.ps1`, STATUS, and REPORT;
  `git diff --check` passes.

The frozen revision 1 head remains preserved for audit. Integrate both
implementation commits `96f4ccbd` and `a88d307d` in order; do not integrate
the claim-only commit.
