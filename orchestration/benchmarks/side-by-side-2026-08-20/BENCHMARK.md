# Side-by-side benchmark — native vs browser (2026-08-20, D-124)

Composites: sxs-01..05 (left = native C++ client in REMOTE mode vs C++
server; right = browser reference). Browser scenes staged via
capture-harness Chronicles login + scripted play on port 6563; native
side = frozen 0070 reference scenes.

## Delta list (drives the P1 presentation wave)

1. GROUND: browser has full terrain tiles; native is flat dark grid.
   -> TASK-0075 (terrain1/terrain4 assets already exist, unused).
2. HUD: browser has ornate orbs + iconed quickbar + minimap + panels;
   native has text bars + boxed labels. -> TASK-0076.
3. Surface density: browser village (walls, paths, fountains, NPCs) vs
   sparse native surface. -> next wave after 0075/0076.
4. Panels/typography: expedition panel, guide banner, chat — later.

Rerun after each P1 task: CAPTURE_PORT=<port> node
orchestration/benchmarks/side-by-side-2026-08-20/browser-scenes.mjs
then scratchpad composite (native side from fresh 0070-style captures).
