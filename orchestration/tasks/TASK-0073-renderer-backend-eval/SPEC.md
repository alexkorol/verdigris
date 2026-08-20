---
task: TASK-0073
title: Renderer backend evaluation matrix (Stage 2 prep - research only)
state: READY
packet: BOUNDED-DESIGN
lane: any (research/doc; no build changes)
priority: medium (feeds the Stage 2 renderer ADR)
owned_paths:
  - orchestration/tasks/TASK-0073-renderer-backend-eval/**
forbidden_paths:
  - everything else (RESEARCH ONLY - no code, no deps added)
---

# Outcome

EVALUATION.md comparing candidate rendering approaches for the native
client against the convergence doc Stage 2 criteria: Direct3D 11,
OpenGL 3.3 core, SDL2(+SDL_gpu), sokol_gfx, and staying on optimized
GDI as the null option. Per candidate: Windows+macOS viability, sprite
batching, atlases, shader support (orbs/auras/post), text rendering,
offscreen capture for tests, resource lifetime, build integration with
plain MSVC + CMake (no package manager), binary size/dep weight, and a
migration sketch from the current render_list (ops stay the contract).
Recommend a shortlist of 2 with rationale. Cite sources. NO decision -
the ADR is architect+owner.

# Acceptance

Doc is concrete (tables, not vibes), maps every Stage 2 criterion,
lists real integration risks. Architect reviews; owner reads the
shortlist.
