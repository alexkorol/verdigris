# VG-ART-004 — tin village environment kit (planning ID)

Village route `route:tin:1:0` assembles one kit: dwelling, shrine, tree,
ruin, and a **non-solid** gate. Pivots are world positions. Collision
radii are `SceneryItem.radius` published on `render::Op::Scenery` and as
`collision-proxy:<kind>` HUD ops for **solid** pieces only.

The dressing gate must not appear as `collision-proxy:gate`. Movement
blocking uses the same `solid` set (`scenery_blocks_segment`).

A circle-on-stick lollipop cannot certify the kit. Shipped trees are a
forked bole with a root flare and clustered canopy. A scalloped market
stall cannot certify a dwelling; shipped houses have walls, thatch, and
a door. A covered wagon cannot certify a ruin; shipped ruins have a
broken wall and rubble. A stone blob cannot certify a shrine; shipped
shrines are a fountain (basin, column, water). A solid slab cannot
certify a gate; shipped gates have two pillars, a lintel, and an opening.
The shrine and dressing gate sit inside the spawn capture frustum.

Acceptance command: `native/build/verdigris_client.exe --scenario kit-chunk`

Capture: `docs/execution/captures/art-wave/kit-chunk-960x600.png`
