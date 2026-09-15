# Lineage import checkpoint — 2026-09-10

96 additive runtime poses and96 unique attachment entries are staged for gameplay review: male/female × eight directions × idle, walk0, walk1, strike0, strike1, strike2. Strike2 is a byte-identical idle recovery. The selected80 source images remain unchanged in candidates; originals and derived alpha cutouts were copied with hashes into the runtime source package.

Actual Pixel Respecter `pixel_perfecter.workspace.reconstruct` produced every non-copied pose. Each direction uses its idle source anchor, common10px pitch,160×192 native canvas, anchor(80,160), and a96-color cycle palette. No pose was independently scaled or fitted. RGB checker/matte cleanup and the reconstructed geometry are recorded in derived repair reports and directional provenance.

`validation.json` passes96 PNGs/96 metadata keys, binary alpha, canvas/margins, shared palette limits, source/derived/runtime/equipment hash consistency, unchanged reconstruction dimensions, and exact16 idle recoveries. This is artifact validation, not final gameplay acceptance. Root reported the catalog gate passed272 total assets including96 lineage assets.

All16 idles were inspected at native/3x and approved by root. All32 motion loops were played in the browser; final combined walk0/walk1 and attack contact/recovery were observed after the last source import. Per-direction native/3x strips and source-wrist crosshair overlays were also inspected. Final maleNE and femaleNE/NW repairs visibly alternate support; maleSE/SW repairs visibly change support and preserve the left striking hand. Source generation failures remain retained by the artist packages.

Remaining visible limitations: E/W side-view foot ownership is weaker under overlapping silhouettes; two walk contacts have no passing phase; NE poses shift their crown/sole registration by several native pixels; femaleNW hair/body changes and some extra cloth texture remain. MaleN/NE contact is a high reach. These are provisional gameplay candidates, not polished full animation cycles. Wrist points are artists' approximate anatomical-left coordinates transformed through the actual importer. The visible crosshair is a wrist/grip estimate; root must still inspect the rendered held-object angle, occlusion and finger cover in both GPU and legacy paths.

## Deliverables

- `processed/{male,female}/{direction}/`:96 normalized PNGs including recovery copies.
- `native/client/assets/raster/runtime/hero-lineage-20260910.provenance.json`: combined source/repair/hash evidence;16 directional reports in the sibling directory.
- `native/client/lineage_equipment.inc`:96 aggregate entries; SHA25678538f5d94f176d06d87593e80eecd21a615ef13554377e66e2aa4f4f938c9c2.
- `equipment-candidate.json`: exact source coordinates, geometry transform, hashes, layering/angle caveats.
- `approvals.json`: hash-pinned root-authorized gameplay candidate staging.
- `previews/global-playback.html`: combined walk/attack playback; all32 clips available. Local server http://127.0.0.1:8873 remains running for root/user review, PID16204.
- `previews/male-female-idle-eight-1x.png`, `all-sixteen-{walk,attack}-{1,2}x.gif`: compact review assets.

Runtime PNGs, source copies and equipment include are frozen at this checkpoint. No main/UI/GPU/core edits or native build were performed for this asset task. Parent owns the supported build and production acceptance. Earlier appearance-selection API validation remains in `.ci-artifacts/appearance-selection/REPORT.md`.
