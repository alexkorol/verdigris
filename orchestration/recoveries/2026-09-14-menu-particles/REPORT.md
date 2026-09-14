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

Final clean package, full packaged checks, normal-entry promotion and remote commits will be recorded below after completion. Existing saves and normal launch remain protected until those gates pass.
