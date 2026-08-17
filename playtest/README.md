# Playtest harness (the goal harness)

**If you are an AI coding agent working on this game: run `npm run playtest`
before declaring anything playable.** Unit tests prove functions; this proves
the game. It boots a real server and *plays* it — moves, fights, loots,
spends points, enters every zone — over the same WebSocket protocol the
browser uses.

```bash
npm run playtest                 # boot a fresh server on :6510, run all scenarios
npm run playtest -- combat loot  # run a subset
npm run playtest -- --attach     # reuse the running dev server (ws://localhost:6500)
```

Exit code 0 = every scenario passed. Each scenario prints its assertions.

## Load-stressed verification

The parity gate is also exercised while the machine is busy. On PowerShell,
the following starts one CPU spinner per available worker (leaving one worker
for Node), runs the full gate, and always cleans up the spinners:

```powershell
$workers = @(); 1..([Math]::Max(1, [Environment]::ProcessorCount - 1)) | ForEach-Object { $workers += Start-Process -FilePath node -ArgumentList @('-e', 'for (;;) { Math.sqrt(Math.random()); }') -PassThru -WindowStyle Hidden }; try { $env:PLAYTEST_LOAD_MODE = '1'; npm run playtest } finally { Remove-Item Env:PLAYTEST_LOAD_MODE -ErrorAction SilentlyContinue; $workers | Stop-Process -Force -ErrorAction SilentlyContinue }
```

Repeat the command for the required consecutive-run evidence. The runner
prints the load-adaptive timing guard and event-loop diagnostics; a missing
server event still fails within the explicit 1.75x cap. In load mode only,
server-side instance admission uses a bounded 12s authored floor (21s after
the cap) because the child server can be starved independently of this
client. Ordinary runs keep each scenario's authored deadline. The load-mode
session-arc final-death observation similarly uses a bounded 20s authored
floor (35s after the cap), with periodic adjacent-target retries. These
load-mode floors are only active when the documented CPU-load command opts in.

## Why this exists

Five shipped bugs — a skill tree that forgot allocations, healers that made
packs unkillable, WASD that died after clicking UI, right-click menus that
never opened, monsters two-shotting at-level players — all lived behind a
green 400+ unit-test suite. They were only findable by *playing*. This
harness makes playing scriptable, so "it works" claims are cheap to verify
and expensive to fake.

## Playing the game from code

```js
import HeadlessPlayer from './playtest/harness.mjs';

const p = await HeadlessPlayer.connect();      // guest login, like the dev account
const s = await p.state();                     // ONE authoritative snapshot:
                                               // x/y, hp, scene, inventory, wear,
                                               // passiveTree, monsters, groundItems,
                                               // stairs, layout, lifecycle

await p.move('down', 3);                       // walk like a held key
await p.enterZone('crypt', 'gauntlet');        // Adventure menu equivalent
await p.attack(s.monsters[0]);                 // swing toward a target (auto-attack sustains)
const menu = await p.rightClick(x, y);         // REAL server-built context menu
await p.takeItem(groundItem);                  // right-click → Take → wait for pickup
p.useSkill('ability-1', 'left');               // quickbar equivalent
p.saveSkillTree(snapshot);                     // what the tree pane persists

await p.waitFor(async () => (await p.state()).hp.current < 50, { label: 'took damage' });
p.close();
```

## Wiz/dev mode (scenario setup in seconds)

Dev-only server commands (`NODE_ENV !== 'production'`), also usable from any
WS client or the browser console:

| verb | effect |
|---|---|
| `p.devTeleport(x, y)` | jump anywhere; landing on a portal walks through it |
| `p.devGive('bronze-sword', 1)` | grant items through the real inventory pipeline |
| `p.devSetLevel(10)` | set level + refresh derived stats |
| `p.devHeal()` | full HP/mana |
| `p.state()` | the `dev:state` snapshot above |

## Scenarios = the core-loop checklist

| scenario | proves |
|---|---|
| `economy` | moving shop/bank NPC → real pane menus → buy/sell/deposit/withdraw |
| `movement` | steps move the player; entering a zone mid-walk doesn't bounce |
| `combat` | a pack **with a healer** can actually be killed, and you survive |
| `loot` | kill → drop → real context menu → Take → in inventory |
| `party` | two logins → invite/accept → ready → shared instance → leave/return cleanup |
| `zones` | every Adventure zone: right layout, stairs, populated, named; stairs-up returns you to your pre-entry tile |
| `skilltree` | a saved build survives a full relog |
| `crossroads` | scion spawns at their House wagon; town is sanctuary + monster-free; road purse, deposits, treasury-funded outfitting |
| `gates` | each of the four road gates opens its own Wayfinder's Chart without moving you |
| `world-web` | chart → zone travel, Warden bars the onward gate, Warden death unlocks children + persists, cleared zone stays cleared on re-entry |

Add a scenario per new feature: one file in `playtest/scenarios/`, default
export `async ({ connect, assert }) => {…}`. Throw (or fail an assert) to fail.

## What this does NOT cover — the browser boundary

The harness drives **server truth over the real protocol**. It cannot see
client-side rendering or input binding bugs (dead Vue event bindings, canvas
focus traps, stale HUD labels). After UI changes, do a short browser pass
(`npm run dev`, http://localhost:5173 — the Login button is prefilled;
Claude Code should use its preview tools with the `delaford-dev` config
instead of a raw shell): move with WASD **after clicking a UI element**,
right-click the canvas and an inventory item, open/close the skill tree.
