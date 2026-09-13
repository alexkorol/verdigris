# REVIEW — TASK-0181 native-orb-render-adapter

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~08:25 PDT
- head reviewed: 06625ba6 (branch
  codex/TASK-0181-native-orb-render-adapter-cursor, parent 084e3160;
  already ancestor of the program branch)
- verdict: **ACCEPTED, conditional on TASK-0168 acceptance** (SPEC
  dependency; 0168 still in review).

## Evidence

- Harness PASS, reproduced independently: 17 checks + legacy denylist
  green.
- Scope exact: native/client/orb_renderer.hpp (+140) + task dir. Frozen
  surfaces untouched; native boundary compliant (pure constexpr planner,
  determinism covered by checksum stability test).
- Substance genuine: LayerId enum maps 1:1 to the six TASK-0168 plates and
  manifest roles (native/client/assets/wizard/orbs/manifest.json:11-66,
  WIZARD provenance pinned, per-layer sha256).

## Conditions / advisories

1. TASK-0168 ACCEPTED required first.
2. plan_orb ignores OrbState.kind (orb_renderer.hpp:101-124) — Life and
   Mana produce identical plans; TASK-0185 (orb HUD) must tint by kind.
   Record this in 0185's review checklist.
3. Ratio <=80 (<=8%) maps to Empty band and hides the liquid plate even
   when current > 0 (orb_renderer.hpp:85-90,:111) — a live player at 5% HP
   renders an empty orb. 0185 must confirm against WIZARD reference
   visuals before this ships player-visible.
4. plan_checksum (:126-138) is a weak XOR determinism probe — never
   promote it to a persisted/compared artifact.
5. Process: STATUS.md lacks frozen head SHA; tighten lane template per
   BUS.md.
