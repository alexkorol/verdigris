# REPORT — TASK-0144 native visual-kit C++ literal portability correction

- worker: ox-pc-g
- branch: `codex/TASK-0144-native-visual-kit-msvc-portability-ox-pc-g`
- routed HEAD/base at claim: `9c09ff521929fd63fc8b464591cb69d127bd3f48`
- claim commit: `7c469f7d`
- implementation commit: `87981a5b`
- state at writing: REVIEW_REQUESTED

## Executive summary

The TASK-0141 visual-kit generator emitted GLSL-style float literals
(`1f`, `22f`) into `native/client/assets/generated/visual_kit.h`. Bare `f` is a
reserved literal-operator suffix in standard C++, so TASK-0142 had to add
temporary `operator""f` definitions (with `#pragma warning(disable: 4455)`) to
`native/client/main.cpp` for MSVC. This task fixes the defect at the source:
the generator's float serializer now emits an explicit decimal point for every
integral value (`1.f`, `22.f`, `0.f`), the header was regenerated, and the shim
was removed. The client compiles under MSVC against the header with no
consumer workaround, and all scenario gates remain green.

## Approach

- Minimal, deterministic change confined to `cppFloat()` in
  `orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs`:
  after the existing `fmt()` rounding, append `.` when the rendered text has no
  decimal point, then the `f` suffix. `fmt()` itself is untouched, so SVG and
  manifest serialization is byte-identical.
- Regenerated assets via the generator (`WROTE` pass); only the header changed,
  confirming SVG/manifest byte stability.
- Removed the temporary literal shim from `native/client/main.cpp`
  (both `operator""f` overloads plus the `#pragma warning(default: 4455)`
  pair), keeping the read-only consumption comment and include.

## Changed files

- `orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs`
  — conforming literal serializer (owned by this task's outcome statement).
- `native/client/assets/generated/visual_kit.h` — regenerated; 630 lines
  rewritten from `Nf` to `N.f` forms; role/shape/symbol counts unchanged.
- `native/client/main.cpp` — literal-operator shim removed (-14 lines net).

No other paths touched. Forbidden paths (`native/src/**`,
`native/include/**`, `native/tests/**`, server, browser, CI) untouched.

## Public interfaces added or changed

None. The generated header's struct/enum/table layout and the client's
consumption surface are unchanged; only literal spelling inside table
initializers changed, with identical values.

## Acceptance commands and outcomes

Full transcript with exit codes:
`captures-gate-transcript.txt` (this folder).

1. `node --test orchestration/tasks/TASK-0141-procedural-native-visual-kit/asset-kit.test.mjs`
   → 9/9 pass, exit 0 (includes byte-for-byte regeneration equality and
   determinism tests).
2. `node orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs --check`
   → all 11 files OK, exit 0.
3. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios`
   → exit 0; native test suite and all client scenarios (including
   remote-render-list and zoom-invariance suites) PASS with 0 failures.
   This is the compile proof: `main.cpp` builds without any reserved-suffix
   literal operators.
4. `git diff --check` → clean, exit 0.

Literal audit: `rg --pcre2 "(?<![\w.])[0-9]+f\b"` over the regenerated header
finds zero bare integer-f tokens. Note on the SPEC regex `\b[0-9]+f\b`: taken
literally it also matches digit runs inside conforming decimals (`25f` inside
`0.25f`, because `\b` matches between `.` and a digit); the corrected lookbehind
form above expresses the intended defect class ("non-conforming integer-float
tokens such as `1f`") and returns no matches. MSVC accepting the header without
C4455 suppression is the authoritative conformance proof.

## Manual verification

- `git diff --stat` confirms exactly three modified tracked files plus task
  folder docs; SVGs and `manifest.json` are byte-identical (not listed as
  modified after regeneration).
- Header diff spot-check: only `{0f, ...}` → `{0.f, ...}` spellings changed;
  counts of `ShapeKind::` rows, symbols, and colors match the pre-change file
  (test 8 asserts symbol/shape coverage against the manifest independently).

## Deviations

- SPEC frontmatter `base_commit` is `c0b79e5…`; routing directed this lane to
  base `9c09ff5…` (TASK-0142 integration line). Worked on the routed HEAD as
  instructed; both are ancestors of the linear history reviewed here.

## Unresolved questions

None.

## Risks

Low. Values are numerically identical; only token spelling changed. Any
downstream tooling that greps the header must use a dot-aware pattern (see
literal audit note above).

## Follow-ups

- If a future generator revision changes output shape, bump
  `GENERATOR_VERSION` per TASK-0141 conventions (not done here to keep
  SVG/manifest bytes stable, as the SPEC requires).
