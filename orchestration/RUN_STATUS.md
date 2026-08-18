# Run status (snapshot â€” rewritten each architect sweep, never a diary)

- Current integration tip: origin/codex/native-reconstitution @ 80b6c890
- Master: 80b6c890 (PR #24 N4 parity; PRs 20-23 earlier today)
- Owner-visible target: native client C1 SHIPPED (2D top-down, visible
  combat, inventory). Next: 0051 client harness + 0047 N4 parity.
- Last architect playthrough: 0050 exe, 2026-08-18 ~13:55 (11 driven
  captures â€” rigid world, live combat exchange, inventory verified)
- Immutable objective: D-116 parity, D-110 playable-first

## RUNNING

| Task | Owner | Notes |
|---|---|---|
| 0053 composition polish | kimi | verify-first |
| 0054 client C2 polish | deepseek | claimed post-INC-011 fix |
| 0055 browser follow-ups | kimi-work | claimed after 0047 |

## READY (unclaimed)

| Task | Packet | Notes |
|---|---|---|
| 0051 native client harness | BOUNDED-DESIGN | deepseek expected next (continuous mode); add damage-number scenario per 0050 review nit 2 |

## SHIPPED TODAY (2026-08-18)

- PR #20: 0049 first-session UI wave (deepseek, first-pass, $1.47)
- PR #21: 0042 first-loot moment (kimi, first-pass)
- PR #22: 0050 native client C1 (deepseek, first-pass, architect
  play-gate) + 0052 flake hardening (kimi, first-pass)
- Orchestration v2, camera2d scaffold, loopback binds, branch protection

## WATCH

- `loot` scenario marginal timeout under load (1 sighting via 0052's
  loaded run; 2nd sighting = extend 0052 pattern to loot.mjs).
- KimiWork quota experiment ongoing.
- Queued polish: "House House" pane title doubling (0050 nit 1);
  identity-chip truncation (0049 nit); server-side adventure payload
  to replace client mirror (0049 nit 2).

## Fleet + budget

- deepseek: ~$8 credits; continuous mode; expect 0051 claim. 6540â€“6559.
- kimi K3: continuous mode, on 0053. Monthly pool tight (resets 08-23). 9880â€“9899.
- kimi-work K3: on 0047. 6510â€“6529.
- codex: out of tokens.

## EXP-1 CLOSED (packet-type A/B)

Both arms first-pass with deepseek. Scaffolded MECHANICAL additionally
produced zero scope deviations. RULE (adopted, enforced by spec
authoring practice): scaffold whenever risky math/cross-cutting design
exists; interface-only acceptable for pure presentation.

## Next architect actions

1. Review 0047 on flip (rebuild + 13-attach MYSELF).
2. Review 0053 on flip (captures + gates + verify-first dispositions).
3. Watch 0051 claim; review = run scenario set MYSELF + negative.

