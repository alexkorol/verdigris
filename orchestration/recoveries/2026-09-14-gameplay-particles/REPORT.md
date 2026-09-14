# Gameplay particles — native implementation and handover

Source: `30ed52c842cc0500db2b3cf0a6397454c52410c8` (particle implementation `c191b7ee6`, render-copy fix `30ed52c84`).
Checkout: `C:/Users/Alex/Documents/ChatGPT/verdigris-consolidated-20260913`.
Working branch: `codex/native-consolidated-20260913`.
Shared development baseline: `codex/native-reconstitution`.
Normal entry: `C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe`.

## Implemented

Four external particle recipes now accompany existing playable actions: two expanding bronze-gold War Cry rings with rising flecks, bounded dust/grit along the accepted dash segment, brighter scattered critical-contact sparks/slivers, and inward/upward ground-pickup motes. Existing level-up, melee, death, dust, and fire/trail recipes remain intact. No combat damage/cost, save format, menu, equipment or Crossroads return rules were replaced.

The server emits `player:buff-applied` only after the existing War Cry resource check and `player:pickup-confirmed` only after real ground admission. Pickup quantity excludes overflow. Automatic coins use the same explicit confirmation. Foreign/unknown buff events and foreign/malformed pickup events are ignored. Inventory refresh, grants, login, banking, and rejected actions do not fabricate these particles. Existing presentation events still serve the rest of the UI.

The CPU layer adds a bounded ring shape (seeded angular phase, evenly spaced world-XY points, configurable radius and radial velocity). Rendering retains the shared 64x64 point-sampled atlas, 32-emitter/384-particle limits, two optional lights and existing depth ordering. There is no additional fullscreen pass. Detached particles stay where emitted; route changes/disconnect clear transient state.

## Verification

`powershell -NoProfile -File native/build.ps1 -RunTests`: PASS, all eight native suites. Added tests cover paid/rejected War Cry, real automatic pickup of 37 coins, no pickup event from grants/empty ground, real-socket confirmation delivery and malformed/foreign event rejection. Existing progression, movement, combat, inventory, extraction and persistence tests remain green.

Both focused client scenarios pass. `gameplay-particles` creates a real native server/socket session, stages an open forest position and one disposable item, then sends real War Cry, dash, drop, and pickup commands. Those resulting effects are captured by production perspective paint. It rejects repeated dash feedback, checks no pickup particles on the fixture grant, verifies ring geometry/detachment and full cleanup, and measures 258 live particles at 3440x1440 under the unchanged 40 ms average gate. The critical-contact image is a controlled presentation-event fixture, not a claim that a random critical was earned in that screenshot. Existing menu-particles still checks actual level-up presentation, all original recipes, caps and retirement; real earned XP events remain covered by networking tests.

Production screenshots were inspected at their rendered 1920x1080 size. War Cry is a readable expanding double ring, dash leaves restrained ground dust and grit along the accepted segment, pickup motes gather around the feet, and critical sparks separate from the body without obscuring the actor. The first development pass failed because the dash recipe named a nonexistent `smoke` tile. That reference was corrected to the actual four smoke atlas frames. Asset loading now preserves the real validation error and never falls through to another checkout when a present package is invalid; the scenario exits safely on missing assets. These were fixed before the clean source commit.

The final clean package at `native/build/player-package-30ed52c84/` passed all **82 exact packaged scenarios**, package integrity (1285 files, 363 required runtime resources), and both top-level launcher lifecycles. The same executable and same isolated QA profile saved and reloaded Effects **70%**. The full package rechecked perspective, authored actor animation, movement/attacks, House/Scion button flow, title/Continue, inventory, Crossroads return and settings. Final packaged timing: 258 particles **37.449 ms**, sustained inventory drag **36.519 ms** (39.433 ms peak), moving fullscreen **24.3 ms**, all below the unchanged 40 ms average gate. Final packaged particle captures were inspected separately. See `evidence/verification.json`, logs and PNGs.

Entry SHA256: `e1aaad7d90c76b82bf8de2a42ba83f64bd3f5c52c898018a3c354e6bdb43539d`.
Client SHA256: `e07572b307da52093089e921bee4713322cd3205d8e3b1d98d803f06a7685240`.

The first clean candidate (`c191b7ee6`) failed the complete gate on sustained inventory drag, 40.055 ms against the unchanged 40 ms bound. It was never promoted. The renderer still copied every GPU frame through a second full-size CPU buffer before drawing to the native DIB. The follow-up copies directly from GPU staging to the DIB and creates an owned diagnostic snapshot only when requested. GPU staging is retained; no caller pointer is retained. Byte-for-byte checks verify direct output, snapshots after the caller overwrites its pixels, and snapshot freshness after a changed frame. Focused verification after this code change measured drag at 36.175 ms and 258 particles at 26.053 ms. No bound was raised and no resolution/visual quality was reduced.

## Preservation and remaining scope

The verified package was promoted into the same normal installation after checking no owner process and acquiring its exclusive profile lock. All previous installation files and saves were retained at `native/build/normal-launch-rollback-before-30ed52c84/`. The normal entry was then actually launched via the application-owned title-only smoke. Source, child client/server paths, working directory, normal profile, perspective startup, menu Quit and clean shutdown passed. One existing save was **byte-identical before and after normal launch**, and normal settings stayed absent. No QA profile was promoted. The normal 3440x1440 title capture was inspected at original size and shows build `30ed52c842cc`; it remains locally at `native/build/gameplay-particles-installed-evidence/normal-title.png`.

No Computer Use session or desktop input was needed for build/verification. App-owned native scenarios and launcher diagnostics drove disposable identities and the normal-entry title-only smoke. No owner game was killed. Raw owner saves are retained only in local rollback/evidence, never committed.

Future content work remains as previously recorded: playable fire-vessel content/tag, per-frame hand sockets, Burning Touch gameplay and authoritative flying projectiles. Their existing recipes and API are preserved; this change does not pretend that the old ranged telegraph is a flying projectile. Those content extensions are separate from the four implemented playable-action effects.

The verified implementation through `30ed52c84` was pushed atomically to both `codex/native-consolidated-20260913` and `codex/native-reconstitution`; `git ls-remote` confirmed both exact 40-character SHAs. Subsequent handover commits change only documentation/evidence, not packaged game code. No failed check or running verification remains. Continue authorized implementation and ordinary verified branch pushes without inventing another acceptance checkpoint.
