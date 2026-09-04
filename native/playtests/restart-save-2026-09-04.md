# Native restart-save and naming fix pass — 2026-09-04

## Owner reports addressed

- Accounts/House/Scion disappear whenever the native server process restarts.
- Continuing an existing character must keep purchased items, currency, gear,
  build choices and House advancement; vendors need to remain testable.
- House/Scion name input overlaps the left edge of its Framekit raster.

## Implementation

- Native owner-play now writes versioned/checksummed atomic account files to
  `%LOCALAPPDATA%\Verdigris\Saves`. Port/build/cwd changes do not move the saves.
  An exclusive directory lock prevents two owner servers writing stale copies.
- Durable fields include exact item UUIDs, quantities/footprints, equipment,
  Vessel Brands/Bonds/awakening/history, complete tablet rolls, bank/House
  stores, Chronicle, progression, passive allocation, actor stats, RNG streams,
  active/reserve Scion profiles, final death and own-account relic circulation.
- Successful session replies follow saving; failed storage stops additional
  mutations. Corrupt/unsupported saves reject admission and remain untouched.
- Selecting a previously played Scion restores its profile instead of
  unconditionally discarding its items. New Scions cannot inherit the prior
  character's level/build. Resuming cannot silently remove its existing oath.
- The platform supplies an item-identity namespace at server boot; new drops
  cannot reuse the old process's counter-only item IDs. Chronicle creation
  skips IDs already present in living/history records.
- Name text uses the authored image's inner inset, a vertically centered
  clipped rectangle, and a scrolling visible suffix. The actual name remains
  intact. Both House and Scion modes have long-name regression captures.
- Native build gates now propagate core/network/camera failures instead of
  allowing a later passing command to hide them. Separate build directories
  allow testing without overwriting an owner's running executable.

## Evidence

- `native/build.ps1 -RunTests -RunClientScenarios -BuildSubdirectory restart-fix
  -CaptureRoot native/build/restart-fix/captures`: native suites and all 33
  client scenarios passed; final frame-budget result 11.8 ms at 3440x1440 (40 ms gate).
- `node native/tools/test-account-restart.mjs
  native/build/restart-fix/verdigris_server.exe`: real WebSocket/server process
  restart gate passed. The test buys and banks a weapon, equips a seeded
  Vessel, carries a rolled tablet, allocates passive nodes, chooses an earned
  House investment, switches Scions, force-kills and restarts the server,
  checks exact durable state, completes another zone round trip, and verifies
  mortality, exact relic recovery, writer exclusion, fail-stopped writes and
  corruption rejection. Latest full fixture:
  `C:\Users\Alex\AppData\Local\Temp\verdigris-restart-RUBdfj`.
  Fixtures are isolated in OS temp directories; the script prints their paths.
- `npm run playtest`: 32/32 gameplay scenarios passed.
- Viewed generated long-name captures at 960x600 and 1727x1395 under
  `native/build/restart-fix/captures/`; live owner-size startup/naming/key-input
  captures are in the same directory (`live-key-naming.png` shows entered W
  and caret within the input's opening).
- A separate real server/client ran through `play-native.ps1` on 6538. Only
  that verification window was closed; the launcher verified no orphan child
  processes. The owner's original client/server on 6520 were left running.
- Windows Computer Use image capture failed with `SetIsBorderRequired: No
  such interface supported`; the repository's PrintWindow capture worked.
  Computer Use keyboard actions and accessibility targeting worked. Its
  bulk-text insertion did not enter text in the custom game field; physical
  key entry did. Paste/IME/dictation handling needs a dedicated follow-up.

## Still open / limitations

- Startup layout simplification and the fully interactive WIZARD island title
  scene remain open; this pass does not claim the full voice backlog is done.
  The 960x600 capture also exposes an overflowing House ledger heading, now
  recorded for the startup pass (distinct from the fixed editable field).
- New save support cannot restore data already lost by an older server or
  hot-migrate the owner's currently running memory-only server. Do not claim
  that old window has acquired disk saves merely because a new build exists.
- Active instances/drops are intentionally retired on restart. The restart
  gate covers one account, not cross-account relic trades or live-party
  transaction/crash consistency. Multi-House switching needs separate coverage.
- Save storage is for the local native account identity, not cloud accounts or
  authentication. Corruption is surfaced, not automatically repaired.
