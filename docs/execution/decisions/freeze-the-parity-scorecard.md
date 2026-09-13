# VG-GOV-003 — Freeze the parity scorecard

Draft 2026-09-06. Extends TASK-0152 / TASK-0119 / TASK-0206 evidence; does
not mint TASK numbers and does not claim commercial parity. Owner still
approves thresholds. VG-GOV-002 remains unstamped.

## Negative control

Counting shipped features, VG IDs, or TASK packets cannot produce a
parity pass. A dimension without a named journey, measurement, and
accountable approver stays OPEN.

## Dimensions (journey + measurement + approver)

| Dimension | Journey / command | Measurement | Approver |
|---|---|---|---|
| Input | `--scenario input-latency`, `eight-way`, `aim-hold` | p50/p95 input-to-present; eight-way names; held aim | Cursor Grok (client) |
| Threat / telegraph | `--scenario telegraph-spec`, `attack-beat` | Catalog ticks+reach; sim event beats | Cursor presentation; core ACT is Kimi |
| Build distinctness | `--scenario build-fixtures` | Three named loops; tinted melee clones fail | Cursor sheet; core BUILD is Kimi |
| Reward / loot | `--scenario loot-filter` | Hide cannot mutate ownership | Cursor facts; ITEM sim is Kimi |
| Navigation | `--scenario dressing-pass`, route card in HUD | Dressing hash ≠ topology | Cursor dressing; WORLD sim is Kimi |
| HUD vitals | `--scenario vital-orbs` | Life left/red, mana right/blue; mute is a chip | Cursor Grok |
| UI scale | `--scenario hud-scale-floor`, `hud-pane-readability` | Type floor; pane vs orbs | Cursor Grok |
| First session | `--scenario first-session-clarity` | Slay wardens + dash on F3-off HUD; local walk-on rejected | Cursor Grok |
| Persistence | Kimi SAVE / House journeys | Restart retains House value | Kimi |
| Audio mix | `--scenario dense-mix`, `combat-audio` | Event-id mix; not a silent schedule | Cursor sink |
| Frame budget | `--scenario frame-budget` | <40 ms avg at 3440×1440 | Cursor Grok |
| GPU trial | `--scenario gpu-sample` (VG-GOV-005 / TASK-0114) | Software sample ≠ engine migration | Cursor GPU lane |

A zero on integrity, lethal-threat readability, or input response cannot
be averaged away by a high score elsewhere.

## Existing packets

Reuse TASK-0152 (benchmark), TASK-0119 (first-session audit), TASK-0206
(Owner Demo capture). Do not re-spec TASK-0108 or Owner Demo journeys.

## Status

Scorecard **drafted**. Threshold numbers above that already have machine
gates (frame-budget 40 ms) stay frozen. Other pass bars remain owner
stamps. Feature-count parity is REJECTED.
