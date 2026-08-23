# FINDINGS — TASK-0121 Owner content approval matrix

- **Lane:** ox-pc-bb (`openrouter/stealth/ox-alpha`)
- **Base commit:** `9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4`
- **Branch:** `worker/verdigris/pc/ox-pc-bb`
- **Date:** 2026-08-23
- **Machine artifact:** [`captures/owner-gates.json`](captures/owner-gates.json)

## Executive summary

Every current or terminal owner-only content decision is inventoried as one of
**15 gates** in `captures/owner-gates.json`, covering all twelve spec domains:
art/assets (G-01), lore/naming (G-02, G-03), magic (G-04), balance (G-05, G-06,
G-11), economy (G-07), campaign content incl. travel (G-08, G-09), season rules
(G-10), bosses/monsters/items/skills (G-11), music (G-12), distribution (G-13),
and irreversible accounts (G-14), plus the provisional content-scale envelope
(G-15). Each gate records an evidence prerequisite, critical-path deadline,
recommended evidence-gathering step, at least two viable decision classes,
an acceptance rubric, dependent tasks/gates, and fallback work.

**No choice is resolved, recommended into canon, or chosen here.** All gates are
`UNRESOLVED_OWNER_ONLY`. Where an owner-input packet documents a recommended
choice, it is quoted as `packet_recommendation` with an explicit not-a-ruling
marker. The evidence boundary is respected throughout: nothing is promoted to
`READY_FOR_OWNER`; that promotion belongs to the architect when named evidence
is accepted.

## Negative control (required by SPEC)

**G-04 — Arcane Lattice production magic direction** is included as a
`PARKED_NONCRITICAL` gate with executable fallback (`TASK-0102` audit +
content-neutral `TASK-0109` scaffolding ship without any magic ruling). G-07
(economy), G-09 (fast travel), and G-10 (season inheritance) carry the same
parked status. This batch therefore stops at the evidence boundary rather than
interrupting noncritical decisions.

## Gate matrix (summary; full detail in JSON)

| ID | Domain(s) | Title | Status | Deadline | Evidence prerequisite |
|---|---|---|---|---|---|
| G-01 | art/assets | Production asset/font policy, provenance, generated assets | WAITING_EVIDENCE (TASK-0093/0094) | before production asset/font promotion + packaging | accepted 0093/0094 inventories + playable plate/font comparison |
| G-02 | lore, naming | Lore canon + world/House/place/character naming | WAITING_EVIDENCE (OI-008 batch) | before production content breadth; before final release | versioned content sets w/ exact source + playable comparison |
| G-03 | naming, lore | `legacyRelicId` / `bronze-dagger` dispositions | WAITING_EVIDENCE (TASK-0085) | before denylist compat cleanup (not Gate B/C path) | occurrence/breakage table proving live consumers |
| G-04 | magic, balance | Arcane Lattice production direction — **negative control** | PARKED_NONCRITICAL | before production skill/magic content | inspected WIZARD material + boundary proposal from 0102/0109 |
| G-05 | balance, items/skills | Combat/progression balance: Brands/Bonds math, recovery odds, loadouts | WAITING_EVIDENCE (OI-008; OD-002/006/008 open) | before production breadth/final release; affix math unfrozen until then | D-114 coherence tables, deterministic loot results, played sessions |
| G-06 | balance | Authoritative passive-tree topology/node source | WAITING_EVIDENCE (TASK-0105) | before TASK-0112 node-content implementation | accepted approximation audit + candidate authored trees |
| G-07 | economy | House crafting/economy boundary, rates, sinks, trade | PARKED_NONCRITICAL | before rates/sinks/trade/passive income/exchange implement | item lifecycle seams + abuse/sink model draft |
| G-08 | campaign | Campaign branch density for 6–30h range | PARKED_NONCRITICAL | before authored multi-act scale | accepted TASK-0096 route/branch measurements |
| G-09 | campaign | Fast-travel/town-portal risk model | PARKED_NONCRITICAL | before fast-travel implementation | travel seam demo + playtest cost/encounter data |
| G-10 | season, campaign | Seasonal inheritance + weather/live-content direction | PARKED_NONCRITICAL | before season reset release | one playable season prototype of the extension boundary |
| G-11 | bosses/monsters/items/skills, balance | Owner-authored rosters approval | WAITING_EVIDENCE (this batch) | before production content breadth; before final release | per-family representative packs staged through validated pipeline |
| G-12 | music | Sound/music direction, licensing, mix, accessibility | WAITING_EVIDENCE (TASK-0117) | before authored audio/music ships | accepted 0117 audit + authored-vs-licensed client comparison |
| G-13 | distribution | Monetization, channel, pricing, platform delivery | TERMINAL_UNRESOLVED | before installer/signing/notarization/public distribution | clean-machine builds + manifests + channel/pricing brief |
| G-14 | irreversible accounts, distribution | Certificates, notarization, publishing, account/GitHub settings, payments | TERMINAL_STANDING_RULE | whenever such action becomes necessary; never delegable | append-only handoff ledger per action |
| G-15 | all content families | Content scale/quality tier selection | PROVISIONAL_ENVELOPE_ACTIVE | before committing production content-breadth investment | accepted vertical slice + normalized runway rates |

## Key findings

1. **The register is already well-formed.** Owner-only decisions live in three
   places that agree: `DECISIONS.md` §Owner-only (D-O1..D-O5), the
   `orchestration/owner-input/` queue (OI-001, OI-003..OI-009), and the
   terminal-gate language of `PROGRAM_GRAPH.md` (T6 "owner-approved content",
   item 13 "irreversible or account-level actions remain owner-only"). No gate
   found outside these sources.
2. **Two terminal gates have no OI packet yet**: distribution/monetization
   (G-13) and irreversible/account-level actions (G-14). They are inventoried
   here so this batch is complete; creating their packets remains architect
   work. Until then the standing rules hold: no agent performs signing,
   publishing, or account changes; reversible packaging work proceeds.
3. **Balance is split across three gates deliberately**: tuned-table approvals
   (G-05), the authoritative passive-tree source (G-06, which also carries a
   separate owner-controlled balance table), and roster/breadth approvals
   (G-11). This matches OI-008's wave-based process and keeps affix math
   unfrozen while evidence accumulates.
4. **Scale tier selection (G-15) silently gates everything else.**
   `CONTENT_SCALE_MATRIX.md` states quantity/scale choices remain owner
   decisions and uses the blockbuster envelope only for planning. Decomposition
   floors under D-128 inherit those counts, so an early owner ruling prevents
   large re-decomposition later.
5. **Nothing blocks.** Every gate names executable fallback work consistent
   with the owner-input README rule ("Unrelated tasks continue per each
   packet's fallback section") and PROGRAM_GRAPH's owner-gates-with-fallback
   table.

## Out-of-scope cross-references (recorded for batch completeness)

- **OI-002 renderer dependency ruling** — technical infrastructure gate on the
  TASK-0114/TASK-0088 ADR path; blocks Stage-2 renderer backend work, not a
  content domain in this SPEC. Tracked in the OI queue.
- **D-111 day/night default** — provisional architect call, flagged for owner
  attention, cheap to reverse; not owner-only.
- **OD-005 transport, OD-010 Legends scope, OD-013 pane settings persistence** —
  engineering/design unknowns resolvable through the normal task ladder unless
  the owner claims them.

## Compliance notes

- Owned paths only: everything in this task folder; no other file touched.
- Resource capsule honored: read-only research, no asset generation, no
  external messages, no ports (6500 untouched).
- No canon chosen or recommended beyond quoting documented packet
  recommendations with explicit non-ruling markers.
