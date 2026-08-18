---
task: TASK-0055
title: Browser follow-ups — identity chip, server-side zone preview payload
state: READY
priority: medium (BOUNDED-DESIGN; closes 0049 review debts)
owned_paths:
  - server/core/party.js
  - server/player/handlers/**
  - src/components/layout/GameHUD.vue
  - src/core/adventure-objectives.js
  - src/core/adventure-objective-data.js (DELETE it)
  - tests/**
  - orchestration/tasks/TASK-0055-browser-followups/**
forbidden_paths:
  - playtest/** assertions (new/strengthened scenarios welcome)
  - native/**
base: current program tip
architect_review_required: true
---

## Deliverables (from accepted 0049 review notes)

1. **Identity chip truncation fix**: long House/Scion names currently
   ellipsize into the HP orb. Cap the chip width with full text on
   hover (title attr), or move it above the orb — visually verified at
   1366x768 AND 1920x1080 with a long generated name.
2. **Server-side zone preview payload**: the adventure/zone payload
   gains the display fields the client currently mirrors (boss display
   name, guaranteed treasure item level, depth) — sourced from the
   same server tables the game already uses. Then DELETE
   `src/core/adventure-objective-data.js` and read the payload in
   `adventure-objectives.js`. No client-side mirror remains (the
   drift hazard the 0049 review flagged).
3. Protocol shape: additive fields only on the existing envelope —
   nothing removed or renamed (old clients must not break).

## Acceptance evidence

1. `npm run test:unit` + full `npm run playtest` + `npm run
   smoke:browser` literal transcripts (default path).
2. Hard-fail capture script asserting: chip shows full name on hover
   (or fits untruncated), and one Adventure row's preview text matches
   the SERVER-sent values (grep the payload in the capture log).
3. Diff proof that adventure-objective-data.js is deleted.

Architect reruns gates + inspects 1-2 captures.
