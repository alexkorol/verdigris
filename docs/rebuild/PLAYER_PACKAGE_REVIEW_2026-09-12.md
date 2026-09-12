# Native player package review — 2026-09-12

Lane C owns package assembly and independent launch QA. Exact runtime model
identifier/reasoning effort were not exposed to this worker; no precise model
claim is made. Work preserves the existing dirty journal and does not reset,
fast-forward, push, publish, or import owner saves.

## Artifact and reproduction

- Double-click entry: `C:\Users\Alex\Documents\Verdigris Review 2026-09-12\Verdigris.exe`.
- Native client: same folder, `native\build\verdigris_client.exe`.
- Native server: same folder, `native\build\verdigris_server.exe`.
- File inventory/provenance: same folder, `package-manifest.json` (206 hashed files).
- Build: Lane A's coordinated `native/build.ps1` build, stable client binary
  timestamp `2026-09-12T22:05:23Z`; package command below consumes that build.
- Source snapshot when assembled: `0dc59b0c9fbf9eb92df15fd0eb33438ca2093b87`
  plus the manifest's explicit dirty-file list (includes Lane A frontend work).
  This is a local review snapshot, not a clean release-commit claim.

```powershell
powershell -NoProfile -File native/tools/package-native.ps1 -SkipBuild -OutputDirectory 'C:\Users\Alex\Documents\Verdigris Review 2026-09-12'
powershell -NoProfile -File native/tools/test-player-package.ps1 -PackageDirectory 'C:\Users\Alex\Documents\Verdigris Review 2026-09-12'
```

All 206 package file hashes pass. Launcher SHA-256:
`7172c4e99713ebf3be1f9f14f5912b4012ba04a936740f214d4dc1e4d83b0fac`.
Client SHA-256: `7edcca799a748da091d7e03ca125b8aa2684de2111c694c6a007da2798660ef1`.
Server SHA-256: `5ef18dca5128d78b1bae21f468d358c5707395e2bc75bd1ee8024c6f2707322f`.

## Package behavior

The GUI launcher selects a free loopback port in 6520–6539, waits for the
server's exact ready line, then starts the client with the normal title/
Chronicles path. Both child consoles are suppressed. All working directories
resolve to the package root. Native art, the founding-slice PNG plates, shared
orbs/inventory UI, and bundled fonts travel with the binaries.

Default review saves are under package `profile/saves`; ordinary user settings
remain at the normal per-user location. Explicit `--profile <directory>`
isolates both saves and settings for QA. A filesystem lock prevents concurrent
writers to the same profile, including path aliases. The launcher requests a
graceful server stop on client exit, with bounded kill fallback and a Windows
job as cleanup protection if the launcher terminates.

## Verification and limits

- `npm run playtest`: **32/32 PASS**, exit 0. Log:
  `native/build/lane-c-playtest-normal.log`. Load mode false; p99/max event-loop
  lag 38.50/130.68 ms.
- Full load-mode run with two tracked CPU spinners: **31/32**, exit 1. Log:
  `native/build/lane-c-playtest-load.log`. Combat returned `died` in 4.3 seconds
  before the kill assertion; this was not a 30-second timeout. All other
  scenarios passed. P99/max lag 37.91/156.89 ms. Both spinners were stopped.
- Focused combat rerun under the same two-spinner load: **1/1 PASS**,
  surviving at 134/142 HP. Log:
  `native/build/lane-c-playtest-load-combat-rerun.log`. The full-load run remains
  a recorded failure; this does not certify full-machine saturation.
- Package validation: **206/206 hashes**, simultaneous trailing-separator
  profile alias rejection, and reopen after prior lock disposal all pass.
  The regression test invokes the actual packaged launcher lock method.
- Packaging into an existing destination fails before overwriting files.
- Baseline outside-checkout launch used
  `C:\Users\Alex\AppData\Local\Temp\Verdigris-review-lane-c-baseline-20260912`
  with isolated profile
  `C:\Users\Alex\AppData\Local\Temp\Verdigris-lane-c-profile-20260912`.
  Real keyboard inputs founded a House, created a Scion, and entered a READY
  server-backed town. Viewed 3440×1440 captures:
  `native/build/lane-c-baseline-entry.png`, `lane-c-baseline-house.png`,
  `lane-c-baseline-scion.png`, `lane-c-baseline-world.png`, and
  `lane-c-baseline-load.png`. F3 reported PNG billboards/scenery loaded,
  framekit loaded, and item art 4. Game/client/server PIDs were checked absent
  after normal close; the log records client=0 and graceful server=0.
- Computer Use's screenshot API failed twice with
  `SetIsBorderRequired: No such interface supported`. The repository's
  supported `capture-window.ps1` captured successfully; captures were viewed.
  Live input used the Computer Use window API.

## Final outside-checkout validation

The final Documents package was launched twice from an unrelated Temp working
directory, explicitly reusing only the disposable review profile above. Its
title screen appeared before Chronicles and offered House & Scion, Settings,
and Quit. All native screenshots below were captured at 3440×1440 and viewed.

- `native/build/lane-c-final-title.png`: final title and packaged backdrop.
- `native/build/lane-c-final-settings.png`: settings accessible before play.
- `native/build/lane-c-final-settings-saved.png`: changed Sound to Muted and
  Effects to 90% through real keyboard navigation; save confirmation displayed.
- `native/build/lane-c-final-settings-reloaded.png`: new process, same isolated
  profile, visibly restored Muted/90%/100%. The file contains `version=1`,
  `muted=1`, `sfx=900`, `music=1000` before in-game mute testing.
- `native/build/lane-c-final-chronicles.png`: earlier review House and first
  Scion survived both package change and server restart.
- `native/build/lane-c-final-world.png`: the preserved Scion entered READY
  server-backed town. Pressing M changed only muted to 0; sfx=900/music=1000
  remained intact.
- `native/build/lane-c-final-load.png`: final F3 status shows PNG billboards
  and scenery, framekit loaded, item art 4; live paint was 17.6 ms at capture.
  The inherited dense diagnostic overlay still overlaps text; this report
  does not certify the parked renderer/HUD overhaul.

The unchanged gameplay harness attached to the final package's real server:

```powershell
$env:PLAYTEST_WS_URL = 'ws://127.0.0.1:6520'
npm run playtest -- --attach quickstart movement loot party persistence
```

**5/5 PASS**, exit 0, including loot combat, movement, co-op party lifecycle,
equipment identity/stat persistence, and quickstart. Log:
`native/build/lane-c-native-package-attach.log`; p99/max lag 37.65/44.89 ms.
The environment override was removed after the command.

Final first-session normal close left launcher/client/server PIDs
26932/29852/10332 absent, with client exit 0 and graceful server exit 0 logged.
On the second session, deliberately terminating only launcher PID 36488 proved
Windows-job cleanup: launcher/client/server PIDs 36488/31816/26292 were all
absent 800 ms later. No other processes were terminated. Logs are retained in
the isolated profile `logs` folder. The final package itself has no imported
review account: ordinary double-click starts its fresh `profile` directory.

Settings lane independently reviewed the package implementation and found a
path-hash mutex alias race. Replacing it with an exclusive file handle on the
actual profile's `session.lock` fixed the finding; the reviewer confirmed the
correction and no additional blocker. No recursive review delegation occurred.

The package is unsigned, local-only, Windows x64 with .NET Framework 4.x.
Clean-machine testing, signing, installer/update/rollback certification,
macOS packaging, and public distribution are not claimed. The renderer,
campaign, economy, afflictions, slow spell, and broader Intelligence remain
outside this lane.
