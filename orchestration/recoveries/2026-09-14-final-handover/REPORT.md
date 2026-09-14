# Combined inventory and unattended handover

Foundation: b21054d63 inventory extensions, merged with typography branch
d4498d2af (owner-selected m5x7 implementation cefd238b2). Preserved accepted
paperdoll composition, authoritative equipment/stats, purse, six skill-gated
drawers, exact-cell transfers, outside drops, perspective and animation.

Sustained drag benchmark exercises the real window handler with 1,440 pointer
events and 40 measured 3440x1440 production frames. It verifies latest-pointer
ghost anchoring, item conservation and absence of attacks. Initial failure:
75.327 ms average, including 51.196 ms HUD cost (earlier run 83.538 ms).
Bounded per-asset well cache preserves gradient/texture pixels and avoids
recompositing unchanged surfaces. Actual authored-weapon drag now averages
36.290 ms (45.187 peak); the unchanged gate is average below 40 ms.

All eight native suites passed. The enhanced inventory scenario passed after
the cache and real-art benchmark changes. Captures inspected: before/after
equipment composition with authoritative ratings and sustained drag at 3440.
The dragged auxiliary test objects remain explicitly labeled synthetic QA
fixtures; no production auxiliary loot, art or combat abilities were invented.

New normal launcher smoke uses an app-owned hidden title window, actual normal
profile selection, confirmed menu Quit, and owned server shutdown. It records
actual child image paths with QueryFullProcessImageName (avoids a MainModule
enumeration startup race reproduced during development), and working directory.
Test script snapshots existing saves/settings, accepts only additive saved
fields or the authorized currency-coordinate migration, and verifies clean
exit of both owned children. Development physical-copy smoke passed, including
a copy of the owner save; actual owner installation was untouched at this point.
The development junction fixture correctly failed the stronger actual-image
path assertion; the physical fixture passed without weakening the check.
Same-executable, same-QA-profile Effects 70% save/reload also passed.

Computer Use remains stopped. No desktop input was resumed. These checks are
application-owned regression fixtures; they do not assert owner live acceptance
or retroactively complete the interrupted desktop walkthrough. Final package,
installed normal smoke and shared-baseline handover are pending this checkpoint.
