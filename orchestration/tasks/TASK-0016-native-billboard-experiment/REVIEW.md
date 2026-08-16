---
task: TASK-0016
verdict: ACCEPTED
reviewed_commits:
  - 6d1b7d6
---

## What was reviewed

Client diff, native gates rerun on the Codex tip (green, headless
unchanged), and an independent driven-input pass with my own PrintWindow
captures.

## What is correct

Verified live by the architect: billboarded scion renders with contact
shadow over the intact scene (grid, HUD, skill strip, event log, loot
labels), debug line reports "billboards: on (scion_str / raider / boss;
magenta keyed)". Keying is clean at this scale; scene composition
unbroken; fallback path present in code.

## Problems

1. (Evidence quality, no code change) All six captures committed with the
   task are blank/white — they were taken before the first paint. My
   independent captures supersede them for acceptance. Future driven
   passes must wait for a painted frame (the 0013 pass did this
   correctly).

## Required corrections

None (accepted on architect-verified evidence).

## Architectural effect

The native lab is now visually real; scenery billboards are the natural
follow-up. Integration approved.
