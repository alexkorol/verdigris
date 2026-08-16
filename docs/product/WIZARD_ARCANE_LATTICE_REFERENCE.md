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
