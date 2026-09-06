# VG-ART-004 — tin village environment kit (planning ID)

Village route `route:tin:1:0` assembles one kit: dwelling, shrine, tree,
ruin, and a **non-solid** gate. Pivots are world positions. Collision
radii are `SceneryItem.radius` published on `render::Op::Scenery` and as
`collision-proxy:<kind>` HUD ops for **solid** pieces only.

The dressing gate must not appear as `collision-proxy:gate`. Movement
blocking uses the same `solid` set (`scenery_blocks_segment`).

A circle-on-stick lollipop cannot certify the kit. Shipped trees are a
forked bole with a root flare and clustered canopy.

Acceptance command: `native/build/verdigris_client.exe --scenario kit-chunk`

Capture: `docs/execution/captures/art-wave/kit-chunk-960x600.png`
