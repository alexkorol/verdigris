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
