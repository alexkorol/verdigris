# REPORT — TASK-0094: Native asset provenance manifest audit

- **Lane:** ox-pc-bc · **Model:** openrouter/stealth/ox-alpha
- **Worktree:** `Z:\Code\.worktrees\verdigris\ox-pc-bc`
- **Branch:** `worker/verdigris/pc/ox-pc-bc` (fast-forward pushed to `origin/codex/native-reconstitution` per established lane flow)
- **Immutable SPEC base:** `d2423873c577d299b3b39c56024d1d840993c72b`
  (ancestor-verified: `git merge-base --is-ancestor d2423873… HEAD`)
- **Claim:** STATUS.md `state: CLAIMED` committed and pushed at `5cdfed6e`
- **Implementation commit:** this evidence commit (see frozen head in STATUS.md)
- **Deliverables:** `FINDINGS.md`, `captures/assets.json` (179 assets),
  `captures/hash-assets.mjs`, `captures/acceptance-*` transcripts

## Executive summary

Every asset consumed or proposed by native presentation is enumerated, hashed
(sha256), measured (format-header dimensions), and classified with in-repo
provenance/license citations. Result: **179 assets — 150 KEEP, 29 UNKNOWN,
0 BLOCKED.**

The only asset bytes inside the shipped native executable are the procedural
shapes/palettes of `visual_kit.h` (KEEP; deterministic owned generator,
TASK-0141). The native runtime reads zero asset files at runtime.

**Negative control satisfied:** 29 UNKNOWN assets remain non-shippable and are
named (headline example: `src/assets/fonts/pixelmix.ttf`, no license document
anywhere in the repository). No license was inferred for any asset.

Notable audit discoveries recorded (not fixed): `objects.png` IHDR is
288×1056 while its `.tsx` declares 1024 (stale Tiled metadata); inventory
README claims "119 item finals" vs 114 files present; dead unlicensed font
declaration (`UIFont` → Px437).

## Approach

1. **Scope resolution** — "consumed or proposed by native presentation":
   - consumed today = `native/client/assets/**` (SVG kit + manifest + generated
     header) and triage captures;
   - proposed/candidates = WIZARD plates/shader + browser-reference families
     named by the constitution, LEGACY_MATRIX, TASK-0093, and intake docs.
2. **Consumer mapping** — every family's build/runtime consumers cited from
   code (main.cpp, client.js, Quickbar.vue, wizard-orb-renderer.js, tsx gid
   tables, vite imports).
3. **Mechanical enumeration** — zero-dependency Node script
   (`captures/hash-assets.mjs`) computes bytes + sha256 + dimensions via format
   header parsing (PNG IHDR / JPEG SOFn / WebP VP8X|VP8|VP8L / SVG viewBox).
   Dimension parser validated against documented values (skills webp 128×128 =
   README; monsters.png 2752×64 = 43×64 per actor-art-pipeline; human-v2.png
   256×256 = 4×4 of 64px; SVGs 64×64 = kit viewBox).
4. **Classification** — rubric in FINDINGS §1; UNKNOWN dominates wherever a
   license record is absent; BLOCKED empty because every policy-failing asset
   also lacks license evidence.
5. **Constraints honored** — resource capsule read-only; no downloads; port
   6500 untouched; only files already present hashed; WIZARD/reference assets
   not copied.

## Acceptance command transcripts (literal)

### Acceptance 1 — rg sweep over native/docs surfaces

Command (run literally from repository root):

```text
rg -n "asset|atlas|terrain|splash|orb|png|jpg|bmp" native docs/rebuild docs/product --glob "*.md" --glob "*.cpp" --glob "*.hpp" --glob "*.json"
```

Exit code: **0**. Output: 275 lines; full literal transcript preserved at
`captures/acceptance-1-rg.txt`. Head excerpt:

```text
docs/product\WIZARD_ARCANE_LATTICE_REFERENCE.md:45:| Vessels of Life & Mana | Life/mana orb HUD, status feedback, and visual language | Native simulation owns resource/effect truth; the shader is presentation. |
docs/product\WIZARD_ARCANE_LATTICE_REFERENCE.md:47:| Verdigris Splash | Menu/splash world, atmosphere, and title presentation | Presentation only; no simulation or persistence dependency. |
docs/product\WIZARD_ARCANE_LATTICE_REFERENCE.md:52:`tools/wizard_orbs/README.md`, `tools/rpg_inventory/README.md`,
docs/product\WIZARD_ARCANE_LATTICE_REFERENCE.md:53:`tools/verdigris_splash/README.md`, and `tools/cartographer/README.md`.
```

Tail excerpt:

```text
native\client\main.cpp:6332:    if (!reference_present(first, 1920, 1080, png_1080)) {
native\client\main.cpp:6339:        !reference_present(wide, 1366, 768, png_768)) {
native\client\render_list.hpp:18:  Tile,       // label = "terrain1" | "terrain4"; x/y = projected tile center
```

### Acceptance 2 — machine-readable manifest gate

Command (literal):

```text
node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/captures/assets.json','utf8')); if(!Array.isArray(x.assets)) process.exit(1); console.log('asset manifest: PASS')"
```

Literal output (`captures/acceptance-2-node.txt`):

```text
asset manifest: PASS
```

Exit code: **0**.

### Acceptance 3 — whitespace gate

Command (literal): `git diff --check`

Output: *(empty)* — transcript `captures/acceptance-3-gitdiffcheck.txt`.
Exit code: **0**.

### Acceptance 4 — change-confinement gate

Command (literal): `git diff --name-only`

Output: *(empty — all task deliverables are new files, i.e. untracked until
staged; nothing tracked was modified)* — transcript
`captures/acceptance-4-gitnames.txt`. Exit code: **0**.

Confinement corroboration — `git status --short` at acceptance time:

```text
?? orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/FINDINGS.md
?? orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/captures/
```

All four commands exit 0; changes confined to the owned task folder.

## Evidence inventory

| File | Purpose |
| --- | --- |
| `captures/assets.json` | The manifest: 179 rows × {path, type, dimensions, bytes, sha256, provenance_evidence, license_status, license_evidence, build_package_use, classification} + counts + named negative controls + external candidates |
| `captures/hash-assets.mjs` | Re-runnable enumerator (zero deps, offline) |
| `captures/acceptance-1-rg.txt` … `acceptance-4-gitnames.txt` | Literal acceptance transcripts |
| `FINDINGS.md` | Narrative findings, discrepancies, packaging successor |

## Owner-only questions (explicitly out of audit scope)

1. Font license selection/approval (pixelmix / IBM VGA8 family) — blocks
   TASK-0093 successor and any native text styling that wants those faces.
2. WIZARD orb plate + shader licensing for a future life/mana HUD adapter.
3. Legacy Delaford graphics reuse policy (terrain/objects atlases, item
   atlases, human.png, favicon) under LEGACY_MATRIX REFERENCE_ONLY.
4. Music replacement/removal decision for the undocumented 4.83 MB mp3.
5. Reconciliation inputs: objects.tsx height drift (1056 vs 1024), inventory
   count drift (README says 119 finals; capsule holds 114).
6. Whether to stand up the recommended packaging successor
   (hash-pinned `pack/manifest.json`) and when to intake the external Claude
   demo plates.

## Stop conditions

None hit. No missing-provenance inference, no asset/license/canon mutation, no
network use, no cross-lane branch operations, no force-push.
