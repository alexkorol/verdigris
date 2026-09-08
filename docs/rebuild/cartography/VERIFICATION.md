# Cartography checkpoint verification

2026-09-08, `codex/cartographer-expeditions`, isolated worktree based on native production commit `2b5da07b1`.

## Passing gates

- Native MSVC build and legacy denylist: PASS.
- 1,200 C++ seed/size/recipe invariant cases: PASS.
- 1,200 C++/JavaScript fingerprints: PASS, covering tiles, graph, sockets, room variants/depth/tier and encounter population.
- Core tests, including fifteen real WorldSimulation theme/layout combinations, safe nonoverlapping monster placement and reachable treasure: PASS.
- Networking, camera, presentation event and audio mixer tests: PASS.
- Native session tests: PASS. Normal-protocol Gate-B journey covers death, successor admission, heirloom recovery and reconnect. The older combat-seam test retains its existing developer teleport setup for boss telegraph coverage; it does not claim a full manual walk to the guardian.
- `verdigris_client.exe --scenario all`: 62 scenarios, zero failures, exit 0. Full output: [scenarios.log](scenarios.log).
- New cartography scenario: real renderer/session interface, generated material, discovery, hidden guardian, same-scene fog reset. Generated scene paint 8.18 ms at 1366x768 in the full run (6.78 ms in the isolated run).
- Existing fullscreen frame benchmark: 28.0 ms average over 20 frames at 3440x1440, 12 logical CPUs, Win32/GDI, below the unchanged 40 ms bound. This is this machine's result, not a universal performance guarantee.

## Live inspection

Launched the actual server and client with `native/tools/play-native.ps1 -Port 6532`. Entered the saved test Scion, took the tin road and toggled M in the real window. Captured with the repository's `capture-window.ps1` and visually inspected [the generated entrance](live-world.png) and [the discovery atlas](live-atlas.png). The entrance is clear of generic trees, the stone material is readable, the exterior is void and the chart labels only discovered ground. The playtest was closed afterward. Movement/collision journeys are established by the session tests; the brief UI key press was not used as movement evidence.

The first new render fixture incorrectly omitted both session backends and crashed; it now uses a deterministic ClientModel through IClientSession. Per-frame HALFTONE filtering initially cost 68.6 ms; load-time area filtering of the material reduced that cost. Both issues were resolved before the passing run. The original image file is retained unchanged.

## Shared source receipts

WIZARD source checkpoint: `d383b00d208f62c5922d1be669f23e4219c468fa`.

- `expedition.js` SHA256: `32067AF49AC00957E0EDD570A041EFE192C50446702DADA155A295A71B31D401`.
- `limestone-floor.png` SHA256: `7C30D3B7E19300F6DF503ABBDF37EE47676B67BF9234D12855CE57552768A533`.

Both match the WIZARD files byte-for-byte. Original imagegen prompt: [PROVENANCE.md](../../../native/client/assets/wizard/cartographer/PROVENANCE.md). Technical integration and scope: [CARTOGRAPHY.md](../../../native/CARTOGRAPHY.md).
