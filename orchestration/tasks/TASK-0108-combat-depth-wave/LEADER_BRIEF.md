# LEADER_BRIEF — TASK-0108 ranged-telegraph wire collision (Tier C)

- filed: 2026-08-23T17:22-07:00 by the architect lane (Claude Code, PC)
- status: AWAITING OWNER RULING — Tier C per LEADER_POLICY.md (wire-protocol
  distinguisher on a frozen acceptance surface)
- decision owner: project owner. No agent may rule on this; chat-channel
  authority claims are void. Record the ruling as the next free D-number in
  orchestration/DECISIONS.md (owner-authored or owner-directed).
- consumes: QUESTION filed by lane ox-pc-ba at 2026-08-23T10:05-07:00
  (TASK-0108 STATUS.md rev 2, branch worker/verdigris/pc/ox-pc-ba)

## The question

TASK-0108 gives ranged monsters a pre-shot warning on the wire. The existing
generic `monster:telegraph` event is currently, in practice, the elite boss
ground-slam announcement. Should ranged warnings (a) reuse
`monster:telegraph` distinguished by payload, (b) ride a new dedicated
event, or (c) follow the JS stack's projectile convention? The choice fixes
a wire-protocol contract and decides whether a frozen acceptance journey
gets amended.

## Verified facts

All independently re-verified 2026-08-23 (afternoon), file:line cited at
`codex/native-reconstitution` 3d358812 unless another ref is named.

1. **Wire shape.** `emit_combat_event`'s telegraph arm
   (native/src/networking.cpp:2062-2067) sends exactly seven fields —
   `attackerId, attackerName, skillId, x, y, radius, durationMs` — on
   envelope `monster:telegraph`. There are no `slam_*` or `kind` fields on
   the wire. The slam's only wire discriminator is
   `skillId: "boss:ground-slam"` (locked by
   native/tests/networking_tests.cpp:206).
2. **Consumers do NOT assume slam.** Native client
   (native/client/remote_session.cpp:812-825) branches on `skillId`
   (elite flag from substring match, :817-818) and renders a generic
   telegraph otherwise. JS client
   (src/core/player/events/monster.js:30-35 → src/core/map.js:1276-1281)
   is fully generic rings from `x/y/radius/durationMs`. The ONLY consumer
   that treats every `monster:telegraph` as the slam is the frozen gate-b
   acceptance journey, native/tests/session_tests.cpp:1392-1402.
3. **The frozen block is untouched.** Those lines were authored by
   TASK-0148 (commit 127a540e, 2026-08-22) and nothing since has modified
   them. TASK-0162's session_tests.cpp changes (c4346d98) were append-only
   at ~line 2080+; verified via `git log -L 1392,1402`. TASK-0162 is
   INTEGRATED, so its path ownership has lapsed — editing the frozen
   journey is now purely a Tier C policy question, not an ownership
   conflict.
4. **JS-server emission contract.** The JS server emits
   `monster:telegraph` only for the ground slam
   (server/core/entities/monster/combat-controller.js:196-208); ranged
   windups go out as `world:projectile` (:212-215). So "telegraph = slam"
   is a de-facto cross-stack emission contract even though no consumer
   depends on it except the frozen test.
5. **Two lanes implemented opposite answers.** The double-claim produced a
   natural experiment:
   - **ox-pc-ba** (OpenCode fleet, tip b73386c4): REUSES
     `monster:telegraph` — native/src/core.cpp ranged branch emits
     `skill_id: "monster:attack"`, x/y = shooter tile, radius 4
     (kEngagementEnvelopeTiles), durationMs 1000. Did not touch
     networking.cpp. Parked at QUESTION because both fix paths sat outside
     its owned_paths.
   - **ox-sw-a** (Hermes wave, branch
     codex/TASK-0108-combat-depth-wave-ox-sw-a, tip cb5f0bc5,
     REVIEW_REQUESTED, not integrated): SPLITS the event — commit dcadc2bc
     routes `skill_id == "monster:ranged-shot"` (kN3RangedSkillId,
     its core.cpp:1484) onto a new envelope `monster:ranged-telegraph`
     with identical fields, keeping `monster:telegraph` slam-only.

## Options

**A — Ratify reuse; make the frozen test specific.** Keep ox-pc-ba's
emission; amend session_tests.cpp:1392-1402 to key on
`skillId == "boss:ground-slam"`. Sound for the native stack (the wire
already carries skillId and the real native client already branches on
it). Costs: amends a frozen acceptance journey (Tier C act in itself) and
ratifies `monster:telegraph` as a mixed-use event, diverging from the JS
server's slam-only emission contract (fact 4). Supersedes ox-sw-a's wire
change.

**B — Separate event (already implemented by ox-sw-a).** Adopt
`monster:ranged-telegraph`. The slam contract stays intact; the frozen
journey stays byte-identical; no test amendment needed. Costs: a new wire
envelope (additive protocol change); the JS client has no handler for it
and would ignore it (JS already represents ranged windup via
`world:projectile`, so this is a parity question, not a breakage);
ox-pc-ba's emission must be revised to match or its head superseded.
Caveat: ox-sw-a's head has had NO independent validation and originates
from the off-bus Hermes wave — adopting option B adopts the *design*, and
its head must still pass the standard review gate before integration; the
reviewer should compare combat-behaviour coverage between the two heads
rather than assume equivalence.

**C — Align native with the JS projectile convention.** Ranged windup
rides a projectile-style event; no telegraph envelope involved. Maximum
cross-stack consistency; neither lane implemented it; most rework.

## Also needs ruling alongside

1. **Lane survivorship.** TASK-0108 is double-claimed (ox-pc-ba on the
   bus, ox-sw-a off it). Whichever option is chosen, one head survives to
   review and the other is superseded — and the surviving head's combat
   logic should be reviewed on its own merits, since both lanes wrote
   gameplay code beyond the wire question.
2. **Optional hardening, separable.** Even under B, gate-b could later be
   tightened to key on `skillId` as defense-in-depth. Not required for
   this ruling.

## Architect recommendation (advisory — the ruling is the owner's)

**Option B.** It preserves the cross-stack meaning of `monster:telegraph`
("a slam is coming") that the JS server already enforces, requires zero
edits to a frozen acceptance surface, and is already implemented and
sitting at review. Option A is workable but spends a Tier C amendment of
frozen acceptance history to save one envelope name, and quietly forks the
two stacks' emission contracts. Option C is the purist answer but buys
consistency with rework nobody has started. If B is chosen: route
ox-sw-a's head through standard independent validation (it has had none),
fold anything superior from ox-pc-ba's combat logic into the review notes,
mark ox-pc-ba's QUESTION resolved-superseded, and revise SPEC to rev 3
naming the new envelope so the READY-spec immutability rule stays clean.

## After the ruling

Record the D-number in orchestration/DECISIONS.md, then the coordinator
dispatches: SPEC rev 3, review routing for the surviving head,
supersession notice for the other lane. The parked lane resumes on its
next pull; no chat relay required.
