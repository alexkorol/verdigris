# Native player typography - owner-selected m5x7

The owner selected the m5x7 captures supplied as download.png and download (1).png.
The runtime now uses that font at the same 32px em (14px capitals, 2px authored
steps). Pixel Operator 8 was the middle reference; the interim Pixel Operator HB
revision is superseded. Nox's exact font remains unidentified. The selection
establishes the owner's typography preference, not acceptance of every layout.

Verdigris Sans privately bundles Daniel Linssen's m5x7 under CC0. Original source,
license text and attribution are in native/client/assets/fonts/sans/. The build
renames the family, adds compatible punctuation aliases and aligns line metrics
to the source pixel grid without changing glyph outlines or advances. The font
maps 326 characters; unsupported glyphs explicitly use the same family's '?'.

Ordinary roles use 32px em, titles 64px, and inventory cell counts and small orb values use the native
16px em so ordinary quantities remain legible inside tiny cells. Cached GDI
fonts rasterize without antialiasing or interpolation. Name editing, selection,
scrolling, executable-relative loading and explicit missing-resource failure
remain. Names retain the established 40 printable-ASCII limit.

Measured geometry now gives top buttons and hotbar labels enough room. House
intro and portrait descriptions reserve their full wrapped height. Tooltip
measurement and drawing share fixed-width lines, including glyph-boundary
breaks for words wider than the available world gap. This avoids DrawText's
CALCRECT width expansion and the resulting lost final words. No new dialogue,
chat, gameplay stats or inventory schema was introduced.

The change is isolated from consolidated c00ade08e on
codex/native-typography-sans-20260914, preserving concurrent unrelated inventory
edits in verdigris-consolidated-20260913. Existing verified equipment/stat fixes,
WIZARD composition, perspective rendering and authored animation remain included.

Evidence naming: files without a font prefix are historical serif evidence;
`sans-` files are interim Pixel Operator comparisons; `selected-` files are the
owner-selected m5x7 production renders and logs. Synthetic long-name/count/log
fixtures are labeled stress evidence, not actual player data or a dialogue system.

Validation and delivery results are appended after the final checks.
Desktop automation was stopped by the user's Escape key earlier in the session.
No further automated desktop input is authorized by that stopped run. A fresh
live check of the final normal installation remains separate from scenario
captures and package verification.

Pre-package validation: actual production renders inspected at 960x600,
1280x800 and 3440x1440. Typography (including fixed-width long-word retention,
measured menu fitting and three-digit counts), inventory/equipment, vital-orbs,
hitch-warmup and frame-budget pass. Missing font exits 2 with its actual path;
font and coverage regeneration are byte-identical. Frame budget averaged
31.398ms static and 34.245ms moving (48.242ms peak), below the unchanged 40ms
average gate. No browser tests were used for native acceptance.

The first m5x7 all-scenarios run passed 78/80: hitch-warmup exceeded its relative
single-frame comparison by 0.567ms and passed the isolated unchanged retry;
vital-orbs failed because large numeric captions obscured the colored glass.
The compact numeric role fixes that production defect; the unchanged color
check passes. The full failed log is retained, not silently presented as green.
The final clean-package suite is pending at this source commit.

Earlier native compilation and eight non-UI suites passed across a full-build
run and solo retries. The first session-suite run had reconnect/death failures
while another test session was active; its complete solo retry passed. Both
failed and successful logs are retained as interim evidence.

Clean package: source cefd238b234c0c489abb2718e6b86ba001672c6c,
native/build/player-package-cefd238b2/Verdigris.exe. Fresh compilation and native
denylist passed. Package verification passed all 1,270 file hashes, 349 required
resources, embedded clean source identity and profile-lock alias/reopen checks.
The source commit is verified on origin/codex/native-typography-sans-20260914.

Concurrent inventory milestone 1d3db8ab7 in the other consolidated checkout is
separate work and is not included in this typography package. Its files and
branch were not modified. Future integration must retain both that work and
the selected m5x7 family/scale; do not restore Novel from that older font base.
The normal-installation promotion checks the previously inspected c00ade08e
identity before replacing anything, so a newer installation is not overwritten.

Final packaged validation: all 80/80 client scenarios pass with exit 0 on source
cefd238b2, including typography, inventory/equipment, vital-orbs and hitch-warmup.
The unchanged fullscreen frame gate averaged 29.284ms static and 35.525ms moving
(52.600ms peak). The complete successful log is selected-package-all.log.
The production title, entry, settings, inventory, tooltip and log captures were
inspected separately. The all-scenarios run invoked the packaged executable from
the source checkout; a separate typography run from the package working directory
also passed. Private font loading is executable-relative in both cases.

Normal installation updated in place on 2026-09-13 at 21:30 local time:
C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe.
Installed source is cefd238b234c0c489abb2718e6b86ba001672c6c. All 1,270 installed
hashes, 349 resources and embedded clean source identity pass. The one existing
save is byte-identical; normal settings remained absent. Obsolete Novel files
were removed only after path containment and backup/hash verification. No QA
profiles or fixtures entered the normal installation.

Rollback: native/build/typography-sans/rollback-20260913213044/installation.
The owner was asked to open the usual launcher for a final read-only live capture.
That final normal-window check remains pending; packaged production captures and
hash verification do not substitute for it. No further desktop input followed
the earlier Escape stop. No whole-product visual approval is claimed.
