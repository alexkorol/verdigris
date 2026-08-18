# Claude demo asset intake

Status: inventoried 2026-08-15. The source plates remain outside the
repository; this document records the presentation mapping without making the
deterministic native core depend on files, DOM, or image libraries.

## Sources

- Demo source: `C:\Users\Alex\.codex\attachments\37bcbd77-ac09-42f3-a085-c2520575ff6c\pasted-text.txt`
- PNG directory: `C:\Users\Alex\Downloads\claudedemo`
- Count: 22 PNG files, approximately 50 MB total.
- The demo describes these as hot-magenta plates. Magenta removal and edge
  de-spill belong to the renderer/content adapter, not simulation.

## Presentation roles

| Native presentation slot | External plate |
| --- | --- |
| `tree` | `ChatGPT Image Aug 15, 2026, 10_23_21 PM.png` |
| `ruin` | `ChatGPT Image Aug 15, 2026, 10_23_30 PM.png` |
| `dwelling` | `ChatGPT Image Aug 15, 2026, 10_23_40 PM.png` |
| `boss` / green-mask warden | `ChatGPT Image Aug 15, 2026, 10_23_48 PM.png` |
| `raider` | `ChatGPT Image Aug 15, 2026, 10_23_56 PM.png` |
| `scion_str` / Arm of the Hearth | `ChatGPT Image Aug 15, 2026, 10_24_23 PM.png` |
| `scion_dex` / Far-Walker | `ChatGPT Image Aug 15, 2026, 10_24_15 PM.png` |
| `scion_int` / Bronze-Hand | `ChatGPT Image Aug 15, 2026, 10_24_05 PM.png` |

Additional plates are useful shared presentation inputs:

- `10_22_34 PM (1)` through `(6)`: six square ground/terrain material tiles.
- `10_22_35 PM (7)`: terrain/rock/grass/roots atlas.
- `10_23_03 PM (1)`: wagon; `(2)`: shrine/altar.
- `10_23_03 PM (3)` and `(6)`: alternate ornate UI frame plates.
- `10_23_03 PM (4)`: equipment and clothing atlas.
- `10_23_03 PM (5)`: symbolic crest/icon atlas.
- `10_23_03 PM (7)`: four-quadrant dirt/grass/stone/mud reference.

The filename prefix and timestamp are intentionally retained here because the
source directory currently has no stable asset IDs. A future checked-in
manifest should replace these names with content hashes and a versioned slot
schema.

## Integration boundary

1. A renderer-side asset adapter resolves the slot IDs above from a user- or
   build-provided manifest.
2. The adapter applies magenta chroma-keying, de-spill, premultiplied-alpha
   normalization, and billboard anchor metadata at load time. It must never
   overwrite the source PNGs.
3. Native snapshots/events expose semantic roles (tree, dwelling, Scion, and
   so on); they do not expose absolute paths or image handles.
4. WIZARD seams remain composable: Orbs/Splash consume the presentation layer,
   Brands & Bonds supplies item identity/history to UI, and Cartographer can
   supply deterministic map roles that select these billboards.

The first safe renderer experiment is therefore a manifest-driven billboard
layer over the existing placeholder projection. Vendoring or converting the
binary plates is deferred until asset provenance, size, and packaging are
approved.

## Intake recheck — 2026-08-18

The supplied directory still contains all 22 PNG plates (53,121,943 bytes,
50.66 MiB). The source files remain outside the repository and unmodified;
the manifest/provenance decision is still open.
