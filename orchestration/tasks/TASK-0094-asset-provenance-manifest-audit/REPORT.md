# REPORT — TASK-0094: Native asset provenance manifest audit (lane ox-pc-bc)

- **Task:** TASK-0094 · packet MECHANICAL · topology INDEPENDENT · priority P2
- **Lane:** ox-pc-bc · **Model:** openrouter/stealth/ox-alpha
- **Base SHA (immutable SPEC base):** `d2423873c577d299b3b39c56024d1d840993c72b`
  (`git merge-base --is-ancestor` verified ancestor of worktree HEAD)
- **Claim commit:** `5cdfed6e` ("claim(TASK-0094): asset provenance manifest audit (ox-pc-bc)"),
  pushed to origin before work began
- **Branch:** `worker/verdigris/pc/ox-pc-bc` (worktree `Z:\Code\.worktrees\verdigris\ox-pc-bc`)
- **Deliverables:** `FINDINGS.md`, `captures/assets.json` (179 assets),
  `captures/hash-assets.mjs` (re-runnable enumerator), acceptance transcripts
  `captures/acceptance-1-rg.txt` … `acceptance-4-gitnames.txt`

## 1. Executive summary

Every asset consumed or proposed by native presentation is now inventoried in a
machine-readable manifest with path, type, dimensions, bytes, sha256, provenance
evidence, license status, build/package use, and KEEP/UNKNOWN/BLOCKED class.

- **179 assets** enumerated: 150 KEEP, 29 UNKNOWN, 0 BLOCKED.
- Native-consumed set is tiny and fully green: nine procedural SVGs +
  manifest + generated `visual_kit.h` (project-original, deterministic TASK-0141
  generator) and four triage evidence captures. The native runtime performs no
  asset file I/O; kit art is compile-time embedded.
- All WIZARD plates/shader, browser fonts, legacy Delaford tile/item atlases,
  the legacy human sprite, menu music, and favicon are UNKNOWN (missing license
  or provenance evidence). None was copied; none will be inferred.
- Negative control satisfied: named UNKNOWN assets remain non-shippable
  (`pixelmix.ttf` et al. — see FINDINGS §4).
- Four discrepancies recorded for owner action, including stale Tiled metadata
  (`objects.tsx` says 1024 px tall; the PNG measures 1056 px) and an inventory
  README count drift (119 claimed vs 114 present).

## 2. Approach

1. Preflight per AGENTS.md (clean tree; upstream sync 0/0; base ancestor-verified).
2. Consumer survey across native + browser code to fix the "consumed/proposed"
   boundary (grep families: visual_kit, manifest.json, svg paths, fonts.scss,
   wizard orb imports, tile atlases/tsx, skills webp, item/inventory art, mp3,
   favicon).
3. Provenance survey of in-repo evidence docs (DCSS-ATTRIBUTION, inventory/
   skills/mining READMEs, actor/player pipeline docs, CLAUDE_DEMO_ASSET_INTAKE,
   LEGACY_MATRIX, prior audits TASK-0093/TASK-0117/TASK-0035/TASK-0116/TASK-0141).
4. Mechanical enumeration via zero-dependency Node script
   (`captures/hash-assets.mjs --write`): sha256 whole-file; dimension parsing
   from PNG IHDR / JPEG SOFn / WebP VP8X|VP8|VP8L / SVG viewBox; family metadata
   table embedded in-script for reproducibility.
5. Classification under the rubric in FINDINGS §1; negative control asserted by
   the script itself (`if (!counts.UNKNOWN) throw`).
6. Acceptance commands run literally from repo root; transcripts captured.

Constraints honored: resource capsule read-only (no asset/license/manifest/canon
modified), no network downloads, port 6500 untouched, changes confined to
`orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/**`.

## 3. Acceptance command transcripts (literal)

All commands executed from repository root
(`Z:\Code\.worktrees\verdigris\ox-pc-bc`) in PowerShell 5.1.

### 3.1 rg sweep — exit code 0

```
$ rg -n "asset|atlas|terrain|splash|orb|png|jpg|bmp" native docs/rebuild docs/product --glob "*.md" --glob "*.cpp" --glob "*.hpp" --glob "*.json"
exit code: 0   (275 match lines)
```

Full literal output captured at `captures/acceptance-1-rg.txt`. First lines:

```
docs/product\WIZARD_ARCANE_LATTICE_REFERENCE.md:45:| Vessels of Life & Mana | Life/mana orb HUD, status feedback, and visual language | Native simulation owns resource/effect truth; the shader is presentation. |
docs/product\WIZARD_ARCANE_LATTICE_REFERENCE.md:47:| Verdigris Splash | Menu/splash world, atmosphere, and title presentation | Presentation only; no simulation or persistence dependency. |
docs/product\WIZARD_ARCANE_LATTICE_REFERENCE.md:52:`tools/wizard_orbs/README.md`, `tools/rpg_inventory/README.md`,
...
native\client\main.cpp:6331:    const std::string png_768 = out_dir + "\\" + scene.name + "-1366x768.png";
native\client\main.cpp:6332:    if (!reference_present(first, 1920, 1080, png_1080)) {
native\client\render_list.hpp:18:  Tile,       // label = "terrain1" | "terrain4"; x/y = projected tile center
```

### 3.2 node JSON parse gate — exit code 0

```
$ node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/captures/assets.json','utf8')); if(!Array.isArray(x.assets)) process.exit(1); console.log('asset manifest: PASS')"
asset manifest: PASS
exit code: 0
```

Transcript: `captures/acceptance-2-node.txt`.

### 3.3 git diff --check — exit code 0 (no whitespace errors)

```
$ git diff --check
(no output)
exit code: 0
```

Transcript: `captures/acceptance-3-gitdiffcheck.txt`.

### 3.4 git diff --name-only — exit code 0 (empty)

```
$ git diff --name-only
(empty: all deliverables are new files, staged/untracked, nothing modified in place)
exit code: 0
```

Transcript: `captures/acceptance-4-gitnames.txt`.

### 3.5 Confinement evidence (supplemental)

```
$ git status --short
?? orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/FINDINGS.md
?? orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/captures/
```

Only owned-path evidence exists outside the claim commit; no file outside
`orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/**` is created,
modified, or deleted.

## 4. Negative control verification

`captures/assets.json → counts`: `{"total":179,"KEEP":150,"UNKNOWN":29,"BLOCKED":0}`
and `negative_controls_named` lists ten concrete non-shippable assets, headed by
`src/assets/fonts/pixelmix.ttf` (sha256 `8016c197…`, full hash in the manifest).
The enumerator hard-fails if the UNKNOWN bucket would ever be empty.

## 5. Owner-only questions (explicitly not decided here)

1. Font licensing/selection for the native text layer (TASK-0093 dependency;
   pixelmix/PxPlus/Px437 all currently UNKNOWN).
2. WIZARD orb plate/shader licensing for a future life/mana HUD adapter.
3. Legacy Delaford atlas reuse decision (terrain/objects/items/human/sword-icon:
   REFERENCE_ONLY today with no license record).
4. Music policy: replace or document/remove `main_menu.mp3`.
5. Reconciliation of §6 discrepancies in FINDINGS (tsx height drift; inventory
   119-vs-114 count).

## 6. Handoff

STATUS.md flipped to REVIEW_REQUESTED with the frozen pushed head recorded after
this report's commit lands. Successor guidance (packaging manifest design,
intake rules, attribution travel) lives in FINDINGS §7.
