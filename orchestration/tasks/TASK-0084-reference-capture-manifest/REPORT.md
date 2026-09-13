# TASK-0084 REPORT — acceptance transcripts

- task: TASK-0084 · state: REVIEW_REQUESTED (see STATUS.md)
- lane: ox-pc-bb · model: openrouter/stealth/ox-alpha
- branch: `worker/verdigris/pc/ox-pc-bb` · claim commit `30a96556`
- SPEC base_commit `d2423873c577d299b3b39c56024d1d840993c72b` verified ancestor
  of routed HEAD `d20305e4` via `git merge-base --is-ancestor` (exit 0).
- Deliverables: `orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs`
  (dependency-free verifier, node: builtins only) and
  `orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.json`
  (30 frozen entries: 25 images + 5 native render-list JSONs).
- Resource capsule honored: image evidence read-only (never decoded into pixels,
  never rewritten); zero binary dependencies added; no ports bound or probed;
  port 6500 untouched; worker writes confined to owned paths
  (`reference-manifest.mjs`, `reference-manifest.json`, task folder).
- Transcript capture note: all transcripts below were captured in one session
  pass against the final tool code at worker head `91630905`, AFTER the
  deliverables were committed and BEFORE this `REPORT.md` file existed.
  `--write` regenerated `reference-manifest.json` byte-identically (the
  post-run `git status --short` prints nothing), proving manifest generation
  is deterministic and evidence-neutral.

## Coverage summary

The five-scene matrix pins, per scene `01-route-entrance`,
`02-pack-combat`, `03-elite-telegraph`, `04-named-drop-gear`,
`05-critical-health`:

| side | files | expected resolution | source revision |
| --- | --- | --- | --- |
| native captures | `native-after/NN-<scene>-1920x1080.png` (5) | 1920x1080 | `77b21764c043585f5da55d3c9008361f55e30d2b` |
| native render lists | `../../tasks/TASK-0070-reference-scenes/captures/NN-<scene>.json` (5) | 1920x1080 declared + structural rules | `0a0144207cccfdf6024ebc467c7d81b2462b2c52` |
| browser reference | `browser-NN-<name>-1920x1080.png` (5) | 1920x1080 | `ec7d88ccf429a1b9d21572ab84f048af40501f95` |
| composite gen 1 | `sxs-NN-<name>.jpg` (5) | 1920x568 | `ec7d88ccf429a1b9d21572ab84f048af40501f95` |
| composite gen 2 | `sxs2-NN-<name>.jpg` (5) | 1920x568 | `77b21764c043585f5da55d3c9008361f55e30d2b` |
| composite gen 3 | `sxs3-NN-<name>.jpg` (5) | 1920x568 | `6a3a46c33c218aaca38048c297f3806c66567447` |

Verifier failures covered (each observed or by construction): missing files,
duplicate manifest entries, zero-byte files, wrong resolution (header-parsed,
no pixel decode), malformed PNG/JPEG headers and render-list JSON, sha256 /
byteLength / metadata mismatches, unmanifested images on disk, unexpected
manifest entries, and unsupported schema versions.

## 1. `node --check orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs`

```text
EXIT=0
```

Exit code: **0** (syntax check silent, as expected).

## 2. `node orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs --write`

```text
wrote Z:\Code\.worktrees\verdigris\ox-pc-bb\orchestration\benchmarks\side-by-side-2026-08-20\reference-manifest.json: 30 entries (10 native, 5 browser, 15 composite)
EXIT=0
```

Exit code: **0**. Regenerated ONLY `reference-manifest.json`; all 25 image
files and the 5 render-list JSONs were only read. The regeneration was
byte-identical to the committed manifest (see §6).

## 3. `node orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs --verify`

```text
verification OK (Z:\Code\.worktrees\verdigris\ox-pc-bb\orchestration\benchmarks\side-by-side-2026-08-20\reference-manifest.json): 30 entries (10 native, 5 browser, 15 composite)
EXIT=0
```

Exit code: **0**. Read-only verification of all 30 entries: paths present,
sizes/hashes match, PNG IHDR / JPEG SOF dimensions parse and match the naming
matrix, every render-list JSON parses with scene id, 1920x1080 declaration,
and non-empty typed ops, and no unmanifested image exists under the benchmark
directory.

## 4. `git diff --check`

```text
EXIT=0
```

Exit code: **0** (no whitespace/conflict-marker errors; silent as expected).

## 5. Authentic negative — copied manifest with one bad hash

Procedure (disposable copy inside the benchmark directory so relative paths
resolve; removed immediately after):

```powershell
Copy-Item "orchestration\benchmarks\side-by-side-2026-08-20\reference-manifest.json" "orchestration\benchmarks\side-by-side-2026-08-20\reference-manifest-negative-copy.json"
# corrupt exactly one recorded sha256 (browser-01-route-entrance entry)
(Get-Content $bad -Raw).Replace("87ac3584e549c442ed6707149e8179f871565e4208de7804c9b93730b86e1173", "deadbeefe549c442ed6707149e8179f871565e4208de7804c9b93730b86e1173") | Set-Content -NoNewline $bad
```

Command: `node orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs --manifest reference-manifest-negative-copy.json --verify`

```text
corrupted one sha256 in disposable copy
FAIL: sha256 mismatch for browser-01-route-entrance-1920x1080.png: recorded deadbeefe549c442ed6707149e8179f871565e4208de7804c9b93730b86e1173, actual 87ac3584e549c442ed6707149e8179f871565e4208de7804c9b93730b86e1173
verification FAILED with 1 error(s) against Z:\Code\.worktrees\verdigris\ox-pc-bb\orchestration\benchmarks\side-by-side-2026-08-20\reference-manifest-negative-copy.json
NEGATIVE EXIT=1
disposable copy removed
```

Exit code: **1** (non-zero, as required). The disposable copy was then
deleted; `git status --short` confirmed the tree returned to clean and no
image evidence was touched at any point.

## 6. Post-write idempotence probe

```text
=== tree state after --write ===
STATUS_EXIT=0
```

`git status --short` printed nothing after §2's `--write`: regenerating the
manifest over committed evidence produces identical bytes, so the checked-in
manifest cannot silently drift from disk truth.

## Stop conditions

Not triggered: verification reads headers and bytes only (no pixel decode, no
image rewriting) and uses exclusively node: built-ins (`crypto`, `fs`,
`path`, `url`) — zero new dependencies, binary or otherwise.
