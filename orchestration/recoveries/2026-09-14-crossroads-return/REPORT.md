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

Clean packaging, final packaged scenarios and installation are pending this
checkpoint. Commit/push verified implementation under standing authorization;
do not terminate the owner game or overwrite its active executable/save.
