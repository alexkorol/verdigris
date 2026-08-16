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
| Existing passive lattice | REFERENCE_ONLY | Do not bulk-port; native specialization is House-aware and open. |
| Existing world-web routes | REIMPLEMENT | Keep graph ownership and Warden-gated progression as a small native proof. |
| Existing fishing/cooking/mining/smithing defaults | REMOVE | Explicitly denied for native starter scope. |
| Bronze dagger/generic starting coins | REMOVE | Obsolete inherited starter assumptions. |
| WIZARD Arcane Lattice | REFERENCE_ONLY | Record actual design; no generic magic replacement in this sprint. |
| Earlier 2.5D demo | REFERENCE_ONLY | Camera/feel evidence only; archive not present in this checkout. |
| Supplied billboard assets | OWNER_DECISION | Optional later visual pass; first client uses shapes. |
| Legacy tests contradicting constitution | REMOVE | Rewrite/remove rather than restoring denied behavior. |
