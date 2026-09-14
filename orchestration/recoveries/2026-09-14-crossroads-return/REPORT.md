# Crossroads safe-return repair

Owner report: equipped/carried items and coins disappear when leaving a combat
zone. Reproduced: ProtocolSession::finish_extraction drained all inventory and
all worn seats into house_store_ on every return path. This store was separate
from the accessible bank_. Older tests explicitly required this stripping.
The new regression failed on the original implementation before the fix.

Safe return now retains the living Scion's inventory, purse, compartment cells,
equipment and authoritative ratings. Existing explicit House deposits remain.
Every protocol return path shares finish_extraction, including walking, dash,
return action and teleport-driven fixture transitions. Local core extraction
fixtures remain separate; no combat/economy rebalance was made.

Old extraction-only records migrate into the existing accessible House bank on
load, retaining exact UUIDs/quantities and leaving normal bank deposits intact.
No equipped seat or owning Scion is guessed. Migration is idempotent after
persist/restart. Bank withdrawals now validate successful inventory placement
before removing the stored record; full bags retain the item, and partial
currency withdrawal preserves the remaining balance. Nonpositive quantities
are rejected. The normal smoke preservation check verifies the exact old-bank
plus old-extraction-store sequence against the new bank, rather than allowing
arbitrary removals or relaxing whole-save comparison.

Read-only inspection found 370 coins in the owner's old extraction store.
The normal game was running; it was neither stopped nor modified. This report
does not claim the running old process already has the fix or recovered bank.

Verification checkpoint: eight native suites passed, including migration,
UUID/quantity preservation, restart idempotency, partial coin withdrawal with a
full backpack, rejected equipment withdrawal, and actual remote dash return.
The enhanced native inventory scenario passed using a real server and full
loadout: return action and walking across stairs retain exact item identities,
coin quantities, worn seats and ratings; subsequent same-profile restart also
retains the equipment/pack. Actual Crossroads capture inspected. Sustained
drag average 34.550 ms (unchanged <40 ms gate).

The existing bank row now withdraws its displayed stack in one action; an
actual production row click restores the full purse in the native scenario.
Bank row/title/footer heights follow the actual font metrics, preventing the
recovery label from being clipped. The footer is shortened to the balances.

Intermediate failures are retained rather than relabeled as passes: the first
expanded run issued the next drag before InventoryAccepted; the fixture now
waits for both authoritative removal from the seat and acknowledgment. The
next run passed all functional assertions but missed sustained dragging's
unchanged 40 ms budget (41.828 ms average, floor 26.139 / HUD 15.689). Earlier
same-feature measurement was 34.685 ms; final packaged verification must still
pass the unchanged gate. No performance bound or assertion was relaxed.

Clean packaging, final packaged scenarios and installation are pending this
checkpoint. Do not terminate the owner game or overwrite its active
executable/save. The final package's source hash and results follow below when
available.

The final focused run (crossroads-recovery-ui3.log) passes all assertions,
including the whole-stack row click and acknowledged drag sequence, with
34.636 ms sustained-drag average (54.627 ms peak; the existing gate is average).
The corrected bank capture was viewed at 1366x768: no clipped withdrawal text
or overflowing footer. This is a focused result, not final package acceptance.

## Exact-package save recovery

Clean package source: `7e11bc52939137573bc4b9845a433e69b4e38789`.
Candidate: `native/build/player-package-7e11bc529/Verdigris.exe`.
The candidate top-level executable was launched twice with its normal profile
containing a copy of the owner's save, never the live owner profile. The first
startup moved the one legacy coin record (370 coins) into the accessible bank;
the complete preservation comparison passed. The second startup preserved that
bank exactly, without duplication. Both checks verified actual child executable
paths, working directory, perspective startup, menu quit and owned-process
cleanup. Normal settings remained absent. Raw copied owner saves stay only in
ignored local QA evidence, not Git.

## Packaged performance failure and correction

Package 7e11bc529 was held back: its full suite failed only sustained dragging
(44.116 ms; 31.123 world / 12.992 HUD). Its item, currency and recovery checks
passed, but it was not installed. The failed package log is retained.

The live window and scenario now share a full-resolution 32-bit DIB composition
surface, reducing HUD composition cost. GPU stage measurements identified
world shading/readback as the remaining cost. The renderer now excludes world
shading only inside the opaque inventory well, whose production skin fills
every pixel before any texture overlay. No resolution, animation, shader blur,
camera behavior or frame-budget threshold was reduced. A real GPU comparison
checks that every pixel outside the excluded rectangle is identical and that
closing the panel restores the complete original image. The live window's
actual backbuffer is also checked, not only a test-created surface.

Final focused run: crossroads-occlusion-ui2.log passes all assertions, including
return/recovery and GPU pixel preservation; 39.605 ms average, 41.571 ms peak.
Production sustained-drag capture inspected. Earlier diagnostic test mistakes
(an invalid zero camera projection parameter and a height-sign assumption in
GetObject) were corrected; assertions still require a valid rendered sprite,
actual hidden-pixel changes, exact visible pixels and full-resolution surface.
Final clean package verification follows this implementation checkpoint.

Package 4c40fdccc passed gameplay, returns, recovery and dragging (34.172 ms),
but nine assertions in the legacy resource-envelope fixture expected a DDB
floor cache while its new DIB target intentionally bypassed that cache. The
fixture now explicitly requests a device bitmap for its cache resize cycles;
all existing one-bitmap, dimensions, pen, brush and effect limits remain. Its
focused run passes. The failed package was not installed.

Further profiling found conversion cost in the full-frame GDI upload. The live
window now passes its known top-down DIB storage to the GPU compositor, which
flushes preceding GDI commands and copies exact output bytes directly. Other
HDC callers retain the normal conversion path. Buffer sizes are checked; real
GPU tests prove byte-identical output and rejection of an undersized buffer.
Focused inventory/recovery/GPU tests pass (37.906 ms dragging); the actual
window backbuffer is captured for the final package inspection.
