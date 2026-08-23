---
task: TASK-0092
title: Owner launch and packaging readiness audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: c84fc0d6
reviewed_at: 2026-08-23T19:20:00Z
revision: 1
---

# Review — TASK-0092 (owner launch and packaging readiness audit)

## Verdict: ACCEPTED

Frozen head `c84fc0d6` (worker branch `worker/verdigris/pc/ox-pc-bd`),
content head `f004c44d`, reviewed in detached worktree
`review-task0092-c84fc0d6`.

## Scope

Worker's own commits (`39fc7be0..c84fc0d6`) touch only
`orchestration/tasks/TASK-0092-owner-launch-packaging-audit/**` (FINDINGS.md,
REPORT.md, STATUS.md, captures/package-inventory.json,
captures/acceptance-1-launcher-sweep.txt, captures/acceptance-2-cmake-sweep.txt).
No build/launcher/source file modified; read-only capsule honored (nothing
executed, no ports touched, port 6500 untouched). `git diff --check` clean.

## Acceptance gates

1. `rg -n "play-native|verdigris_client|verdigris_server|6520|6539" native/README.md native/tools native/build.ps1`
   → 56 lines, exit 0.
2. `rg -n "CMAKE|MSVC|WIN32|APPLE|install|package" native/CMakeLists.txt native/CMakePresets.json native/build.ps1`
   → 15 lines, exit 0.
3. `node -e "JSON.parse(...package-inventory.json...); console.log('package inventory: PASS')"`
   → prints `package inventory: PASS`, exit 0.
4. `git diff --check` → clean, exit 0.
5. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent and well-structured: launcher assessment, executable/
  dependency/generated-file inventory, asset/save locations, negative control,
  failure-message inventory, version metadata, platform gaps, SPEC-required
  layer separation, and a sequenced packaging plan (PK-0→PK-5) stopping at the
  owner-only signing/distribution boundary.
- **Negative control verified genuine:** `build.ps1:159` runs bare
  `python tools/check_legacy_denylist.py` with no availability probe, firing
  after ~30 compile/link steps — clean Windows machines without Python fail late
  with an obscure error (or a silent Store-alias stub). The suggested preflight
  guard mirrors the existing vcvars probe pattern. Confirmed in source.
- **Latent 6500 invariant risk verified:** `server_main.cpp:10` defaults
  `port = 6500` for a bare-launched server; only the launcher always passes an
  explicit capsule port. Correctly flagged as a latent violation of the frozen
  owner-6500 invariant that PK-2 should close.
- **No packaging targets verified:** `rg "install|package|APPLE"` over
  CMakeLists/Presets → no matches (exit 1); zero VERSIONINFO/icon/CRT-pinning.
- Sequenced packets (preflight hardening, layout contract, executable identity,
  unsigned handoff, launch UX, macOS spike) are concrete, independently
  revertible, and stop at the owner boundary.
- Machine-readable twin `captures/package-inventory.json` parses.

## Capsule

Read-only audit respected throughout: no launcher executed, no server started,
no port probed, port 6500 untouched, only owned task-folder paths changed.

## Follow-up

Cut packets PK-0 (python preflight + pinning + provenance stamp) and PK-2
(executable identity + server default-port successor) first; both are small,
localized, and close the two substantive findings (NC-1 and the 6500-default
latent violation).
