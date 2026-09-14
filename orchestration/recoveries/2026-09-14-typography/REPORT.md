# Native player typography â€” sans-serif revision

The owner corrected the direction to **sans serif**. The runtime now uses
Verdigris Sans, a private-family derivative of **Pixel Operator HB** by
Jayvee Enaguas (HarvettFox96), under CC0. The author's distribution page is
https://www.dafont.com/pixel-operator.font; the original font and license are
bundled in `native/client/assets/fonts/sans/`. The serif Novel candidate and
its resources are superseded. No owner visual approval or exact identification
of the reference's font is claimed.

The owner liked the sans-serif direction and requested more visible pixelation.
Pixel Operator HB keeps the proportional family and its 16px grid, with heavier
hand-authored pixel strokes. Lower-grid trials were rejected after actual native
captures exposed overly wide text and clipping. No synthetic bold or bitmap
interpolation is used. The result is a heavier pixel treatment, not a claim to
have reproduced Nox's exact font. Capitals are 9px; Titles use a 32px em.
Viewport UI tiers multiply raster sizes by integers. Camera and animation remain.

Shared Body, Label, Heading, Compact and Title roles cover menus, House/Scion,
equipment/stats/tooltips, the existing message log, HUD and world labels.
The prior measured editing/selection/scrolling and log wrapping remain. Fonts
are cached and registered from executable-relative bytes with no system-font
fallback. Names retain their 40 printable-ASCII limit. The font has 239 mapped
characters: Latin-1, selected extended Latin, punctuation and symbols. It lacks
the earlier serif candidate's Cyrillic coverage; unsupported glyphs explicitly
display `?`. Existing curly quotes are preserved; ellipsis uses three periods.
No new dialogue or chat system was built.

Evidence in `evidence/` from the first implementation is historical serif
comparison evidence. Files prefixed `sans-` describe the final sans revision.
Log, long-name and roster screenshots are production-paint fixtures, not proof
of an implemented dialogue system. Before title/entry captures are live views
of installed source 43c104c5a; before inventory/log images are labeled fixtures.

The sans change was isolated from c00ade08e on
`codex/native-typography-sans-20260914` after concurrent inventory source edits
appeared in the consolidated working directory. Those unrelated edits remain
there and are excluded from the typography package. The verified equipment/stat
repairs already in c00ade08e remain included.

Validation, package identity and normal-installation handover are recorded below
after the isolated native gate and clean package complete.
