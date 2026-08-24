# REVIEW — TASK-0168 wizard-orb-raster-pack

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~09:10 PDT
- head reviewed: 5b8c03f7 (single task commit on
  codex/TASK-0168-wizard-orb-raster-pack-cursor; already ancestor of the
  program branch, zero drift on owned paths since)
- verdict: **ACCEPTED — INTEGRATED**

## Evidence

- 10/10 manifest entries (6 layers + 4 states) independently verified:
  sha256, byte size, and stated dimensions against actual image headers
  (PNG IHDR; JPEG SOF0 for the five JPG plates). Mode claims consistent.
- Contact sheet genuine art: 5016x706, four panels, both orbs in
  full/low/reserved/empty states.
- Life-plate-only question resolved: at WIZARD 66a5d9f there is exactly
  one shared plate set; life-vs-mana is procedural in src/orb.frag
  (lifeLiquid :78, manaLiquid :227). SPEC satisfied; downstream kind
  differentiation is shader/tint work for TASK-0185 (matches 0181's
  review assumption).
- Provenance: 5/6 plates + 3 state PNGs byte-match git-tracked WIZARD
  blobs at the pinned commit; harness + corrupt negative control
  reproduced; denylist PASS; scope exact (17 files, owned paths only).

## Advisories (non-blocking)

1. manifest.json:6 module_id "wizard.wizard-orbs" disagrees with
   TASK-0166 registry id "wizard.orbs" — string inconsistency, follow-up
   correction.
2. The orb mask's sha256 matches only the untracked WIZARD
   tmp/orbs-original/extracted file, not any blob at the pinned commit
   (mask was iterated via fix_mask.py). Bytes are preserved and hashed in
   this repo so integrity holds, but the commit pin overstates
   reproducibility for that one file.
3. STATUS lacked frozen-head SHA at flip (recurring template gap).
4. Trivia: states/empty.jpg duplicates layers/empty_glass.jpg bytes;
   branch ox/TASK-0168 is misnamed (contains TASK-0170 work), not a
   duplicate claim.
