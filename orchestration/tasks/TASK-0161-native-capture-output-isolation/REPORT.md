# TASK-0161 report — native scenario capture-output isolation

## Executive summary

Native scenario evidence can now be isolated under a validated, contained
capture root so a full validation gate writes fresh evidence into a disposable
directory instead of rewriting committed captures from TASK-0070, TASK-0122,
TASK-0145, TASK-0156, or TASK-0159. The seam is one explicit `-CaptureRoot`
build option on `native/build.ps1`, handed to `verdigris_client.exe` through a
single documented environment variable (`VERDIGRIS_CAPTURE_ROOT`), validated as
strictly repository-contained by both layers, threaded through every scenario
capture helper, and rejected loudly before any filesystem mutation when invalid.
The literal acceptance command passed twice (EXIT=0, all six test suites plus
all 12 client scenarios), and the post-commit re-run left `git status --short`,
`git diff --check`, and `git diff --name-only` all empty with no cleanup
commands. Default owner play and direct task-specific evidence runs are
unchanged: without the variable every helper keeps its historical ladder.

## Approach

- **Client seam (`native/client/main.cpp`, Windows section only):** a cached
  tri-state decision (`capture_root_decision()`) reads `VERDIGRIS_CAPTURE_ROOT`
  once per process, resolves it via `GetFullPathNameA`, discovers the repository
  root by walking up from the cwd/executable directory for the
  `native\`+`orchestration\` markers (same ladder style as the existing
  helpers), and requires the resolved root to be *strictly* inside it
  (case-insensitive prefix with a `\` boundary, equality rejected). Containment
  is proven before any directory creation, so a rejected target is never
  created or written; valid roots are created component-by-component with
  `CreateDirectoryA`. Rejections print `FAIL capture-root: <reason> (nothing
  written)`.
- **Threading:** all five helpers (`chronicles_capture_dir`,
  `animation_vfx_capture_dir`, `progression_capture_dir`,
  `readability_capture_dir`, `reference_capture_dir`) consult
  `capture_root_override()` first and otherwise run their historical bodies
  unchanged. An override-rejected helper returns empty, which is impossible on
  the historical ladder, and each call site guards with an explicit
  `scenario_check(false, "... capture root rejected before any write")` /
  printf+count failure before any write is attempted (no swallowed errors).
  `--reference-scene` output is isolated through the same seam.
- **Build option (`native/build.ps1`):** `[string]$CaptureRoot` validates
  immediately after parsing (relative roots resolve against the repository
  root derived from `$PSScriptRoot`, not the invoking shell's cwd; rooted paths
  must resolve strictly inside; rejection throws before any build or run work).
  The validated absolute path is exported as `VERDIGRIS_CAPTURE_ROOT` around
  only the `--scenario all` invocation and restored in a `finally`.

## Changed files

- `native/client/main.cpp` — contained capture-root validation + threading
  through five helpers + fail-before-write guards at five call sites.
- `native/build.ps1` — `-CaptureRoot` parameter, pre-build containment
  validation, env handoff with restore, scenario exit-code propagation.
- `orchestration/tasks/TASK-0161-native-capture-output-isolation/**` — STATUS,
  REPORT, and eight fresh review captures produced by the isolated gate run
  (committed so the post-gate tree is clean).

## Public interfaces added

- `native/build.ps1 -CaptureRoot <path>` (optional; absent behavior unchanged).
- Environment seam `VERDIGRIS_CAPTURE_ROOT=<contained path>` consumed by
  `verdigris_client.exe --scenario ...` / `--reference-scene ...` runs.

No rendering, gameplay, runtime authority, CI, dependency, or protocol change.
Historical capture files were never deleted, overwritten, restored, or touched;
the only new files live under this task's folder.

## Verification (exact commands and outcomes)

Positive gate — literal SPEC command, run twice:

```
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review
→ GATE1-EXIT=0  (denylist PASS; core/networking/camera2d/session/
   presentation-events/audio suites green; all 12 scenarios PASS 0 failures;
   8 fresh PNGs under .../captures/review)
→ GATE2-EXIT=0  (re-run after committing implementation + captures;
   regenerated PNGs byte-identical)
```

Literal clean-tree proof after GATE2 (all output empty):

```
git status --short        → (empty) STATUS-EXIT=0
git diff --check          → (empty) CHECK-EXIT=0
git diff --name-only      → (empty) NAMEONLY-EXIT=0
```

Negative controls (client seam, direct binary):

```
$env:VERDIGRIS_CAPTURE_ROOT="Z:\Code\.fleet\tmp\ox-pc-ah\outside-probe"
  & native\build\verdigris_client.exe --scenario hud-pane-readability
  → "FAIL capture-root: ... outside repository root ..." / OUTSIDE-EXIT=1;
    Test-Path outside-probe → False (nothing created)

$env:VERDIGRIS_CAPTURE_ROOT="native\build.ps1\probe"  (inside repo, nested
  under an existing file — uncreatable)
  → "FAIL capture-root: ... could not be created (nothing written)";
    INVALID-EXIT=1; tree unchanged

$env:VERDIGRIS_CAPTURE_ROOT="..\..\..\.fleet\tmp\ox-pc-ah\escape-probe"
  → resolves to Z:\Code\.fleet\tmp\... outside repo; rejected;
    ESCAPE-EXIT=1; Test-Path escape-probe → False
```

Negative control (build layer, rejects before building/running):

```
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot Z:\Code\.fleet\tmp\ox-pc-ah\outside-probe
→ throw at build.ps1:34 "...outside the repository root..."; SCRIPT-REJECT-EXIT=1;
  nothing created; client never invoked
```

Default preservation (no env var set):

```
& native\build\verdigris_client.exe --scenario move-and-camera → DEFAULT-MOVE-EXIT=0
& native\build\verdigris_client.exe --scenario first-fight     → DEFAULT-FIGHT-EXIT=0
git status unchanged apart from owned-path edits; historical ladder intact
```

## Manual verification

Reviewed the eight fresh real-GDI review captures under
`captures/review/` (front door, expedition HUD, animation/VFX composite pair,
progression pair, readability closed/open pairs at both resolutions). All are
non-trivial (486 KB–1.29 MB) and render the expected HUD/pane states.

## Commit SHAs

- Claim: `7730e49a3a1d7743132438d23b13500f3106564b`
- Implementation (+ review evidence): `c8cedf1730455d0fc62badc3c707dcbc96123254`
- Handoff (REVIEW_REQUESTED): this commit.

## Deviations

- Added `if ($LASTEXITCODE -ne 0) { throw ... }` after the scenario invocation
  in `build.ps1`. Previously a failing scenario run would not fail the script;
  propagating the exit code is required for the spec's "fails before writing"
  control to be visible and honors "no swallowed write errors". Green-path
  behavior is identical.
- Default-preservation was proven by code-path identity (the inactive branch
  returns the untouched historical ladder) plus green non-writing scenario
  runs, deliberately NOT by running default capture-writing scenarios, because
  that would overwrite committed historical evidence — exactly what this task
  forbids. Direct task-specific evidence runs remain supported unchanged.
- The eight review captures are committed under the task folder so the literal
  post-gate `git status --short` is genuinely empty; GATE2 proved the gate
  regenerates them byte-identically.

## Unresolved questions

None.

## Risks

- Capture byte-stability relies on deterministic rendering + deterministic
  GDI+ PNG encoding, observed across two full gate runs on this machine. A
  future font/GDI stack change could make regenerated PNGs differ (dirty
  status); the fix would be pointing `-CaptureRoot` at an ignored path for
  repeat runs — noted here, not acted on.
- Scenario servers still bind their historical fixed loopback capsules
  (6580-6599, 6780-6799, 7100-7119, 7120-7139, 7160-7179) inherited from prior
  lanes; this task neither uses nor changes them, and both gate runs bound
  cleanly alongside today's concurrent lanes.

## Follow-ups

- Optional later packet: route density-bench and lifecycle-soak outputs through
  the same contained-root seam (out of this task's scope).
