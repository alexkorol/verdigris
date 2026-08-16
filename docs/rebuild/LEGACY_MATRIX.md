# Legacy firewall matrix

The browser game and Delaford remain playable historical references. This matrix
controls what may cross into the new native production workspace.

| Area | Classification | Rule |
|---|---|---|
| Browser Vue/Node runtime | REFERENCE_ONLY | Keep playable; do not mechanically port. |
| Delaford maps, graphics, NPCs, terminology | REFERENCE_ONLY | Reuse only after an explicit product decision. |
| Existing Chronicles House/Scion concept | REIMPLEMENT | Preserve the durable House/Scion loop in the native model. |
| Existing shared Str/Dex/Int actor concepts | REIMPLEMENT | Use one native actor schema for players and enemies. |
| Existing Vesselforge catalogue/formulas | OWNER_DECISION | Stable item identity/history is required; exact formula is open. |
| WIZARD Vessels of Life & Mana orbs | REIMPLEMENT | Intended HUD/presentation integration; simulation remains authoritative. |
| WIZARD Brands & Bonds inventory | REIMPLEMENT | Intended item/UI integration through the native identity/history model. |
| WIZARD Verdigris Splash | KEEP | Intended menu/splash presentation foundation, not core simulation. |
| WIZARD Cartographer map generator | REIMPLEMENT | Intended seeded content adapter after native collision/route validation. |
| Existing passive lattice | REFERENCE_ONLY | Do not bulk-port; native specialization is House-aware and open. |
| Existing world-web routes | REIMPLEMENT | Keep graph ownership and Warden-gated progression as a small native proof. |
| Existing fishing/cooking/mining/smithing defaults | REMOVE | Explicitly denied for native starter scope. |
| Bronze dagger/generic starting coins | REMOVE | Obsolete inherited starter assumptions. |
| WIZARD Arcane Lattice | REFERENCE_ONLY | Record actual design; no generic magic replacement in this sprint. |
| Earlier 2.5D demo | REFERENCE_ONLY | Camera/feel evidence only; archive not present in this checkout. |
| Supplied billboard assets | OWNER_DECISION | Optional later visual pass; first client uses shapes. |
| Legacy tests contradicting constitution | REMOVE | Rewrite/remove rather than restoring denied behavior. |

## Provenance addendum — 2026-08-16 (from the TASK-0005 archaeology audit)

Evidence base: `orchestration/tasks/TASK-0005-legacy-archaeology-audit/REPORT.md`
(file/line citations and commit archaeology). Key ruling: **the
Delaford/Verdigris boundary is file-level, not directory-level** — treat
individual catalogues, constants, and schemas as tagged evidence; never
assume all of `server/` is legacy or all of `src/` is current.

Refinements to the table above:

| Area (audit-precise) | Classification | Rule |
|---|---|---|
| `{event,data}` socket envelope + handler dispatch (`server/Delaford.js`, `server/socket.js`) | REFERENCE_ONLY | A native network adapter translates this protocol; the socket server never enters the simulation. |
| Curated Verdigris item bases (`server/core/data/items/verdigris.js`) | KEEP (as data) | Verdigris-era curated content; extract as data for native item specs, not as code. |
| Vesselforge pack + item schema (`server/core/items/vesselforge/verdigris-pack.js`, `engine.js` schema + seeded Mulberry32) | KEEP (as data/schema evidence) | Feeds the future item-formula decision (owner-held); the serializable pack and schema are legitimate native inputs. |
| Legacy item catalogues (`server/core/data/items/{weapons,armor,belts,jewelry,general,vessels}.js`) | REMOVE | Delaford residue; useful only as provenance for rejected IDs. |
| Monster catalogue/archetypes/rarities (`server/core/data/monsters/`, `server/core/monsters/`) | KEEP (as data) | Verdigris-era shared STR/DEX/INT curves and archetypes; extract as data respecting actor symmetry. |
| Shared stats rules (`server/shared/stats/index.js`) | REIMPLEMENT | Direct evidence input for the native actor schema. |
| World-web graph (`server/core/world-web.js`) and seeded instance recipes (`server/core/map.js`) | REIMPLEMENT | Deterministic hash/PRNG design carries; extract recipes as data. |
| Hand-authored Delaford surface maps (`server/maps/surface.tmx`, layer JSON) | REFERENCE_ONLY | Delaford world imagery/topology; no automatic reuse. |
| Chronicles SQLite schema (`server/core/repositories/chronicles-repository.js`) | REFERENCE_ONLY | Design input for native persistence (future ADR-002); not a port source. |
| Skills/quickbar registry (`server/shared/skills/index.js`) | REFERENCE_ONLY | Data-shaped definitions inform native skill design (core trio landed via TASK-0007). |
| Balance constants (`server/core/combat/index.js`, `server/shared/combat.js`) | KEEP (as data) | Measured pacing values are legitimate tuning inputs. |
| Retired gathering/anvil code paths (`server/core/data/helpers/database.js`) | REMOVE | Already retired in browser; denylist (hardened in TASK-0008) enforces native absence. |

Known documentation drift (do not trust without checking):
`docs/chronicles-persistence.md` describes JSON-file persistence as the
default while the authoritative repository defaults to SQLite
(`server/core/repositories/chronicles-repository.js`).
