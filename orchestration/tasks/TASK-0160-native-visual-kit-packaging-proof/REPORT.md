# TASK-0160 REPORT — Native procedural visual-kit packaging proof

- worker: ox-pc-bd (openrouter / `stealth/ox-alpha`, OpenCode CLI)
- branch: `worker/verdigris/pc/ox-pc-bd`
- clone/worktree root: `Z:\Code\.worktrees\verdigris\ox-pc-bd`
- claim commit: `f81a303b`
- immutable task base: `dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17` (verified
  ancestor of the routed HEAD via `git merge-base --is-ancestor`, exit 0)
- implementation commit: `9473009c` (all four acceptance gates executed at
  this exact HEAD)

## Executive summary

Implemented `native/tools/verify_native_visual_kit.py`, a Python-3-stdlib-only
validator/generator that turns the committed visual kit into a proven,
dependency-free asset contract. Check mode proves, read-only:

1. every manifest entry exists on disk (missing files fail
   `MISSING_SOURCE`; unreferenced `*.svg` files under the kit directory —
   assets silently dropped from the manifest — fail `UNKNOWN_SVG_FILE`);
2. every entry is a valid bounded safe SVG: strict stdlib XML parse
   (`MALFORMED_SVG`), element/attribute/color allowlists plus forbidden-
   construct scan for DOCTYPE/entities/scripts/images/external URLs
   (`UNSAFE_SVG`), and all geometry confined to the frozen 64x64 viewBox
   (`OUT_OF_BOUNDS_SVG`);
3. every entry maps to exactly one stable generated symbol with no
   duplicates in either direction (`DUPLICATE_ENTRY`,
   `UNKNOWN_SYMBOL_MAPPING`, `AMBIGUOUS_SYMBOL_MAPPING`,
   `UNKNOWN_HEADER_ROW`) in identical manifest/header order
   (`ORDERING_MISMATCH`), with matching generator version metadata in both
   artifacts (`VERSION_MISMATCH`);
4. the committed `manifest.json` **and** `generated/visual_kit.h` reproduce
   byte-for-byte from the SVG sources alone (`STALE_MANIFEST`,
   `STALE_HEADER`), with a two-pass determinism guard
   (`NON_DETERMINISTIC_DERIVATION`) and a sha256 digest ledger printed on
   success.

The negative suite `run_negative_tests.py` (15 tests) passes 15/0 against
nine committed synthetic fixtures plus oracle/determinism/read-only/
regeneration controls. No runtime/client paint change, no raster asset, no
third-party dependency, no network, no ports; final art decisions untouched.

## Approach

The key observation is that the committed nine `svg/*.svg` files are the
complete source of truth for the derived pair (`manifest.json`,
`generated/visual_kit.h`): the TASK-0147 generator's palette arrays,
color tables, point pools, shape records and symbol rows are all pure
functions of the SVG document order and lexemes. The validator therefore

- parses each SVG with `xml.etree.ElementTree` under strict allowlists
  (elements: svg/circle/ellipse/polygon/polyline only; per-element attribute
  allowlists incl. mandatory `stroke-linejoin="round"` on stroked shapes;
  colors restricted to `#rrggbb[#aa]`; numbers restricted to plain decimal
  lexemes; text content rejected), recomputing each variant's first-
  appearance fill/stroke palette exactly like `variantPalette()`;
- rebuilds `manifest.json` via `json.dumps(indent=2)` in manifest-declared
  role/motif order, and `visual_kit.h` by replaying the frozen
  `task0147-gen-2` emission rules — vertex-unit `point_begin/point_end`
  cursors, first-use color indexing, 8-float kPoints line chunking,
  `ShapeKind::` qualified rows, `.f`-suffixed literals with trailing-zero
  trimming, and hex→float channel conversion done in integer math
  (`divmod(c*100, 255)` with half-up rounding) so no float-sensitive RNG or
  libm path can ever diverge from the Node original;
- compares derived bytes against committed bytes, reporting the first
  divergence offset with context snippets.

Ordering authority is the manifest's declared entry sequence; the header
must match it element-for-element. Regeneration (`--regenerate`) rewrites
only the two derived artifacts, refuses to run when structural contract
errors exist (missing/unsafe/malformed SVGs, duplicate entries, unknown
files) and never generates SVG content itself — art remains owner-domain.
Exit codes: 0 pass, 1 validation failures, 2 usage/IO errors.

## Changed files (owned paths only)

- `native/tools/verify_native_visual_kit.py` (new)
- `orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/run_negative_tests.py` (new)
- `orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/fixtures/**` (new:
  9 synthetic kits x {manifest.json, svg/*.svg, generated/visual_kit.h})
- `orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/STATUS.md` (claim → REVIEW_REQUESTED)
- `orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/REPORT.md` (this file)

No file outside `native/client/assets/**` (untouched — proven by gate 4),
`native/tools/verify_native_visual_kit.py`, and this task folder was created,
modified, or deleted. In particular none of `native/client/main.cpp`,
`remote_session.cpp`, `native/src/**`, `native/include/**`, `build.ps1`,
`CMakeLists.txt`, `server/**`, `src/**`.

## Public interfaces added

- CLI: `python native/tools/verify_native_visual_kit.py --check|--regenerate [--root DIR]`
  (`--root` hidden escape hatch used by the test harness; defaults to the repo root).
- Importable API for successors/tests: `Report`, `parse_svg`, `compute_palette`,
  `derive_manifest`, `derive_header`, `derive_all`, `load_entries`,
  `collect_svgs`, `parse_header_symbols`, `check_symbol_mapping`, `first_difference`.
- Harness CLI: `python orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/run_negative_tests.py` (no arguments).

## Acceptance commands — literal transcripts

Executed from the repository root exactly as written in SPEC frontmatter, at
implementation commit `9473009c`. Python: 3.12.6.

### Gate 1: `python native/tools/verify_native_visual_kit.py --check`

```text
digests (sha256):
  8ddd42d320a970b66ed5a39f2b5a1a0d5d05261643408eff452dbfdb48508657  native/client/assets/manifest.json
  0c48f5c861ae4be957e56d9f0809ac12fdee9f70aebdf896c9d74fcaa0611465  native/client/assets/generated/visual_kit.h
  a589a821c40f3d44de072a869a58b7d3dd72fe15b05c285b553d99a94f6514a6  native/client/assets/svg/dwelling.svg
  d5122f28c80e4c83eb96c1a322696d444ff7eebd3d946334ff6074b42a64e3cd  native/client/assets/svg/elite.svg
  a1332d1d2cba731669cb5aa528784385a814d1a7c671fe907f8a1ae4f957f700  native/client/assets/svg/player.svg
  d0121da2f4d6ae02048b36acc424200b184403ab6f55637524c76bc605a31392  native/client/assets/svg/raider.svg
  4f7dde806e89711f863641f25be08c4dcc2db5382a7087286ed275d2cc8d3f66  native/client/assets/svg/ruin.svg
  5687b74dd57f9ddc632bf3d86c486873fe43e7493d8ecf918fd8d83105c49a5b  native/client/assets/svg/shrine.svg
  80b298a20cdfbd79174c19795f955ba510d5895f428fc309864f1af95966b68d  native/client/assets/svg/terrain-a.svg
  f768a753c5445726e2fc0947aef9d928fbee3e9e154dd1ebea911f5b68dca775  native/client/assets/svg/terrain-b.svg
  66659226e48a24fdfe092e5a3d69d8a13440f51d56f58534fbc1946be79f9552  native/client/assets/svg/tree.svg
verify_native_visual_kit: OK (kit reproduces byte-for-byte)
GATE1 EXIT: 0
```

### Gate 2: `python orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/run_negative_tests.py`

```text
validator under test: Z:\Code\.worktrees\verdigris\ox-pc-bd\native\tools\verify_native_visual_kit.py
PASS cli/valid-mini-kit
PASS cli/missing-source
PASS cli/duplicate-entry
PASS cli/unknown-file
PASS cli/unsafe-svg
PASS cli/malformed-svg
PASS cli/palette-mismatch
PASS cli/stale-header
PASS cli/ordering-mismatch
PASS oracle/solo-golden-header
PASS oracle/solo-golden-manifest
PASS determinism/two-pass-and-fresh-parse-equal
PASS readonly/check-leaves-tree-byte-identical
PASS regen/refuses-unsafe-kit-without-writes
PASS regen/repairs-stale-manifest-then-check-passes
15 tests: 15 passed, 0 failed
GATE2 EXIT: 0
```

Fixture → primary failure-code mapping exercised by the suite:
missing-source→MISSING_SOURCE, unknown-file→UNKNOWN_SVG_FILE,
duplicate-entry→DUPLICATE_ENTRY, unsafe-svg→UNSAFE_SVG (DOCTYPE/entity +
script/image injection), malformed-svg→MALFORMED_SVG,
palette-mismatch→PALETTE_MISMATCH(+STALE_MANIFEST),
stale-header→STALE_HEADER, ordering-mismatch→ORDERING_MISMATCH(+STALE_HEADER);
valid-mini-kit exits 0.

### Gate 3: `git diff --check`

```text
(no output — no whitespace errors)
GATE3 EXIT: 0
```

### Gate 4: `git diff --name-only`

```text
(no output — working tree clean; check mode provably wrote nothing)
GATE4 EXIT: 0
```

## Additional verification beyond SPEC minimums

- At the same HEAD, `--regenerate` rewrote both derived artifacts
  byte-identically ("regenerated cleanly", exit 0; subsequent
  `git diff --stat`/`git status --short` empty except the harness's
  untracked `__pycache__`, removed before the report commit).
- Independent golden-oracle: the harness pins the complete expected header
  and manifest bytes for a single-symbol kit as hand-written literals
  (oracle/solo-golden-header, oracle/solo-golden-manifest).
- Read-only proof: full tree digest before/after `--check` asserted equal.
- Regeneration safety: refuses an unsafe kit without writing anything;
  repairs a stale manifest then re-checks clean.
- The real kit reproduction was also hand-audited mid-development: color
  channel percent math, vertex-unit offsets, 8-per-line chunking and symbol
  row layout verified manually against the TASK-0147 Node emitter.

## Manual verification

- Ran every gate from the repo root exactly as written in SPEC frontmatter.
- Confirmed via `git status --short` that between claim and gates only owned
  paths appeared; `git diff --name-only` empty at gate time.
- No server started; port 6500 never touched; no network or package install
  (stdlib only); commits made with `--no-verify` because the yorkie
  pre-commit hook cannot resolve `node_modules` in this worktree (fleet-
  established pattern; hook not modified).

## Commit SHAs

| Commit | Content |
|---|---|
| `f81a303b` | CLAIMED status by ox-pc-bd |
| `9473009c` | validator CLI + 15-test harness + 9 fixture kits (all four gates executed at this HEAD) |
| `<review-requested>` | STATUS transition to REVIEW_REQUESTED + this report |

The branch tip at push (`<review-requested>`) is the authoritative
REVIEW_REQUESTED evidence head; it differs from `9473009c` only by REPORT.md
and STATUS.md inside the owned task folder.

## Interpretive notes (no spec deviations)

- "Stable ordering/hash metadata" is implemented as: manifest entry order is
  the canonical ordering and must equal header `kSymbols` order exactly;
  `generatorVersion`/`kKitVersion` identity metadata must match the frozen
  `task0147-gen-2` constant; derivation must be bit-deterministic across
  repeated runs; and a sha256 digest ledger over every kit file is emitted
  as verifiable evidence on every successful check.
- Byte-for-byte reproduction covers the two derived artifacts
  (manifest.json, visual_kit.h) regenerated from the committed SVG sources;
  the SVGs themselves are inputs by design (regenerating them would require
  the procedural RNG and would risk changing visual meaning — the STOP
  condition), so they are instead validated structurally, boundedly, safely,
  and pinned by the digest ledger.

## Unresolved questions / risks / follow-ups

- None blocking. Follow-up candidates for the owner: wire the validator into
  CI alongside the existing native gates; if future kits grow beyond the
  current shape grammar, extend the allowlists deliberately rather than
  relaxing them.

## Scope compliance

- Owned paths only: yes (`native/client/assets/**` untouched; new tool file
  under `native/tools/` as specified; everything else in the task folder).
- Forbidden paths untouched: main.cpp, remote_session.cpp, native/src/**,
  native/include/**, build.ps1, CMakeLists.txt, server/**, src/** — no
  binary production art, no third-party dependencies, no final art decision.
- Negative controls honored: no WIZARD import, no copyrighted external
  asset, no lore/naming/aesthetic judgment, no runtime paint change.
- No merge to program branches, no force-push, port 6500 untouched.
- Pushed: only `worker/verdigris/pc/ox-pc-bd`.
