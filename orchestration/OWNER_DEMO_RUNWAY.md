# Owner Demo executable runway

Authority: owner interview captured in VERDIGRIS_VISION.md and OWNER_DEMO_OVERNIGHT.md at the parent workspace. Packet base: 3d3588126e3abc228721fbed0ff3f8d7cae66448.

## Counts

- 43 new formal task packets: TASK-0166 through TASK-0208.
- 14 immediately READY across client models, content, and asset/tooling lanes.
- 28 AUTO_RELEASE successors plus 1 final DRAFT release/correction packet.
- 0 initial owned-path collisions by construction; integration hotspots are sequenced.

## Required first wave

| Pri | Task | Lane | Owner-visible contribution |
|---|---|---|---|
| P0 | TASK-0166 | assets/tooling | WIZARD source/provenance manifest |
| P0 | TASK-0167 | assets/tooling | Framekit raster slice pack |
| P0 | TASK-0168 | assets/tooling | WIZARD orb raster pack |
| P0 | TASK-0169 | assets/tooling | RPG Inventory item-art pack |
| P0 | TASK-0170 | client model | menu/Escape state model |
| P0 | TASK-0171 | client model | inventory grid model |
| P0 | TASK-0172 | client model | paper-doll model |
| P0 | TASK-0173 | client model | actor animation model |
| P0 | TASK-0174 | client model | attack VFX model |
| P0 | TASK-0175 | client model | gate interaction model |
| P0 | TASK-0176 | client model | instance refresh model |
| P0 | TASK-0177 | content | town content seed |
| P0 | TASK-0178 | content | multi-zone graph content |
| P0 | TASK-0179 | assets/tooling | splash asset pack |

## Automatic successor graph

| Task | Dependency release | Outcome |
|---|---|---|
| TASK-0180 | 0167 accepted plus current-tip validation | Framekit render adapter |
| TASK-0181 | 0168 accepted plus current-tip validation | orb render adapter |
| TASK-0182 | 0169 accepted plus current-tip validation | item-art render adapter |
| TASK-0183 | 0170+0179+0180 accepted plus current-tip validation | splash/menu integration |
| TASK-0184 | 0171+0172+0180+0182 accepted plus current-tip validation | grid inventory/paper doll integration |
| TASK-0185 | 0181 accepted plus current-tip validation | orb HUD integration |
| TASK-0186 | 0173 accepted plus current-tip validation | animated actor integration |
| TASK-0187 | 0174+0186 accepted plus current-tip validation | attack arcs/trails integration |
| TASK-0188 | 0175+0178 accepted plus current-tip validation | readable gate integration |
| TASK-0189 | 0176+0188 accepted plus current-tip validation | persistent instance refresh integration |
| TASK-0190 | 0177 accepted plus current-tip validation | town/NPC runtime integration |
| TASK-0191 | 0166+0178 accepted plus current-tip validation | Cartographer native adapter |
| TASK-0192 | 0178+0191 accepted plus current-tip validation | multi-zone runtime |
| TASK-0193 | 0166 accepted plus current-tip validation | geometric skill-tree model |
| TASK-0194 | 0193 accepted plus current-tip validation | first level-up tree integration |
| TASK-0195 | 0166 accepted plus current-tip validation | bounded spell-lattice model |
| TASK-0196 | 0194+0195 accepted plus current-tip validation | first lattice choice integration |
| TASK-0197 | 0180+0190 accepted plus current-tip validation | Chronicles Owner pane integration |
| TASK-0198 | 0169+0184 accepted plus current-tip validation | first Brand crafting loop |
| TASK-0199 | 0198 accepted plus current-tip validation | Bond progress visibility |
| TASK-0200 | 0177 accepted plus current-tip validation | House-vs-Scion model |
| TASK-0201 | 0190+0200 accepted plus current-tip validation | House-vs-Scion integration |
| TASK-0202 | 0169+0192 accepted plus current-tip validation | recovered item loop |
| TASK-0203 | 0178+0186+0187 accepted plus current-tip validation | village-defense prologue |
| TASK-0204 | 0157+0203 accepted plus current-tip validation | Owner Demo audio beats |
| TASK-0205 | integrated journey dependencies accepted plus current-tip validation | fifteen-minute journey gate |
| TASK-0206 | 0205 accepted plus current-tip validation | visual fidelity comparison |
| TASK-0207 | 0205 accepted plus current-tip validation | performance budget |
| TASK-0208 | 0205+0206+0207 accepted plus current-tip validation | release candidate/correction wave |

## Coordinator rules

Claim only READY packets. Re-run board-sentinel before dispatch. After each ACCEPTED result, validate every newly dependency-satisfied successor against the current head, replace likely paths with exact owned/forbidden paths, stamp base/capsule/commands, and promote only a collision-free wave. Preserve expired workers and reclaim from their last durable commit. Never allow an empty ready queue: when fewer than eight effective READY tasks remain, promote validated successors or split a non-hotspot fallback before releasing more workers.

## Product acceptance

The wave converges on TASK-0205, the automated fifteen-minute Owner journey, then TASK-0206 visual comparison, TASK-0207 performance evidence, and TASK-0208 release/correction. No task count or passing unit suite substitutes for the integrated journey and npm run playtest.

