# Guided House/Scion creation — 2026-09-04

## Change

The island's Begin action now leads directly through a compact House-name
card, then a Scion-name card, then world admission. Existing Houses skip the
first card. The large lineage ledger is a secondary menu, not an obligatory
step between every action. The duplicate Create action is hidden when Begin
already leads to creation.

The Scion card contains one `Hardcore (mortal oath)` checkbox and a concise
permanent-death explanation. The selection travels in the typed CreateScion
command and is stored by the server when the Scion is created, not deferred
until admission. Continue still preserves the existing Scion's oath.

Each save waits for authoritative confirmation. Scion admission uses the exact
created-ID receipt, matching House, name and oath; roster order is not identity.
Double-confirm cannot create duplicate Houses/Scions. Back cancels the guided
UI, not already submitted durable work; a late response cannot reopen the UI
or auto-admit a canceled character. Names survive connection loss in the card;
pending saves are never automatically retried. After ten seconds a pending
card explicitly warns against resubmitting and offers Back to the saved roster.

Keyboard flow: type a name, Tab between controls, Space toggles a focused
checkbox, Enter activates the focused action, Escape returns to title.
Mouse targets use the same control functions. Resize clears stale hit boxes.
The transition to the next name field no longer swallows its first character.

## Evidence

- Full `native/build.ps1 -RunTests -RunClientScenarios -BuildSubdirectory
  creation-flow -CaptureRoot native/build/creation-flow/captures`: passed all
  native suites and 35 client scenarios. Log: `native/build/creation-flow-acceptance.log`.
- After the final active-House fallback adjustment, the client was rebuilt
  and all 35 client scenarios passed again (`final-client-scenarios.log`).
  Required `npm run playtest` passed 32/32 (`creation-flow/playtest.log`).
- `guided-creation` drives a real in-process WebSocket server via the production
  RemoteProtocolSession and control functions. It verifies House -> Scion ->
  admission, blank-name rejection, repeated confirm, Hardcore on/off, and late
  save receipts after cancellation. No synthetic roster substitutes for that flow.
- Real-process abrupt-restart/storage gate passed, additionally verifying both
  Hardcore and soft characters survive a server restart **before first admission**.
  Fixture: `C:\Users\Alex\AppData\Local\Temp\verdigris-restart-J20bJ3`.
- Captured production paint at 960×600, 1727×1395 and 3440×1440. Compact and
  owner-half-screen captures were viewed. Framekit field text, checkbox,
  explanation and buttons remain within the compact card.
- Owner desktop controls remain untouched following the earlier Escape.
  These creation captures are scenario renders, **not a live-window interaction
  pass**. Manual keyboard/mouse/paste and final visual acceptance remain pending.

## Remaining

- Full Unicode/IME, selection editing, controller/accessibility semantics.
- Explicit save-failure presentation can be more immediate than the generic
  pending-save text. This UI does not retry uncertain writes.
- The main island's detailed weather/cloud/waterfall and quality ports remain.
- The rest of the voice backlog (skill selection/unlocks, portals, inventory,
  fullscreen tree layering, orbs and notifications) remains in the owner notes.

This build lives under `native/build/creation-flow`; no running game binary
was replaced, no production account was edited, and nothing was pushed.
