# TASK-0156 REPORT — native passive-tree progression visibility

## Executive summary

The shipped `I` gear overlay now shows the authoritative passive-tree
progression that already lived silently in the server payload. The existing
`passiveTree` envelope (schemaVersion 2) is mirrored verbatim into plain
client model fields and rendered as one compact line — earned/unspent skill
points plus committed node/conduit counts. Absence is stated as absence, never
rendered as zero; zero and nonzero states are distinct surfaces. No node ids,
allocation actions, invented copy, or test-only render behavior were added.
No server, simulation, protocol-schema, save, balance, or passive-tree
authority file changed.

State: REVIEW_REQUESTED.

## Approach

1. **Mirror, don't derive** (`client_model.hpp`): new plain struct
   `ClientPassiveProgression { present, unspent_points (points.skill),
   earned_points (earned), node_count (nodes entries), conduit_count
   (conduits entries) }` and a `ClientModel::progression` field.
   `present=false` until any authoritative payload arrives — the tri-state
   anchor required by the spec.
2. **Production parser seam** (`remote_session.cpp`): a single helper
   `apply_passive_tree()` copies only payload-borne values. Wired at all
   three authoritative delivery points on the existing wire:
   - `player:login` → `data.player.passiveTree`
   - `dev:state` → `data.state.passiveTree`
   - `player:skilltree:update` → `data.passiveTree` (new handler for this
     already-existing server event)
3. **Presentation mirror** (`presentation_state.hpp/.cpp`):
   `WorldView::progression` copied verbatim in `sync_world_from_model`;
   `sync_world_from_simulation` leaves it default-absent because the local
   core carries no tree authority.
4. **Shipped surface** (`main.cpp`, `paint_gear_overlay`): one line anchored
   above the Banked footer (y = bottom−70 inside the pane), recorded on the
   existing `render::Op::PaneStat` vocabulary with a `TREE ` label prefix so
   scenarios can assert it. Text:
   - present: `TREE pts <unspent>/<earned>  nodes <n>  conduits <m>`
   - absent: `TREE no authoritative data`
5. **Scenario** (`main.cpp`, registered as `progression-surface`): binds an
   in-process `WebSocketServer` inside THIS lane's routed capsule
   **7120–7139** and drives the real remote session end to end:
   - absent: pre-start model has `present == false`; pane renders the absence
     line;
   - nonzero: real quick-guest admission + zone entry; asserts mirrored
     numbers are consistent (`unspent > 0`, `unspent <= earned`,
     `node_count >= 1`) and the pane text shows the authoritative points;
   - zero: drives the existing browser-wire event `player:skilltree:save`
     through the documented `send_raw` test-harness escape hatch so the REAL
     server commits a fully-spent snapshot and replies with a genuine
     `player:skilltree:update` payload; asserts `unspent_points == 0`,
     `node_count == 3`, `conduit_count == 0`, pane renders real zeros and
     never the absence phrase;
   - emits both required PNG captures from the real GDI paint path
     (`reference_present`) into this task's captures/ folder.

## Changed files (exactly the SPEC-owned paths)

- `native/client/client_model.hpp`
- `native/client/remote_session.cpp`
- `native/client/presentation_state.hpp`
- `native/client/presentation_state.cpp`
- `native/client/main.cpp`
- `orchestration/tasks/TASK-0156-native-progression-visibility/**`
  (STATUS.md, REPORT.md, captures/)

No forbidden path was touched. During the full gate, pre-existing scenarios
regenerated a peer task capture
(`TASK-0144/0145 .../expedition-hud-960x600.png`); it was restored via
`git checkout --` so the final diff stays inside owned paths.

## Public interfaces added/changed

- Added: `verdigris::client::ClientPassiveProgression` (plain struct),
  `ClientModel::progression`, `WorldView::progression`.
- Changed: gear overlay draws one additional readout line (new behavior of an
  existing pane; no op vocabulary change — reuses `render::Op::PaneStat`).
- Client now also consumes the pre-existing `player:skilltree:update` event
  (read-only mirroring).
- No wire, schema, server, or simulation interface changed. No allocation
  command or UI exists.

## Test commands + outcomes (literal transcripts)

### 1. Full native gate

```
PS Z:\Code\.worktrees\verdigris\ox-pc-aa> powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
session tests passed
presentation events tests: PASS
== scenario move-and-camera ==            PASS (0 failures)
== scenario first-fight ==                PASS (0 failures)
== scenario loot-to-bank ==               PASS (0 failures)
== scenario telegraph-dodge ==            PASS (0 failures)
== scenario combat-juice ==               PASS (0 failures)
== scenario remote-render-list ==         PASS (0 failures)
== scenario zoom-invariance ==            PASS (0 failures)
== scenario chronicles-gate-b ==          PASS (0 failures)
== scenario first-session-clarity ==      PASS (0 failures)
== scenario animation-vfx-phase-a ==      PASS (0 failures)
== scenario progression-surface ==
    ok: progression-surface: bound ox-pc-aa capsule server
    ok: absent: no passiveTree payload has arrived
    ok: absent: the gear pane states absence, not zeros
    ok: nonzero: session start
    ok: nonzero: handshake ready
    ok: nonzero: expedition entered
    ok: nonzero: authoritative payload mirrored into the model
    ok: nonzero: unspent and earned points are nonzero
    ok: nonzero: unspent never exceeds earned
    ok: nonzero: the committed node count mirrors the payload
    ok: nonzero: the pane text shows the authoritative points
    ok: nonzero: a present payload never renders as absence
    ok: zero: the committed mirror shows zero unspent points
    ok: zero: genuine zeros are rendered as zeros
    ok: zero: zero is not rendered as absence
    ok: progression-surface: capture readable
    capture: .\orchestration\tasks\TASK-0156-native-progression-visibility\captures\progression-surface-nonzero-960x600.png (489149 bytes)
    ok: progression-surface: capture is non-trivial
    ok: progression-surface: capture readable
    capture: .\orchestration\tasks\TASK-0156-native-progression-visibility\captures\progression-surface-zero-1366x768.png (855028 bytes)
    ok: progression-surface: capture is non-trivial
   PASS (0 failures)
```

Every pre-existing client scenario is preserved and green, including
`first-session-clarity` which proves the Esc-first pane-dismissal contract
("first Esc closes the pane, client stays alive", "bare Escape requests
application exit").

### 2. Focused scenario (SPEC command)

```
PS Z:\Code\.worktrees\verdigris\ox-pc-aa> native/build/verdigris_client.exe --scenario progression-surface
   …identical transcript to the progression-surface block above…
   PASS (0 failures)
EXIT=0
```

### 3. Diff hygiene (SPEC commands)

```
git diff --check        -> exit 0 (no output)
git diff --name-only    ->
  native/client/client_model.hpp
  native/client/main.cpp
  native/client/presentation_state.cpp
  native/client/presentation_state.hpp
  native/client/remote_session.cpp
```

## Manual verification (captures inspected before handoff)

Both PNGs under `orchestration/tasks/TASK-0156-native-progression-visibility/captures/`
were opened and visually inspected from the real GDI paint path:

- `progression-surface-nonzero-960x600.png`: gear overlay shows
  `TREE pts 1/2  nodes 1  conduits 0` (fresh level-1 admission: earned 2,
  root-committed spent 1). Legible above `Banked items 0 trophies 0` and the
  controls line; no collision with backpack cells, banked state, controls, or
  the top HUD.
- `progression-surface-zero-1366x768.png`: same pane after the server
  commits the spent snapshot: `TREE pts 0/2  nodes 3  conduits 0` — genuine
  zeros, visibly different from the absence surface proven in step 1.

Pane geometry is resolution-independent (panel bottom clamps to top+430), so
the line sits clear of collisions at both 960x600 and 1366x768.

## Commit SHAs (this worker branch)

- claim: `e3f4bdf6212e5bdcfa66bcb033fc304d74b28ffd`
- implementation: `518d0b9e85f23d104804ab6a641c5c7a340326cf`
- evidence/review-request: `<this commit>`

Base at provisioning: `c2b814488278f4f093e754cf695ea9ed749d81fb`
(SPEC base `ad1a1e178e689df442d4655937f8e8e037cf4cd2` is an ancestor).

## Deviations

- None functional. Two ownership-driven choices worth noting:
  - `render_list.hpp` is NOT an owned path, so no new render op was added;
    the summary records on the semantically-correct existing
    `render::Op::PaneStat` ("stats readout line") with a distinctive `TREE `
    label prefix, asserted by prefix match exactly like the existing
    `render_list_has` helpers.
  - The ZERO state is produced by driving the ALREADY-EXISTING
    `player:skilltree:save` wire event through the documented test-harness
    escape hatch against the real in-process protocol server, so the zero
    payload comes from the production authority rather than being fabricated
    by the test. No allocation command, UI, or client-side rule was created.

## Unresolved questions

- None blocking. The summary intentionally reports `nodes`/`conduits` counts
  verbatim instead of a server-derived "spent" figure, keeping every displayed
  number a direct mirror of payload fields.

## Risks / follow-ups

- If the owner later ships authored node names/effects or an allocation UI,
  this line is the natural anchor point but remains deliberately minimal now
  per the spec's no-invention rule.
- Pre-existing observation (not introduced by this task): the always-on HUD
  controls hint can overdraw the pane title row behind the overlay; unchanged
  historical layering, out of scope here.

## STOP conditions

- Not triggered. The current payload supports an honest points/allocation
  summary without any server change.
