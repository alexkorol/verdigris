# FINDINGS — TASK-0094: Native asset provenance manifest audit

- **Lane:** ox-pc-bc · **Model:** openrouter/stealth/ox-alpha
- **Base SHA:** `d2423873c577d299b3b39c56024d1d840993c72b` (ancestor-verified of worktree HEAD)
- **Claim head:** `5cdfed6e` on `worker/verdigris/pc/ox-pc-bc`
- **Machine-readable manifest:** `captures/assets.json` (179 assets; every field required by the SPEC: relative path, type, dimensions, bytes, sha256, provenance evidence, license status, build/package use, classification)
- **Frozen invariants respected:** no asset, license, package manifest, or product canon changed; only files already present were hashed; no network downloads; WIZARD and browser-reference assets are recorded as candidates and were not copied anywhere.
- **Stop rule honored:** where provenance or license evidence is missing, the asset is classified `UNKNOWN` with empty license evidence — no license was inferred for any asset.

---

## 1. Scope and method

**Consumed by native presentation today** = the procedural visual kit:
nine SVG sources + `manifest.json`, geometry embedded into
`native/client/assets/generated/visual_kit.h`, which is the sole asset
dependency of the native build (`native/client/main.cpp:42` include, drawn at
`main.cpp:1041-1047`). `rg "fopen|ifstream|CreateFile|LoadImage" native/client`
finds only capture-integrity probes of *output* PNGs — the native client reads
**no asset files at runtime**.

**Proposed by native presentation (candidates)** = WIZARD plates/shader and
browser-reference assets named by the constitution
(`docs/product/VERDIGRIS_CONSTITUTION.md:130-155`: Vessels orbs, Brands & Bonds
inventory art, splash, plus fonts per TASK-0093) and by
`docs/rebuild/CLAUDE_DEMO_ASSET_INTAKE.md`.

Exact enumeration commands (run from repo root):

```text
node orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/captures/hash-assets.mjs --write
```

Hashing: sha256 whole-file (`node:crypto`). Dimensions: format header parse
(PNG IHDR / JPEG SOFn / WebP VP8X|VP8|VP8L / SVG viewBox); `null` where the
format has no raster box (fonts, mp3, xcf/kra archives, json/h/frag).

### Classification rubric

| Class | Meaning |
| --- | --- |
| KEEP | Provenance AND license status documented in-repo (project-original, or PD/CC0 with attribution). Shippable pending ordinary review. |
| UNKNOWN | Provenance or license evidence missing. Non-shippable until owner records it. No inference made. |
| BLOCKED | Evidence complete but an independent rule (policy/format) forbids shipping as-is. |

BLOCKED is intentionally empty this pass: every asset that fails policy also
lacks license evidence, so it lands in UNKNOWN first. The one third-party-
derived asset with complete evidence (DCSS-derived `dungeon.png`) passes as
KEEP because `src/assets/tiles/DCSS-ATTRIBUTION.md` documents PD/CC0.

## 2. Family summary

Full per-file detail lives in `captures/assets.json`. Bytes are decimal MB.

| Family | Files | KEEP | UNKNOWN | MB | Provenance anchor |
| --- | ---: | ---: | ---: | ---: | --- |
| native-visual-kit-svg | 9 | 9 | 0 | 0.02 | TASK-0141 `generate-assets.mjs` seeded generator (`visual_kit.h:1`; role table `generate-assets.mjs:450-461`) |
| native-visual-kit-manifest | 1 | 1 | 0 | 0.01 | generatorVersion `task0147-gen-2`; emitted by same generator |
| native-visual-kit-header | 1 | 1 | 0 | 0.04 | checked-in generated artifact consumed at compile time |
| native-triage-captures | 4 | 4 | 0 | 1.12 | TASK-0035 REPORT.md:78-82; cited by TASK-0116 matrix |
| wizard-orb-plates | 6 | 0 | 6 | 1.72 | consumed by `src/core/hud/wizard-orb-renderer.js:8-13`; upstream WIZARD license NOT recorded here |
| wizard-orb-shader | 1 | 0 | 1 | 0.02 | browser GLSL only; constitution: shader stays presentation |
| browser-fonts | 4 | 0 | 4 | 0.14 | `fonts.scss:2-18`; TASK-0093 FINDINGS.md:232 already flags licensing unresolved |
| tiles-dungeon-atlas | 1 | 1 | 0 | 0.19 | DCSS rltiles PD/CC0 (`DCSS-ATTRIBUTION.md`) + project mining tiles |
| tiles-mining-sources | 3 | 3 | 0 | 0.01 | generation workflow+prompt in `sources/mining/README.md` |
| tiles-legacy-atlases | 5 | 0 | 5 | 2.21 | Delaford-era; LEGACY_MATRIX.md:9 REFERENCE_ONLY; no license doc |
| skills-icons (+source sheet) | 7 | 7 | 0 | 3.20 | `skills/README.md` documents generation and crops |
| actors-shipping-atlases | 2 | 2 | 0 | 0.33 | `docs/actor-art-pipeline.md` builder + source sheets |
| actors-source-sheets | 3 | 3 | 0 | 4.73 | named inputs in actor-art-pipeline.md:8-10 |
| players-production-sheet | 1 | 1 | 0 | 0.06 | full prompt/provenance in `docs/player-art-pipeline.md:38-62` |
| players-master (chroma) | 1 | 1 | 0 | 1.42 | retained master per player-art-pipeline.md:61 |
| players-legacy-human | 1 | 0 | 1 | 0.00 | unused Delaford sprite; generation reference only |
| item-atlases-legacy | 10 | 0 | 10 | 1.04 | Delaford-era atlases + `.xcf`; original README is a GIMP how-to only |
| inventory-art-wizard | 117 | 117 | 0 | 7.55 | `inventory/README.md`: project AI imagery via WIZARD prototype, commit chain recorded |
| audio-menu-music | 1 | 0 | 1 | 4.83 | no composer/license record; flagged bloat docs/archive/code-review.md:229 |
| favicon | 1 | 0 | 1 | 0.00 | `index.html:7` only; no provenance record |
| **Total** | **179** | **150** | **29** | **28.9** | |

## 3. Negative control — UNKNOWN assets remain non-shippable (named)

The SPEC's negative control requires at least one UNKNOWN asset to remain
non-shippable and be named. Twenty-nine exist; these ten are named explicitly:

1. `src/assets/fonts/pixelmix.ttf` — no license file anywhere in repo; TASK-0093 flagged unresolved.
2. `src/assets/fonts/pixelmix_bold.ttf` — same.
3. `src/assets/fonts/PxPlus_IBM_VGA8.ttf` — same.
4. `src/assets/fonts/Px437_IBM_PS2thin2.ttf` — same (also dead declaration: `UIFont` has no consumer).
5. `src/assets/audio/music/main_menu.mp3` — origin/composer undocumented.
6. `src/assets/tiles/terrain.png` — legacy Delaford graphics, REFERENCE_ONLY, license undocumented.
7. `src/assets/tiles/objects.png` — same (plus stale tsx metadata, §5).
8. `src/assets/graphics/actors/players/human.png` — legacy sprite, zero runtime consumers.
9. `src/assets/orbs/wizard/art.png` (+ 5 sibling plates and `wizard-orb.frag`) — WIZARD prototype; upstream license not recorded here.
10. `public/sword-icon.png` — favicon with no provenance record.

Per the stop rule, hashing/classification continued for all other assets while
these stay unresolved; nothing was inferred and nothing was copied.

## 4. What native packaging may consume today

- The **only** asset bytes inside the shipped native executable are the
  procedural shapes/palettes embedded in `visual_kit.h` (KEEP chain complete:
  deterministic generator + seeds checked in at TASK-0141).
- Everything else is either evidence (triage captures) or candidate material
  owned by browser reference code paths (vite imports, CSS url(), server tsx
  gid tables). No CMake target copies, installs, or packages any asset
  (`rg "install|package|copy|asset" native/CMakeLists.txt` → comment-only hit).

## 5. Discrepancies found during audit (recorded, not fixed)

1. **Stale Tiled metadata:** `server/maps/layers/objects.tsx` declares
   `objects.png` as 288×1024, but the PNG IHDR is **288×1056** (33 tile rows,
   not 32). Either the atlas gained a row without updating the tsx, or the tsx
   predates the current atlas. Owner should reconcile before any native tile
   adoption relies on those gids.
2. **Inventory count drift:** `src/assets/inventory/README.md` says "119 item
   finals"; this capsule holds **114** files under `items/`. Hashes here are
   authoritative for what exists; owner intake reconciliation recommended.
3. **Dead font declaration:** `UIFont` → `Px437_IBM_PS2thin2.ttf` in
   `fonts.scss:2-6` has no consumer (TASK-0093 noted the same).
4. **Bundle bloat carryover:** `main_menu.mp3` is 5,068,686 bytes and remains
   statically imported (`AudioMainMenu.vue:37`) despite the archived review
   flag (`docs/archive/code-review.md:229`).

## 6. External candidates not hashable in this capsule

The 22 Claude-demo billboard plates (~50 MB) described by
`docs/rebuild/CLAUDE_DEMO_ASSET_INTAKE.md` live outside the repository and the
resource capsule; they cannot be hashed here without violating the read-only /
no-download constraints. They are recorded as UNKNOWN candidates in
`captures/assets.json` (`external_candidates_not_hashed`); the intake doc
already defers vendoring until provenance, size, and packaging are approved.

## 7. Packaging successor (required by SPEC)

A future implementation task (owner-gated) should:

1. Take `captures/assets.json` as intake truth: only KEEP entries are
   vendorable; every UNKNOWN needs a documented license BEFORE selection.
2. Extend `generate-assets.mjs` (or a sibling packager) to emit a versioned,
   content-hashed bundle manifest (`slot id -> sha256 -> bytes`) so the
   CLAUDE_DEMO intake's promised "content hashes + versioned slot schema"
   materializes inside owned tooling.
3. Keep the native boundary: runtime stays file-free; packaging embeds or
   sidecars hashed blobs resolved by the renderer adapter, never by absolute
   paths from simulation state (constitution §Native architecture invariant).
4. Carry attribution files with any adopted third-party-derived asset
   (currently exactly one family: DCSS-derived `dungeon.png` +
   `DCSS-ATTRIBUTION.md`).
5. Resolve the §5 discrepancies before locking any tile gid or item-art slot
   mapping into native content.

## 8. Exact acceptance commands

See `REPORT.md` for literal transcripts with exit codes. Commands executed
literally from the repository root:

```powershell
rg -n "asset|atlas|terrain|splash|orb|png|jpg|bmp" native docs/rebuild docs/product --glob "*.md" --glob "*.cpp" --glob "*.hpp" --glob "*.json"
node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/captures/assets.json','utf8')); if(!Array.isArray(x.assets)) process.exit(1); console.log('asset manifest: PASS')"
git diff --check
git diff --name-only
```
