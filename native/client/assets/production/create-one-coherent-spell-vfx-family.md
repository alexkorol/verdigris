# VG-ART-006 — WarCry weave family (planning ID)

Reference weave is the existing bronze War Cry aura (TASK-0122 apply/fade),
not a new TASK and not TASK-0108 combat readability.

Cast uses tight rising motes, travel an orbiting ring, impact radial ticks,
and cancel an imploding ember ring. HUD labels stay:

| Beat | Source | Render labels |
|---|---|---|
| cast | `WarCryAura` early life | `vfx-weave:cast` |
| travel | mid life | `vfx-weave:travel` |
| impact | late life | `vfx-weave:impact` |
| cancel | `WarCryFade` | `vfx-weave:cancel` + `warcry-fade` |

Ring radius is capped at 1/6 of the shorter viewport edge so spectacle
cannot blanket a telegraph. A blob or screen-filling ring cannot certify.

Acceptance command: `native/build/verdigris_client.exe --scenario weave-vfx`

Capture: `docs/execution/captures/art-wave/weave-vfx-960x600.png`
