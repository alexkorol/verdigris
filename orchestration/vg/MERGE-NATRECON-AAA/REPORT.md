# MERGE-NATRECON-AAA — merge candidate report

**Status:** PROPOSAL — awaits architect/owner ratification (GOV-002 unstamped). Nothing pushed.
**Author:** kimi-work (coordinator), 2026-09-06.
**Branch:** `kimiwork/merge-natrecon-into-aaa` in `Z:\Code\Games\delaford\kimiwork_verdigris`.

## Parents

| Side | SHA | Subject |
|---|---|---|
| Base (ours) | `e7b65360` | origin/codex/goal-aaa-systems tip — AAA server lane (account persistence, remote skills/HUD, guided House/Scion creation, charted expeditions; ~39 commits over merge-base `486058f3`) |
| Merged (theirs) | `193b7c9f` | origin/codex/native-reconstitution tip — nat-recon presentation wave (Cursor's ~30 commits: HUD certification, vector art, audio mixer paint, main.cpp rewrite) |

## Commits on the candidate

1. `7bee3970` — the merge. Conflict resolution policy: preserve BOTH feature sets; additive-only resolutions; no test weakened. 9 files needed semantic resolution: `docs/rebuild/HANDOFF.md` (union of entries), `native/client/client_model.hpp`, `main.cpp`, `presentation_events.hpp`, `presentation_state.cpp/.hpp`, `remote_session.cpp`, `ui_skin.hpp` (both paint paths retained, aaa server-driven model + nat-recon skin/raster chrome), `native/src/networking.cpp` (aaa emit/account arms + nat-recon `state.xp` snapshot block — the `xp` key survives verbatim at :1564).
2. `ab6cf5e4` — semantic fixups found by build+tests: equipped items dual-listed into both `world.worn` (aaa wear-set seats) and `world.carried` (nat-recon grid/compare/held rig); null-simulation guard in `nearest_pickup_id` (hand-built scenario worlds carry no sim); vital-orbs fallback-only draw (both palettes alpha-blended otherwise); held-item rig scans both lanes.
3. `5148c580` — cleanup: drops `.merge-scratch/` debris accidentally committed inside `7bee3970` by the first (step-limited) worker.

## Gate outcomes (all real runs on this tree, this machine)

| Gate | Result |
|---|---|
| `native/build.ps1 -RunTests` | exit 1 twice, exit-0 components: denylist/core/networking/camera2d/audio all PASS; **sole failure both times: `session_tests` gate-b "slain rare guardian surfaces the circulating heirloom"** |
| `verdigris_session_tests.exe` standalone | **exit 0, 3/3 runs** (plus exit 0 earlier pre-cleanup) |
| `verdigris_core_tests.exe` / `verdigris_networking_tests.exe` | exit 0 |
| `npm run playtest` (browser harness) | **exit 0, 32/32 scenarios** |
| `native/build.ps1 -RunClientScenarios` | exit 5 → 4 failing scenarios (below) |
| `git diff --check` | exit 0 |

**gate-b flake:** "successor fell to ordinary combat" during the relic-pickup leg under full-gate
load only; standalone 3/3 green. Same signature was observed at the UNMODIFIED baseline
`e7b65360` by two earlier lanes this week (documented adds-during-pickup flake,
`session_tests.cpp:1590-1607` note). Third/fourth sightings now — recommend the standing
watch-item follow-up harness task. Cursor was actively running on this machine during these
runs (ambient load).

## Known failures routed to the client lease holder (Cursor, `native/client/main.cpp` lease ACTIVE)

Four presentation scenarios fail on the merged tree (nat-recon scenarios meeting merged HUD geometry):

1. `hud-pane-readability` — controls vs objective / vs minimap clearance at 960x600 (the merged Pixelmix face measures wider than either parent's layout tuning).
2. `endgame-tablet-ui` — contextual controls overflow the pane right inset.
3. `pane-stack` — live HUD must name depth 2, not only the helper.
4. `telegraph-spec` — remote consumer must store the catalog window.

An interrupted worker produced substantive, well-commented fixes that repaired (2) and (3) but
regressed `warden-disciplines`; the diff is preserved UNCOMMITTED-AS-FILE at
`orchestration/vg/MERGE-NATRECON-AAA/client-fixes-attempted.patch` for the lease holder to mine
or discard. I deliberately did not iterate further inside the active `main.cpp` lease.

## Limitations

- kimi-work branches NOT in this merge (they stack after ratification):
  `kimiwork/TASK-0108-ranged-rev3` (`3b929637`, ranged combat core+wire per D-129),
  `kimiwork/VG-SEC-001-json-bounds` (`3ac661a7`), `kimiwork/VG-SEC-002-road-tier-recursion`
  (`6e5188bc` — fixes the unbounded `web_tier_width` recursion that EXISTS UNFIXED on BOTH
  parents: aaa networking.cpp:1291 and nat-recon networking.cpp:756),
  `kimiwork/VG-TOOLS-001-content-validator` (`8fb41e53`),
  `kimiwork/VG-SAVE-001-profile-inventory` (`980dcd9d`).
- `vg-sec-002`'s sibling sink in `server/core/world-web.js:128` (JS stack) remains open.
- The merge does not reconcile product SEMANTICS where the two lines diverge in intent
  (dual House models per VG-SAVE-001 G1/G5) — it preserves both behaviors.
