# QA routing avoids accidental descent

The exact e2a client evidence at the parent's `native/build/local-pose-verification/clients/A/owned-loot.json` already identifies floor `instance:819b45924206fdd15c09f9c70acff4f2:2`, before intentional town return. Its QA route knew only the upstairs location. The server already publishes descent metadata; no terrain, portal or gameplay policy changes are needed.

This patch adds three descent fields to ClientScene and parses existing `metadata.stairsDown`. Scene replacement already resets ClientScene, so town also retires descent coordinates and availability. The co-op QA driver excludes the descent tile and neighboring margin during combat, loot collection and intentional return. Continuous held-input sample guards apply to both stair types. Only an intentional upstairs return can permit an approach transition from instance to town; a floor-to-floor transition now fails with diagnostic evidence. Loot completion also requires the original cleared instance identity.

Captures and combat navigation logs now include descent coordinates. `approach-navigation.jsonl` records target, source/destination scene, both stairs, transition classification and completion reason. Embedded routing regressions use the recorded upstairs (5,20) and downstairs (34,20) geometry.

## Focused verification

- Compiled current `remote_session.cpp`, `local_session.cpp`, `presentation_state.cpp`, and a wrapper around the existing session test translation unit with MSVC C++20. Rebuilding these consumers accounts for ClientScene's changed layout.
- Linked against private copies of the parent's e2a `core`, `networking`, `seasonal`, `service_store` and `service_transport` objects. The focused real-endpoint `remote_dash_return_retires_exit_and_preserves_banked_result` check passed **17 assertions**, including descent coordinate parsing and reset on town transition. This existing local-review fixture supplies an item through a dev command; its actual return uses authoritative dash/movement.
- A test compiled directly from the exact QA header's helper prefix passed **nine routing assertions**: descent remains excluded with or without extraction enabled, adjacent safe tiles remain routable, held samples cannot cross descent, upstairs is allowed only deliberately, and floor-to-floor transitions cannot count as return.
- Compiled the current `native/client/main.cpp` translation unit, including the changed QA header, successfully. Existing getenv/inet_addr/port-conversion warnings were observed in session compilation; no compile errors.
- Evidence and temporary focused wrappers are under this worktree's ignored `native/build/stairs-qa/`; `session-focus.log` contains the 17 assertion results.

This is focused technical verification. The parent must rebuild all consumers of ClientScene and rerun the original two-client Warden/loot/return reproducer. No graphical acceptance or completed original-route verification is claimed here.
