# Native consolidation acceptance record

## Implemented source

- Branch: codex/native-consolidated-20260913
- Merge: 98a0bf736797cb02c7c1543a2890daba47056f41
- Packaged source: 3bd164d34f189d1438dc68990667312e8ecd721f
- Foundation: 5c388d9fd54b65a7092e669a19440294b57f6bdf
- Recovered history: 443fb250314f7be3f3fef3aff86c2ded5cbe3eac
- Entry: C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe
- Client: C:/Users/Alex/Documents/Verdigris Native 2026-09-13/native/build/verdigris_client.exe
- Launcher SHA256: 7b87cd528a417a1232238c70cfe5cbb51e2928f65424411d206141e2c82591f6
- Client SHA256: e88eb857822200c665abd467d4e9d5c27930548870264d7a9153e802f9fc7d83

The complete renderer and recovered histories are retained as merge parents.
The perspective renderer, animation assets and equipment attachments, level
persistence, melee contact and starting movement remain intact. Application
integration reconciles startup/input/session changes, adds editable House and
Scion names, explicit House selection for creation, clickable appearance and
Scion cards, keyboard focus, character management and returning-player Continue.
Both build paths embed the source identity. The normal remote path requires
all 96 authored actor poses; package validation independently requires them.
The package's scenario captures are confined inside its installation root.

The unfinished verdigris-playable checkout was inspected and left untouched.
Its build-identity headers/generation and package assertion were reused. Its
partially applied level/contact/movement fixes duplicate the foundation. Its
inventory-footprint, equipment-seat and combat-stat extensions are separate
unfinished work, including rejected patches, preserved in that checkout and
not claimed integrated here. Existing owner saves and checkouts were not edited.

## Completed checks

- Development build, core/networking/camera/Fable-camera/session/presentation/
  audio/settings tests and legacy denylist passed.
- All 78 development client scenarios passed; unchanged performance bounds.
- Fresh package from the clean source commit above; 1,216 file hashes and 296
  required resource paths passed, plus embedded-source identity and profile
  lock/alias checks.
- All 78 scenarios passed in this exact packaged client, launched with the
  package as cwd, outside the checkout. Captures: package qa/captures.
- Packaged consolidated-flow drives real Win32 handlers against a native socket
  server: named House, named female Scion, clicked admission, hardware
  perspective, pause/return/title/management, Continue and reconnect Continue.
- Packaged Fable fullscreen average: 30.355 ms; the 40 ms gate was unchanged.
  Final moving-frame gate averaged 24.0 ms (33.9 ms peak).
- Inspected packaged perspective/movement captures and authored pose sheets.
- Launched the actual Verdigris.exe with isolated QA profile at
  C:/Users/Alex/Documents/Verdigris Native QA 2026-09-13. Launcher log records
  source=3bd164d34f189d1438dc68990667312e8ecd721f, dirty=0, Fable perspective=1, 96 authored
  poses, and the asset root inside this same package.
- Live title and Settings inspected at 3440x1440 and 1280x800; source ID visible.
  Mouse opened Settings, reduced Effects to 90%, and returned via Back.
  settings.ini contained version=1, muted=0, sfx=900, music=1000.
- Mouse opened the new House/Scion card screen and focused its House-name field.
  A live normal-game perspective frame was subsequently visible after user input.

## Incomplete acceptance / interruptions

Full package acceptance is NOT claimed. The user physically pressed Escape,
which stopped Computer Use during live name-entry verification. No further
app input was sent. The user took control of the running QA game; it was left
running. Remaining: agent-controlled live House/Scion/name/appearance journey,
normal movement/attack walkthrough, and settings persistence verified by
quitting/relaunching this same packaged executable. Automated coverage of these
menu/gameplay paths passed, but is distinguished from the unfinished live check.

The initial outside-checkout scenario attempt failed because capture validation
required repository markers. Commit 3bd164d34 fixed packaged-root containment;
the rebuilt package's full 78-scenario run then passed. The first candidate at
C:/Users/Alex/Documents/Verdigris Consolidated 2026-09-13 is superseded.
The Computer Use screenshot helper also failed with SetIsBorderRequired /
0x80004002; the repository capture-window.ps1 fallback produced viewed live
captures. This was a tooling failure, not a renderer failure.

Normal launch entries were not modified. Publish this task branch for review;
complete live acceptance before promoting it to the shared native program
branch or switching the user's normal launch. This is a task-specific remaining
acceptance gate, not a blanket push ban or an extra approval requirement.

Local logs: native/build/integration-build-2.log, packaged-scenarios-final.log,
package-build-final.log and live-*.png. All are retained in the isolated
consolidation checkout. Native core/gameplay sources and tests remain unchanged
from the selected renderer foundation; browser tests were not substituted for
native verification.
