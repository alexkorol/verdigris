# TASK-0114 REPORT — Stage-2 renderer backend evaluation matrix

```yaml
task: TASK-0114
lane: ox-pc-bb
model: openrouter/stealth/ox-alpha
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
branch: worker/verdigris/pc/ox-pc-bb
claim_commit: 84ede1a0
deliverables_commit: adf17d7b5123b129e35b57cd3152abf64ebdcbf2
review_requested_at: 2026-08-23
```

## Executive summary

Evaluated all five SPEC candidates — Direct3D 11, OpenGL 3.3 core, SDL2 plus an
explicit batching layer, sokol_gfx, and optimized GDI as the null option —
against every required criterion (Windows/macOS viability, sprites, atlases,
shaders, text, offscreen capture, resource lifetime, plain MSVC/CMake
integration without a package manager, licenses, binary/dependency weight,
migration from render-list ops). Deliverables:
`EVALUATION.md` (matrix + risk table + migration sketch + unknowns) and
`captures/source-index.json` (20 numbered sources with URLs and access dates,
upstream version pins). Recommendation made for **two** candidates (sokol_gfx
and SDL2 2.32.10, each with an explicit batching layer); **no decision** taken
— the ADR remains architect+owner.

## Approach

1. AGENTS.md preflight (clean tree, branch in sync 0/0; base commit verified as
   an ancestor of HEAD).
2. Local evidence pass: `render_list.hpp` op contract, GDI paint/capture call
   sites in `client/main.cpp`, `CMakeLists.txt` link surface, `native/README.md`
   build/test gates.
3. Primary-source web research only (resource capsule honored: no downloads,
   builds, dependencies, or ports; port 6500 untouched): Microsoft Learn (D3D11,
   WARP, Map, DirectX SDK location, GDI, AlphaBlend), Khronos registry/refpages,
   floooh/sokol (README, LICENSE, pinned sokol_gfx.h full text), libsdl-org/SDL
   (releases API, LICENSE.txt, README-cmake.md at release-2.32.10, wiki pages),
   Dav1dde/glad license.
4. Upstream pins frozen in `source-index.json`: sokol @
   `7cee0ba17c358985e4744fe8ac20b6829d328229` (2026-08-18); SDL2
   `release-2.32.10` (2025-09-01); GL 3.3 core spec via registry; D3D11/GDI as
   Windows SDK documentation sets current at access date 2026-08-23.
5. Negative control preserved: Apple's macOS OpenGL deprecation wording could
   not be retrieved from any primary source (live 404 + no Wayback snapshot) and
   is recorded as UNKNOWN U1 rather than inferred. Additional explicit
   unknowns U2–U6 recorded (uncompressed SDL2.dll size, sokol readback spike,
   SDL_ttf license, sokol binary size, osmesa-class headless GL).

## Changed files

Only owned paths (`orchestration/tasks/TASK-0114-renderer-backend-evaluation/**`):

- `STATUS.md` (claim → review-requested)
- `EVALUATION.md` (new)
- `captures/source-index.json` (new)
- `REPORT.md` (this file)

`git diff --cached --name-only` before the deliverables commit showed exactly:

```
orchestration/tasks/TASK-0114-renderer-backend-evaluation/EVALUATION.md
orchestration/tasks/TASK-0114-renderer-backend-evaluation/captures/source-index.json
```

## Public interfaces added/changed

None. Documentation-only task; no code, no dependencies, no build changes.

## Acceptance commands — literal transcripts + exit codes

### 1. Source index validation

```
$ node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0114-renderer-backend-evaluation/captures/source-index.json','utf8')); console.log('source index: PASS')"
source index: PASS
node exit code: 0
```

### 2. Criteria-presence grep over EVALUATION.md

```
$ rg -n "Windows|macOS|sprite|atlas|shader|text|offscreen|license|CMake|GDI" orchestration/tasks/TASK-0114-renderer-backend-evaluation/EVALUATION.md
<100+ matching lines across sections 1–9 of EVALUATION.md>
rg exit code: 0
```

Representative hits confirm every criterion token is present (full transcript
captured in the session log; sample lines):
line 41 Windows viability · line 42 macOS viability · line 43 Sprites ·
line 44 Atlases · line 45 Shaders · line 46 Text · line 47 Offscreen capture ·
line 50 License · line 49 MSVC/CMake integration · lines 109–121 Optimized GDI.

### 3. Whitespace check

```
$ git diff --check
git diff --check exit code: 0
$ git diff --cached --check
git diff --cached --check exit code: 0
```

(supplemental staged variant run because deliverables were staged at command
time)

### 4. Changed-path proof

```
$ git diff --name-only
git diff --name-only exit code: 0          # empty output: nothing unstaged
$ git diff --cached --name-only
orchestration/tasks/TASK-0114-renderer-backend-evaluation/EVALUATION.md
orchestration/tasks/TASK-0114-renderer-backend-evaluation/captures/source-index.json
git diff --cached --name-only exit code: 0
$ git status --short
A  orchestration/tasks/TASK-0114-renderer-backend-evaluation/EVALUATION.md
A  orchestration/tasks/TASK-0114-renderer-backend-evaluation/captures/source-index.json
```

Expected outcome met: all criteria present; only this task folder changed.

## Manual verification

- Every nontrivial claim in EVALUATION.md carries a `[Snn]` citation resolving to
  a URL + access date (2026-08-23) in `captures/source-index.json`; absence
  claims (e.g., "GDI exposes no programmable pipeline") are framed against what
  the cited references document, never folklore.
- sokol readback absence verified by full-text search (`readback`, `read-back`,
  `sg_read`, `staging texture`, `D3D11_MAP_READ`, `GL_PIXEL_PACK`) over the
  complete pinned `sokol_gfx.h` fetched from raw.githubusercontent at the exact
  SHA — zero public-API hits.
- SDL2 asset sizes quoted verbatim from GitHub releases API JSON parsed locally;
  no release artifacts downloaded.

## Deviations

- None from the SPEC. Note (not a deviation): the worker branch tip at claim
  time was `424c3151`, ahead of the immutable base `d2423873…` due to other
  lanes' integrated work; base verified as ancestor, and all edits confined to
  owned paths keep the diff equivalent to base-scoped work.

## Unresolved questions (for the architect/owner ADR)

1. Verify Apple's current macOS OpenGL policy from a primary source (unknown U1).
2. If sokol_gfx is shortlisted: authorize a dependency-spike milestone for the
   GPU→CPU capture path (R7/U3) before committing to it in the ADR.
3. If SDL2 is shortlisted: answer SDL2-vs-SDL3 positioning explicitly (R6);
   SPEC pins SDL2, so SDL3 would need a superseding task.
4. Confirm whether runtime shader compilation (d3dcompiler_47.dll side-by-side)
   vs offline FXC precompilation matters for distribution/signing constraints
   (TASK-0134 territory).

## Risks

See EVALUATION.md §5 (R1–R11) — headline items: D3D11 has no macOS path [R1];
raw GL carries the unverifiable macOS-policy risk [R2]; SDL2 lacks shader
access under its renderer API [R4]; sokol_gfx lacks a documented readback API
[R7]; GDI null option retains the effects ceiling that motivates Stage 2 [R10].

## Follow-ups

- Owner reads the shortlist; architect schedules the Stage-2 renderer ADR.
- If accepted, first implementation milestone should be Phase 0 golden-capture
  parity freeze per EVALUATION.md §6 before any Painter extraction.
