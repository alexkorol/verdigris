---
task: TASK-0048
state: REVIEW_REQUESTED
commits:
  - pending
source_fix: none (reported combat regression disproven)
architect_review_required: true
---

## Verdict

The accepted TASK-0046 Chronicles combat claim is not reproduced as a
mechanical Chronicles failure. The browser wire transcript shows that the
driver sent attacks while it remained out of contact with the opening actor.
The discriminating real-protocol scenario and a rendered browser capture both
prove that a mortal-oath Scion can damage and kill the Old Barrow opener. No
server or client combat patch is justified by the evidence.

## Baseline negative and exact failing path

The baseline capture was run from the accepted arc's source tip `c3988b29`.
`captures/baseline-c3988-wire.json` records the real browser WebSocket traffic:

```text
out:player:move              42
out:player:skill:trigger     14
in:game:send:message          2
in:player:movement           28
in:combat:hit                0
```

The follow-up `monster:state` inspection of that same path located the first
active Dread Vanguard around `(91,107)` while the player remained around
`(101–102,100)`; later actors were dormant. The 14 primary-attack frames were
therefore valid input with no target in the melee arc. The body remained at
full HP and contained no hit/slain line, exactly as the accepted report said,
but this is a non-contact movement/aiming result rather than a Chronicles
state rejection. The path was:

```text
Play as Guest → Chronicles → House Ember → mortal Asha → Set Out
→ Adventure → Old Barrow → WASD cycle/primary frames → no target contact
```

The direct Chronicles admission path also completed successfully during the
investigation. The starter dagger remains in inventory and not worn by the
fresh profile, matching `tests/unit/fresh-scion-profile.spec.js` and the
inventory-equip contract; it is not a combat blocker because unarmed primary
damage is authoritative.

## Discriminating regression scenario

`playtest/scenarios/chronicles-first-combat.mjs` uses the real WebSocket
Chronicles pending-session flow, creates a mortal Scion, enters Old Barrow,
reads the authoritative opening actor, and uses `dev:teleport` only to set up
the controlled contact distance. The fight itself uses ordinary
`player:skill:trigger` plus server combat resolution.

Transcript on the current tip:

```text
✓ mortal oath is carried into world admission
✓ Old Barrow exposes a live opening encounter actor
✓ mortal-oath first attack deals authoritative damage
✓ mortal-oath first kill resolves within 1402ms
✓ first kill emits a readable server message
PASS chronicles-first-combat
```

The scenario also passed independently at 1119ms. An authentic negative on an
unfixed combat base cannot be supplied because the proposed negative was
invalid: it never reached the actor. This is recorded as a review deviation,
not hidden by weakening assertions.

## Rendered proof

`captures/rendered-first-kill.json` is a hard-fail Playwright browser capture.
It reaches the mortal-oath UI flow, requests the real authoritative target,
uses dev teleport for setup, sends two skill frames, and records four
`combat:hit` frames: player damage 5/5/6, incoming damage 5, and a final
`died:true` monster hit. The rendered body includes:

```text
You hit Dread Vanguard with Bronze Arc for 6.
Dread Vanguard died. +13 Attack XP.
You have slain Dread Vanguard.
HP 105 / 110
```

## Mana disposition

No mana numbers were retuned. The current starter values are 90 maximum mana,
2 mana regenerated every two seconds, and skill costs 10–22. Repeated
`Not enough mana.` is an intentional resource gate, but the rejection remains
non-directive. The owner question is whether to change that copy to state the
missing amount and passive recovery cadence; this task does not change the
balance or silently choose that product wording.

## Verification

```text
npm run test:unit                         123 files / 788 tests passed
npm run lint                              passed
npm run build                             passed (379 modules)
npm run playtest -- chronicles-first-combat 1/1 passed
npm run playtest                          32/32 scenarios passed
```

No `server/**` or `src/**` source file changed. The owned implementation is
the targeted Chronicles first-combat regression scenario plus reproducible
wire/rendered evidence; architect review is requested because it overturns
the accepted TASK-0046 diagnosis rather than fixing a confirmed game bug.
