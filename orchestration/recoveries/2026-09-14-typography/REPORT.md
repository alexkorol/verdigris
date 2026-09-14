# Native typography implementation — 2026-09-14

Verdigris Novel is a CC0 derivative of Not Jam Novel 16. Its source, license,
compatible punctuation aliases and reproduction command are documented beside
`native/client/assets/fonts/novel/VerdigrisNovel.ttf`. The supplied reference's
font is unidentified. Native GDI comparisons with Not Jam Serif 11 and existing
Pixelmix favored Novel's narrow, differentiated single-pixel strokes; it is
finer and more condensed than the reference, not an exact match or owner approval.

The shared type roles replace Segoe UI/Georgia and floating-damage Verdana across
menus, House/Scion, equipment/stats/tooltips, the existing message log, HUD and
world labels. Rendering and measurement use the same Unicode conversion and
explicit same-family missing-glyph policy. Private resources load relative to
the executable, cache per integral size, and fail explicitly when absent.
Per-monitor DPI awareness avoids Windows bitmap interpolation. No camera,
animation, equipment, stat or inventory-composition changes were made.

Names retain 40 printable ASCII characters and now support measured caret/click
positioning, selection, replacement, deletion and horizontal scrolling. The
existing message log wraps instead of estimating and truncating a byte count.
No dialogue/chat system was added. Body is the role for those future surfaces.

Validation before the implementation commit:

- `native/tools/verify-native.ps1 -CaptureRoot native/build/typography-evidence/full-gate`:
  build, eight native suites, denylist and 80/80 scenarios passed (full-gate.log).
- The last DPI-awareness change was built separately using the same MSVC client
  flags/object set and passed `--scenario typography` (typecheck.log). The clean
  package will rebuild all final sources together.
- Actual monochrome GDI pixels, exact 2x metrics, selected face, cache reuse,
  UTF-8 quotes, explicit unsupported-glyph replacement, input editing and
  field containment passed. Font regeneration was byte-identical.
- A client copied without its bundled font exited 2 with the resource path;
  no installed font was used as a substitute.
- Production-paint fixture captures inspected at 960x600, 1280x800, 3440x1440:
  equipment/stats, full long item names/quantities, menus/name entry, and log/HUD.
  Reference dialogue/button/heading crops and nearest-neighbor specimen examined.
- `before-title.png` and `before-entry.png` are live PrintWindow captures of the
  installed 43c104c5a package using an isolated profile; its client/server exited
  cleanly. `before-inventory.png` is the actual server equipment fixture before
  typography changes. `before-message-log-fixture.png` is a crop of the existing
  combat-scenario log. The after log sentences and long-name/count/roster images
  are labeled presentation fixtures, not an implemented dialogue screen.

Limitations: 746 mapped characters, selected Latin/Cyrillic/symbol coverage;
unsupported characters display `?`. Smart quote glyphs are straight aliases.
Name entry remains ASCII-only. A steady measured caret is used. The internal
capture-only beat legend retains its developer font. The Computer Use capture
API failed with an interface error; live images use the repository-supported
PrintWindow helper and app input uses the Computer Use API.

Clean package identity, final live checks and normal-launch promotion will be
recorded after packaging. Earlier equipment/stat repair handover d0179c8b3 is
preserved beneath this typography addition.
