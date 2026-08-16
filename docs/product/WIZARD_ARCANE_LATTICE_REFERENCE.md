# WIZARD Arcane Lattice reference

Inspected from the public `alexkorol/WIZARD` repository (`gh-pages`,
`tools/arcane_lattice/README.md` and `index.html`) on 2026-08-15.

This is an actual self-contained Three.js r128 spellcrafting interface, not the
Pixel Alchemy falling-sand sandbox. It describes a connected path through a
diamond lattice with 16 immutable slots and seven strata:

```text
Origin/Source
→ Alignment (Adversarial, Natural)
→ Focus (Destructive, Sustaining, Creative)
→ Element (Fire, Air, Water, Earth)
→ Sphere (Body, Spirit, Mind)
→ Reach (Outer, Inner)
→ Manifestation
```

The canonical adjacency is `adjT`; base-tier weaving only follows directly
adjacent channels. Paradigm Shift swaps two nodes within one stratum (three
refundable uses). Schism rotates six satellites across two nexuses (two
refundable uses). Dimensional Ascension adds channels at Tier 1/Vessel and
complete neighboring-stratum connectivity at Tier 2/Tesseract, with power and
instability costs. Displaced nodes retain `NATIVE`, add power and instability,
and produce hybrid-weave flavor.

The public API is `window.ArcaneLattice` with `onCast`, `getState`,
`setState`, `setMaxTier`, and `getSpell`. The README explicitly labels
`genSpell` as a placeholder for the real effect system and leaves progression
gating to the host game. Per-node VFX profiles are treated as identity, not
polish.

Verdigris therefore records this as design evidence only. The native sprint does
not copy its Three.js implementation, assume a mana wizard, or confuse it with
the separate Pixel Alchemy tool.

## Other WIZARD modules with an intended Verdigris path

The Arcane Lattice is reference-only for this sprint, but the wider toolbox has
four components explicitly intended to mesh with the game:

| WIZARD module | Intended Verdigris role | Boundary |
|---|---|---|
| Vessels of Life & Mana | Life/mana orb HUD, status feedback, and visual language | Native simulation owns resource/effect truth; the shader is presentation. |
| Brands & Bonds / RPG Inventory | Vessel-slot inventory, item history, Brands, Bonds, attunement, and awakening | Native item identity/history owns rules; the browser React shell is not authoritative. |
| Verdigris Splash | Menu/splash world, atmosphere, and title presentation | Presentation only; no simulation or persistence dependency. |
| Cartographer | Seeded deterministic instance/map generation | Content adapter; validate connectivity, spawns, collision, and route semantics against the native runtime. |

The public module descriptions were inspected from the WIZARD repository's

`tools/wizard_orbs/README.md`, `tools/rpg_inventory/README.md`,
`tools/verdigris_splash/README.md`, and `tools/cartographer/README.md`.
