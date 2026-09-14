# Illustrated native menu and sprite particles — 2026-09-14

Working checkout: C:/Users/Alex/Documents/ChatGPT/verdigris-consolidated-20260913
Branch: codex/native-consolidated-20260913
Foundation: 16fb57bf9d706a4b25ee3fee12ae08e9a2826cc0 (includes the shipped Crossroads retention fix).

The title, pause, settings and quit screens now use original illustrated cedar/bronze gateway art and physical red-enamel controls. Runtime labels, focus and amber flecks remain interactive; control endcaps preserve their proportions. House/Scion creation retains the existing server-backed controller, name fields, appearance choices, roster, oath and Continue. It uses the same artwork and physical controls. No replacement launcher or competing game was created.

The native particle module adds a fixed-step, bounded CPU simulation, a 64x64 point-sampled sprite atlas, external recipes, actor/world anchors, local/world particles, burst/continuous emitters, alpha/additive rendering, lifetime curves and seeded replay. The perspective renderer batches adjacent compatible atlas sprites and uses at most two effect lights. The level-up event is emitted only by an actual server XP threshold crossing; loading a save does not infer it.

Implementation and trigger details: native/client/vfx/README.md.
Menu artwork provenance and prompts: native/client/assets/menu/provenance.json (built-in image_gen, original artwork).

Changed areas: native/client/main.cpp, frontend_art.hpp, chronicles_view.hpp, consolidation_scenarios.hpp, ui_skin.hpp, fable_gpu.hpp, fable_world.hpp, presentation_events.hpp, remote_session.cpp; dedicated vfx module/scenario; assets/menu and assets/effects; native/src/networking.cpp; networking/session regressions; native/tools/package-resources.ps1. No save format changes.

Completed development checks:
- Full native build and all eight native suites passed (native/build/menu-particles-verified-build.log, exit 0).
- The real kill/XP regression emits one level-up; the real socket seam passes the event and rejects foreign/fractional events. Admission and snapshot updates emit none.
- Focused particle tests cover seven recipes, lifetime retirement, seeded repeatability, continuous loops, missing attachments, caps, moving world trails, locomotion, equipped-bowl hook, unequip and disconnect.
- Existing native frontend handlers pass mouse/keyboard/controller navigation and held-input protections.
- Real server-backed consolidated flow passes named House/Scion creation, selected appearance, perspective gameplay, title, character management and Continue after reconnect.
- Production captures inspected at 1920x1080 and 640x480; 960x600 Scion roster also inspected. The first capture fixture incorrectly used the legacy camera; it now explicitly requires perspective and the GPU atlas trace. Early compilation errors and the too-early retirement assertion were corrected. Narrow-window text overlap and distorted Chronicle endcaps were fixed after visual inspection.

Content limits (not new gameplay): Burning Touch and a flying projectile are not existing native actions. Their recipes and trail API are shipped and captured; no fake damage/spell system was added. The fire-bowl hook currently uses equipped display names and centralized facing-based hand offsets. A playable fire-bowl item and per-frame exported sockets remain content work.

Final package and promotion results are recorded below.

## Final verified package and normal launch

Installed executable: `C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe`
Packaged source: `3cee73d2667dfcf7e77b4cdd910399bcc20b933f` (clean).
Package: `native/build/player-package-3cee73d26/`.
Entry SHA256: `90CDA81348E5BAF49B86B8C3FFFA0B69C234AFC311FF8E69D63404B663BEAF7B`.
Client SHA256: `DA67BEDF65AC42C97838661431EDBC4E7D8BEBE78C392C0E139834B7614EEF11`.

Completed checks:
- All **81** scenarios passed in the exact packaged client; no failed assertions. Includes perspective/animation, gameplay, complete button-based House/Scion flow, inventory/equipment, coin/item retention and bank recovery.
- All 1281 package hashes and 359 mandatory runtime resources validated; embedded source identity is clean.
- The same top-level executable completed save and reload lifecycles using one isolated QA profile. **Effects 70%** persisted; this is the audio setting, not a completion percentage.
- Sustained 3440x1440 inventory dragging: **34.582 ms average**. Moving fullscreen: **25.3 ms average**. Unchanged 40 ms average limits passed.
- Packaged title, House/Scion roster and VFX phase captures inspected. Normal 3440x1440 title captured from the actual installed launch and inspected; its Continue button and source ID are visible.
- Before promotion, the previous full installation was copied to `native/build/normal-launch-rollback-before-3cee73d26/`. One existing save and normal settings were preserved byte-for-byte during copying. The normal launch then passed source, actual child paths, working directory, normal profile, perspective configuration, menu-confirmed quit and owned-process cleanup.
- The normal save was also byte-identical **after** that launch. Normal settings remained absent. The Crossroads fix and the previously recovered bank coins remain preserved.

The timed VFX phase captures drive a controlled LevelUp event through production presentation for repeatable frames. Real XP threshold emission and the socket seam were verified separately by the native kill/session regressions. Testing used app-owned native windows and hidden launcher diagnostics; Computer Use stayed stopped.

Remaining failed checks: **none**. Remaining content work: fire-bowl item/tag and per-frame sockets, Burning Touch gameplay, and authoritative flying-projectile gameplay. Their effect recipes/API are available as detailed in the module README. No owner process was terminated, no QA profile was promoted, and no alternate normal launch was introduced.

Only this handover/evidence update follows packaged source `3cee73d26`; it contains no further game-code changes. Commit and push completed verified work to `codex/native-consolidated-20260913` and fast-forward the shared `codex/native-reconstitution` baseline under standing authorization. No additional approval checkpoint.
