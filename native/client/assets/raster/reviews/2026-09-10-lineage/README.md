# Selectable male/female Scions — production review

Both owner references are integrated into normal native play with eight
directions, two walking contacts and three attack entries per direction.
The third attack frame reuses idle as recovery. Each appearance persists
per Scion. This is an installed, playable first pass awaiting owner art review.

## View and play

Run `powershell -NoProfile -File native/tools/play-native.ps1` from this
checkout. F11 toggles window mode. Choose Male or Female, create a Scion,
then select its Set out row. Existing Secondborn(male) and Thirdborn(female)
were left available in the local review save. WASD moves; LMB attacks;
Space dashes. The game was left open at the1280×800 picker.

The live review page is `http://127.0.0.1:8873/global-playback.html` while
its local server runs. Portable HTML/GIFs and per-direction native/3× strips
are under `native/tools/raster/hero-lineage-20260910/previews/`.

## Evidence actually inspected

- `picker.png`: final live selector after restarting the server; both saved appearances remain.
- `male-town.png`, `female-town.png`: both in the normal remote game.
- `male-equipped.png`, `female-equipped.png`: all48 production sword-equipped poses per appearance, inspected with the actual GDI draw path.
- `male-new-aim.png`:30fps window recording sampled at20fps; a westward click plays west contact immediately from prior east-facing idle. The empty-handed straight arm remains visually stiff.
- `female-attack.png`: earlier live female anticipation/contact/recovery recording, before the immediate-aim correction.

Root watched the review loops at intended scale and inspected the native
captures. Manual play covered creation, saved-character admission, movement
and attacks in town; it did not cover a completed fight or extraction.

## Verification

`verification.txt` retains the final build/scenario outcomes, selected
performance lines and browser result. Full local logs remain in
`.ci-artifacts/appearance-selection/`:

- `shipping-client.log`: final supported build plus76/76 native scenarios,0 failures; includes the fresh-aim regression and all96 authored selectors/attachments.
- `final-native.log`: core/network/session/presentation/audio/camera suites passed. Its first client run had18 missing sockets because the final include had not arrived; rebuilding with the complete include fixed those failures.
- `browser-playtest.log`: `npm run playtest`,32/32 scenarios, Node22.11.
- `wight-recheck.log` / `wight-warmed.log`: initially the extra asset load consumed a short timed pursuit. Warming before teleport fixed the fixture without weakening its sample/motion checks; focused and final full runs pass.

GTX1660SUPER,3440×1440: Fable20 frames average28.555ms; streamed34 frames
average24.530ms, peak44.126ms. The unchanged limit is40ms average. Legacy
movement averages25.8ms, peak36.1ms. Terrain CPU peak37,343,568bytes.

Source/runtime validation passes96 PNGs,96 sockets,80 selected originals,
16 shared palettes and16 exact idle recoveries. The catalog passes272 PNGs.
Combined provenance SHA256:
`90710314fe9f8f9fe8266c65ef0ccb737901f30d7de821ac7daa51472953f286`.
Equipment include SHA256:
`78538f5d94f176d06d87593e80eecd21a615ef13554377e66e2aa4f4f938c9c2`.

## Art limits retained for review

Two contacts produce a stiff walk without a passing phase. E/W leg ownership
is less readable; diagonal poses retain some torso/foot and baseline drift.
Female NW contact B has slightly shorter hair and a stronger build after a
pose-guided identity edit. Idle recovery is abrupt, and weapon-specific bow/
staff choreography is not authored. Existing scenery/NPCs still differ in
style from the new natural-proportioned adults. No claim of perfect character
or temporal consistency, full UI parity, or final owner approval is made.
