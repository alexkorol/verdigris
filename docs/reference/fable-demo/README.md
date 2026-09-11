# Verdigris 2.5D Overhaul — Reference Package

A working 2.5D rendering tech demo (Songs-of-Conquest-style: tilted perspective,
GPU heightmap terrain, pixel-art billboards, dynamic lighting, day/night, DoF)
packaged as the reference for overhauling Verdigris's renderer.

- Play it: open `dist/songs-of-the-mire.html` in a browser (fully self-contained).
- Implement it: start with `HANDOFF.md`, then `docs/ARCHITECTURE.md`.
- Hack it: edit `src/game_template.html`, run `python3 tools/build.py`.

Layout:
  HANDOFF.md            agent brief + phased integration plan
  docs/ARCHITECTURE.md  the rendering design, math, and known pitfalls
  src/game_template.html  reference engine (assets as __ASSETS__ placeholder)
  dist/                 playable build
  assets/               demo sprites/tiles (test-only; Verdigris uses its own art)
  tools/build.py        rebuilds dist/ from src/ + assets/
