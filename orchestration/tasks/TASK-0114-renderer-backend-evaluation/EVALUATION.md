# TASK-0114 — Stage-2 renderer backend evaluation matrix

- **Lane:** ox-pc-bb (`openrouter/stealth/ox-alpha`)
- **Base commit:** `d2423873c577d299b3b39c56024d1d840993c72b`
- **Branch:** `worker/verdigris/pc/ox-pc-bb`
- **Supersedes:** TASK-0073 (`orchestration/tasks/TASK-0073-renderer-backend-eval/SPEC.md`, same candidate list, now executed against the shipped GDI client)
- **Evaluated upstream pins and access dates:** all web sources accessed **2026-08-23**; per-candidate version pins in `captures/source-index.json` (`upstream_pins`) and in the Source Index below.
- **Resource capsule honored:** read-only research; no downloads, builds, dependencies, or ports; port 6500 untouched.

**Citation convention:** `[S01]`–`[S20]` refer to `captures/source-index.json`. Repository references use `path:line` form against the base commit's tree.

**Status of this document:** evaluation only. It recommends two candidates for the Stage-2 ADR; it does **not** decide anything. The ADR belongs to the architect + owner (`owner_input_dependency: owner approves any production dependency after ADR`). Frozen invariants respected throughout: core simulation untouched, render-list determinism preserved, headless tests preserved, no package manager introduced.

---

## 1. Baseline: what exists today

The native client is a Win32/GDI painter with a semantic recording layer:

- `native/client/render_list.hpp:16-45` defines `render::Op` (Floor, Tile, Scenery, Player, Monster, Telegraph, Swing, Sweep, WarCry, Impact, Death, Damage, TargetFlash, ScreenPulse, Drop, Extraction, Hud, Orb, Quickbar, Minimap, PaneStat, PaneWeapon, PaneItem, PaneBanked, Chronicles, HouseChip). One `render::List` is recorded per presented frame, next to the actual draw calls, so headless scenarios assert on semantics instead of pixels.
- `native/client/main.cpp` paints via memory DC + DIB sections: `StretchBlt` sprite scaling (main.cpp:1388), `AlphaBlend` loaded dynamically from msimg32 with premultiplied alpha (main.cpp:64, 650), `TextOutA` text, `FillRect` panels; offscreen capture via `CreateCompatibleBitmap` + `BitBlt` (main.cpp:3889-3892).
- `native/CMakeLists.txt:75-78` links only `user32 gdi32` on Windows — zero third-party renderer dependencies.
- Build gates are plain MSVC (`./native/build.ps1`) plus CMake/ctest elsewhere (`native/README.md:30-34, 74-79`).

This is exactly the "optimized GDI" null option evaluated below — any migration must beat it on evidence, not vibes.

---

## 2. Evaluation criteria

From the SPEC (all mapped): Windows/macOS viability, sprites, atlases, shaders, text rendering, offscreen capture, resource lifetime, plain MSVC/CMake integration without a package manager, licenses, binary/dependency weight, migration from render-list ops.

---

## 3. Candidate evaluations

### 3.1 Direct3D 11

| Criterion | Finding |
| --- | --- |
| Windows viability | First-class: D3D11 is a Windows/C++ API, minimum supported client Windows 7 [S01]. Feature-level model scales from FL 9_1 to 12_2 on a single API surface [S02]. |
| macOS viability | **None.** The D3D11 documentation declares "Supported runtime environments: Windows/C++" [S01]. There is no Apple-side D3D11 implementation; a macOS port would need a second backend written against Metal or GL. This alone fails the cross-platform criterion unless D3D11 hides behind an abstraction. |
| Sprites | Textured quads via shader pipeline; instancing available from FL 9_3 upward with caveats [S02]. No built-in sprite API — everything is hand-rolled geometry + texture sampling. |
| Atlases | Full control: sample a sub-rectangle of a `ID3D11Texture2D` with UV offsets; NPOT textures unconditionally supported at FL 10_0+ [S02], so atlas sizes are unconstrained on any realistic target. |
| Shaders | HLSL with shader model 5.0 at feature level 11_0; SM 5.1+ requires the D3D12 API [S02]. Runtime compilation uses D3DCompile in d3dcompiler_47.dll, redistributable side-by-side with the application from the Windows SDK redist folder [S05]. |
| Text | Nothing built-in. Options are bitmap/sprite fonts, or DirectWrite/D2D interop (Direct2D is part of the Windows SDK graphics stack [S05]). Interop adds complexity around device/context sharing. |
| Offscreen capture | Render into an offscreen render target, then `CopyResource` into a CPU-readable staging texture and read it back with `ID3D11DeviceContext::Map` (D3D11_MAP_READ) [S04]. Deterministic and well-trodden; WARP software rasterizer makes GPU-less CI hosts viable — valid on all feature levels under Direct3D 11.1 (Windows 8+) [S03]. |
| Resource lifetime | Explicit COM refcounting (`ID3D11DeviceChild` releases); device-removed handling surfaces through Map/HRESULTs [S04]. Verbose but deterministic if ownership is centralized. |
| MSVC/CMake integration | Headers/libs ship in the Windows SDK since Windows 8 [S05]; plain `target_link_libraries(... d3d11 dxgi)` — no package manager needed. Matches the existing `build.ps1` + CMake layout directly. |
| License | OS/SDK component; no third-party license to ship. The only redistributable is d3dcompiler_47.dll, which Microsoft explicitly permits shipping side-by-side [S05]. |
| Binary/dependency weight | Zero new DLLs beyond the optional compiler (~few MB) if runtime HLSL compile is used; can be avoided by precompiling shaders offline (FXC ships in the SDK bin [S05]). |
| Migration from render-list ops | Mechanical for geometry ops (quads/rings), but text (`Hud`, `Pane*`, damage labels) needs a font path first. Capture path changes from memory-DC BitBlt to staging-map. |

### 3.2 OpenGL 3.3 Core

| Criterion | Finding |
| --- | --- |
| Windows viability | Yes, via WGL. The Khronos registry supplies `GL/glcorearb.h` — core profile interfaces with function-pointer typedefs because Windows exposes only GL 1.x entry points statically ("dynamic runtime extension queries, such as Linux and Microsoft Windows") [S08]. |
| macOS viability | Partially verifiable only. SDL2 lists macOS as a supported CMake platform [S16] and sokol_app lists macOS with GL 3.3 among its platform/API matrix [S10], so vendors still ship GL 3.3 contexts there today. However, the widely reported Apple deprecation of desktop OpenGL could **not** be re-verified from a primary source during this evaluation — see Unknown U1. Risk posture: usable, but with an unresolved platform-policy overhang. |
| Sprites | Textured quads; VAO/VBO drawing is standard core profile. All batching is hand-rolled (no abstraction layer in the spec). |
| Atlases | Sub-rect UV sampling; NPOT support is guaranteed in core profiles since GL 2.0-era extensions were folded into the specification lineage (3.3 core assumes full NPOT) — verified capability of the 3.3 core document set [S08]. |
| Shaders | GLSL 3.30 (spec link in registry [S08]); sokol's header notes its desktop-GL path accepts `#version 410`/`#version 430` sources and flags that 430 "is not available on macOS" [S13] — a live example of macOS GL lagging. |
| Text | Nothing in the spec; loaders like sokol integrate external text stacks (sokol pairs GL with sokol_fontstash/sokol_debugtext utilities) [S10]. Raw GL means bringing FreeType/stb/fontstash-class machinery or a bitmap font. |
| Offscreen capture | FBO rendering + `glReadPixels` into client memory, optionally through `GL_PIXEL_PACK_BUFFER` [S09]. Straightforward and synchronous. |
| Resource lifetime | Manual GL object handles with context affinity; correctness depends on disciplined wrapper code (no refcounting). Sokol's integrated loader notes show how much boilerplate is normally hidden [S13]. |
| MSVC/CMake integration | No library to link on Windows (`opengl32.dll` is system-provided); needs a function loader (e.g. glad, MIT license with Apache-2.0 Khronos headers [S19]) plus WGL context scaffolding — all vendorable source, no package manager. |
| License | Specification/headers are Khronos IP (Apache-2.0 for the distributed headers per glad's bundled licensing note [S19]); the runtime is the vendor driver — nothing extra to ship or license. |
| Binary/dependency weight | Effectively zero shipped bytes (system driver + one small generated loader TU). |
| Migration from render-list ops | Same shape as D3D11 minus the COM ceremony; context creation and pixel-format selection add Windows-specific friction the other candidates hide. |

### 3.3 SDL2 + explicit batching layer

(SDL pinned: `release-2.32.10`, published 2025-09-01 [S14]. Note: SDL mainline has moved to SDL3 — release-3.4.14 published 2026-08-03 [S14]; the SPEC pins SDL2, so SDL2 is evaluated as-is with the divergence recorded as risk R6.)

| Criterion | Finding |
| --- | --- |
| Windows viability | Yes — Microsoft Visual C is an explicitly supported CMake platform [S16]. |
| macOS viability | Yes — macOS/iOS/tvOS with Xcode support listed in the official CMake docs [S16]. |
| Sprites | `SDL_Renderer` textures + `SDL_RenderCopyEx`; since SDL 2.0.18, `SDL_RenderGeometry` renders a batched triangle list with optional texture, indices, and per-vertex color/alpha [S18] — a natural substrate for an explicit batching layer on top. |
| Atlases | `SDL_RenderCopy`/`SDL_RenderGeometry` take source rectangles [S18], so atlas sub-rect quads work directly; no format constraints documented beyond SDL pixel-format enums. |
| Shaders | None exposed by SDL2's render API. Custom effects would require dropping beneath `SDL_Renderer` (raw D3D/GL behind SDL windows) — the batching layer must own all effect rendering as pre-baked atlas frames or vertex tricks. This is the main functional ceiling vs. candidates above. |
| Text | Not in core SDL2; historically `SDL_ttf` (separate repo/library). Within this evaluation: treat text as a bitmap-font atlas rendered through the same geometry batcher; exact SDL_ttf licensing was **not** fetched (out of capsule scope to add deps) — flagged in Unknown U4. |
| Offscreen capture | `SDL_RenderReadPixels` reads the current render target into client memory [S17]; documented as very slow and must be called before `SDL_RenderPresent` [S17] — acceptable for test captures, not per-frame work. Headless option: `SDL_CreateSoftwareRenderer` (part of SDL2's render API family) renders without a window. |
| Resource lifetime | SDL object model (`SDL_CreateTexture`/`SDL_DestroyTexture`), C-style, no RAII burden; lifetime tied to renderer — simpler than raw D3D/GL. |
| MSVC/CMake integration | First-party: vendored `add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)` or `find_package(SDL2 REQUIRED CONFIG ...)` with `SDL2::SDL2` targets [S16]. Vendoring fits the no-package-manager invariant cleanly. |
| License | zlib ("Copyright (C) 1997-2025 Sam Lantinga") [S15] — static linking friendly, no copyleft obligations. |
| Binary/dependency weight | Compressed release assets: `SDL2-2.32.10-win32-x64.zip` = 608,965 bytes; `SDL2-devel-2.32.10-VC.zip` = 7,172,966 bytes [S14]. Uncompressed DLL size not measured (Unknown U2). One runtime DLL (or static lib) replaces several hand-written subsystems (window/input/timing). |
| Migration from render-list ops | Strongest structural fit below the abstraction line: window/input move to SDL, draws become one geometry batcher fed by the existing op stream; capture via `SDL_RenderReadPixels` mirrors today's memory-DC snapshot role [S17]. |

### 3.4 sokol_gfx

(Pinned: floooh/sokol master `7cee0ba17c358985e4744fe8ac20b6829d328229`, dated 2026-08-18 [S11]; zlib license [S12].)

| Criterion | Finding |
| --- | --- |
| Windows viability | Yes — D3D11 backend; MSVC auto-links system libraries via in-source `#pragma comment(lib, ...)` [S10, S13]. |
| macOS viability | Yes — Metal and GL 3.3 backends; sokol_app supports macOS (Win32/macOS/Linux X11/iOS/WASM/Android/UWP) [S10]. Caveat: macOS/iOS builds using sokol_app or Metal must be compiled as Objective-C(++) [S10], and GLSL `#version 430` features are "not available on macOS" on the GL path [S13]. |
| Sprites | Buffers/images/pipelines/primitives only; sprites are textured quads you define. `util/sokol_gl.h` provides an OpenGL-1.x-style immediate-mode layer on top of sokol_gfx [S10] — a ready-made seed for the explicit batching layer. |
| Atlases | `sg_image` + UV-offset quads; pitch helpers (`sg_query_surface_pitch`) exist for upload packing [S13]. |
| Shaders | Per-backend sources or blobs required; no unified shader language in the header itself. The companion tool sokol-shdc compiles one annotated GLSL file to backend-specific headers [S10, S13]. On D3D11, sokol-gfx dynamically loads `d3dcompiler_47.dll` when given HLSL source [S13]. |
| Text | No core text. Utility stack covers it: `util/sokol_fontstash.h` (fontstash backend) and `util/sokol_debugtext.h` ("simple text renderer using vintage home computer fonts") [S10]. |
| Offscreen capture | Offscreen render passes into image objects with attachment views are first-class [S13]. However: **full-text search of the pinned sokol_gfx.h found no public GPU→CPU pixel-readback function** (no sg_read_pixels/staging-map equivalent; grep for readback/read-back/D3D11_MAP_READ/GL_PIXEL_PACK returns nothing) [S13]. Readback would have to go through the documented raw-API escape hatch (`sg_reset_state_cache()` then call the underlying API directly [S13]) — feasible in principle, unverified in practice (Unknown U3). For logic tests, `SOKOL_DUMMY_BACKEND` "replaces the platform-specific backend code with empty stub functions… useful for writing tests that need to run on the command line" [S13] — an excellent fit for the existing headless scenario runner, though a dummy backend cannot produce pixels. |
| Resource lifetime | Handle-based slot pools created/freed inside `sg_setup`/`sg_shutdown` bounds with declared pool sizes [S13]; destruction order matters less than raw GL, more than SDL. |
| MSVC/CMake integration | Single-header C, compiled per translation unit; no package manager, no generated project files; README documents build-without-a-build-system integration [S10]. Drop-in compatible with `build.ps1` + CMake. |
| License | zlib, "(c) 2018 Andre Weissflog" [S12]. |
| Binary/dependency weight | Source-only; zero DLLs (except transient d3dcompiler_47.dll loading when HLSL-source shaders are used [S13], avoidable with precompiled blobs). Compiled-size figures unmeasured here (Unknown U5 — no builds allowed). |
| Migration from render-list ops | Ops map onto sokol_gl-style command emission naturally; windowing stays ours (Win32 adapter or sokol_app swap-in); the missing readback path is the one capture-plan gap (U3). |

### 3.5 Optimized GDI (null option)

| Criterion | Finding |
| --- | --- |
| Windows viability | Total — GDI "enables applications to use graphics and formatted text on both the video display and the printer"; applications never touch hardware directly [S06]. |
| macOS viability | None. GDI is Windows-only [S06]; macOS keeps the console-fallback client until a second backend exists regardless of this choice. |
| Sprites | Blit-based: `AlphaBlend` displays bitmaps with transparent/semitransparent pixels; blend functions are "currently limited to AC_SRC_OVER", constant alpha combines with per-pixel alpha, and display DCs support all blending operations [S07] — this is the current production path (main.cpp:64,650). |
| Atlases | Works today as source rectangles into shared DIB sections (current asset kit behavior); scaling via StretchBlt [S07 remarks on stretch modes]. |
| Shaders | **None.** GDI exposes no programmable pipeline anywhere in its reference documentation [S06]; orb/aura/post-processing effects cap out at what alpha-blitted layers and manual raster math can fake. This is the hard ceiling motivating Stage 2. |
| Text | Solid: `TextOutA`/fonts are core GDI competency ("graphics and formatted text") [S06]; already used across the HUD (main.cpp:1750, 2113+). Best-in-evaluation text story. |
| Offscreen capture | Memory DC + `CreateCompatibleBitmap` + `BitBlt` [repository evidence; main.cpp:3889-3892] — already deterministic, already wired into captures and scenario evidence. |
| Resource lifetime | GDI object leaks are classic (HBRUSH/HPEN/HBITMAP); current code manages them manually. No change if retained. |
| MSVC/CMake integration | Already done (`gdi32` linked, `build.ps1` green) [repository evidence]. |
| License | OS component; nothing to ship [S06, S07 requirements tables: Msimg32.lib/Msimg32.dll]. |
| Binary/dependency weight | Zero added bytes — the floor no other candidate can reach. |
| Migration from render-list ops | N/A (it is the incumbent); "optimization" scope is batching blits, dirty rects, and caching — preserves every existing seam. |

---

## 4. Comparison matrix

Legend: ● strong / ○ partial or conditional / ✕ absent. Every cell traces to section 3; absence claims cite the reviewed references rather than folklore.

| Criterion | D3D11 | GL 3.3 core | SDL2 (+batcher) | sokol_gfx | GDI (null) |
| --- | --- | --- | --- | --- | --- |
| Windows viability | ● [S01] | ● [S08] | ● [S16] | ● [S10,S13] | ● [S06] |
| macOS viability | ✕ [S01] | ○ (policy status unknown, U1) [S10,S16] | ● [S16] | ● [S10] | ✕ [S06] |
| Sprites | ○ hand-rolled | ○ hand-rolled | ● RenderGeometry [S18] | ○ sokol_gl seed [S10] | ● AlphaBlend today [S07] |
| Atlases | ● [S02] | ● [S08] | ● [S18] | ● [S13] | ● (source rects) |
| Shaders | ● SM 5.0 [S02,S05] | ● GLSL 330 [S08] | ✕ none exposed [S17,S18 context] | ● per-backend + shdc [S13] | ✕ none [S06] |
| Text | ○ bring-your-own [S05] | ○ bring-your-own [S10] | ○ bitmap-font route (SDL_ttf unverified, U4) | ○ fontstash/debugtext utils [S10] | ● TextOutA [S06] |
| Offscreen capture | ● staging+Map [S04]; WARP for CI [S03] | ● glReadPixels [S09] | ● RenderReadPixels [S17] | ○ offscreen passes yes; readback undocumented (U3) [S13] | ● memory DC (incumbent) |
| Headless tests | ○ WARP [S03] | ○ osmesa-class paths unverified (U6) | ○ software renderer | ● dummy backend [S13] | ● current harness |
| Resource lifetime | ○ COM, verbose [S04] | ○ manual handles | ● simple C objects | ● pooled handles [S13] | ○ manual GDI objs |
| MSVC/CMake, no pkg mgr | ● SDK-native [S05] | ○ loader+WGL glue [S08,S19] | ● vendored subdir/targets [S16] | ● single-header [S10] | ● done |
| License | SDK EULA; d3dcompiler redistributable [S05] | Khronos headers Apache-2.0; drivers vendor [S19] | zlib [S15] | zlib [S12] | OS component [S06,S07] |
| Binary/dep weight | ~0 (+optional compiler DLL) [S05] | ~0 (+loader TU) [S19] | one DLL/static lib; x64 zip 608,965 B compressed [S14] | source-only (size U5) [S12,S13] | 0 |
| Render-list migration fit | ○ | ○ | ● | ● | ● incumbent |

---

## 5. Risk table

| ID | Candidate | Risk | Likelihood | Impact | Mitigation sketch |
| --- | --- | --- | --- | --- | --- |
| R1 | D3D11 | No macOS path at all [S01] | Certain | High | Only viable behind a second backend or as Windows-only stage; contradicts cross-platform presentation goal unless explicitly scoped |
| R2 | GL 3.3 core | macOS platform policy unresolved (deprecation claim unverifiable here, U1); GL feature ceiling per Apple shown by sokol's macOS GL restrictions [S13] | Medium | Medium-High | Prefer abstraction (sokol) over raw GL; owner verifies Apple position before any GL-first ADR |
| R3 | GL 3.3 core | Context/loader boilerplate on Windows increases determinism-sensitive surface (pixel formats, driver quirks) | Medium | Medium | Vendor a known loader (glad, MIT [S19]) and pin context creation code; golden-capture parity suite |
| R4 | SDL2 | No shader access under `SDL_Renderer`; orb/aura/post effects must bake into atlases/vertex tricks [S17,S18] | High | Medium-High | Accept stylized-effects ceiling, or accept future drop beneath SDL_Renderer to raw APIs (complexity jump) |
| R5 | SDL2 | Capture performance documented as "very slow" [S17] | Certain (per-call) | Low | Restrict to test-time captures (same role as today's snapshots); never per-frame |
| R6 | SDL2 | Line divergence: SDL2 latest 2.x published 2025-09-01 while SDL3 mainline released 3.4.14 on 2026-08-03 [S14]; SPEC pins SDL2 → potential maintenance drift, and sdl2-compat-era ecosystem pressure | Medium | Medium | If shortlisted, record explicit pin + review trigger; do not start on SDL3 without a new task (supersede discipline) |
| R7 | sokol_gfx | No public GPU→CPU readback at pinned SHA [S13]; capture plan depends on raw-API escape hatch [S13] (unverified, U3) | Medium | High | Prototype capture spike first if shortlisted (needs a dependency-approval milestone; outside this capsule) |
| R8 | sokol_gfx | Shader workflow requires per-backend sources or the sokol-shdc toolchain step [S13] | Certain | Low-Medium | Either adopt shdc at build time or hand-maintain tiny HLSL/GLSL pairs (2D-only surface is small) |
| R9 | sokol_gfx | macOS Metal unit must compile as Objective-C++ [S10] — build-system wrinkle for the CMake lane | Low | Low | Isolate one `.mm` TU behind the platform seam |
| R10 | GDI null | Effects ceiling: no programmable pipeline documented [S06]; per-op CPU cost grows with scene density | Certain | Medium | Keep as fallback baseline; invest in blit batching/dirty rects only if ADR defers Stage 2 |
| R11 | All non-null | Any dependency addition waits on owner approval post-ADR (`owner_input_dependency`); evaluation deliberately adds nothing | — | — | Sequencing constraint, not a blocker |

---

## 6. Migration sketch (applies to whichever winner the ADR picks)

The load-bearing idea: **the semantic render-list stays the contract**; only the paint sink swaps. `render_list.hpp` recording stays co-located with draws (per the harness discipline in `native/README.md:121-124`).

```text
Phase 0 (all options)   Freeze golden captures + scenario suite as parity oracle.
                        Current oracle: render::List assertions + PNG snapshots
                        (memory-DC BitBlt today, main.cpp:3889-3892).

Phase 1 Platform seam   Introduce native/platform/ window/input adapter.
                        - SDL2 route: SDL_CreateWindow + event pump [S16].
                        - sokol route: keep Win32 adapter, or adopt sokol_app
                          later (macOS TU becomes Objective-C++) [S10].
                        Simulation untouched (command/event boundary intact,
                        constitution §Native architecture invariant).

Phase 2 Paint seam      Extract paint_scene's GDI calls behind a Painter
                        interface with two impls:
                        - GdiPainter (today's code, unchanged)
                        - BatchPainter: interleaved quad vertices
                          {pos2f, uv2f, rgba8} + one atlas + one pipeline;
                          FillRect→quad, sprite blit→textured quad,
                          Ellipse ring→triangle fan, TextOut→bitmap-font
                          quads (fontstash/debugtext class utils for
                          sokol [S10]; plain atlas for others).
                        Each BatchPainter emit records the SAME render::Op
                        next to the draw (determinism preserved).

Phase 3 Effects         Shader-capable backends add orb/aura/post passes
                        (D3D11 SM5.0 [S02] / GLSL 330 [S08] / sokol shdc
                        [S13]). SDL2 caps at baked-atlas effects [R4].

Phase 4 Capture parity  Per backend:
                        - D3D11: offscreen RT → CopyResource → staging Map [S04]
                          (WARP for GPU-less CI [S03])
                        - GL: FBO → glReadPixels [S09]
                        - SDL2: SDL_RenderReadPixels before present [S17]
                        - sokol: offscreen pass [S13] + raw-API readback
                          escape hatch [S13] — SPIKE FIRST (R7/U3)
                        - GDI: unchanged memory DC
                        PNG encode + byte-compare against Phase 0 goldens;
                        semantic render-list asserts run everywhere including
                        sokol's dummy backend for pure CLI runs [S13].
```

Rollback: GDI remains the default Painter until BatchPainter passes the full parity suite; no commit flips the default without green scenarios.

---

## 7. Recommendation (two candidates — not a decision)

**Recommend ADR consideration of:**

1. **sokol_gfx (+ explicit batching layer)** — best invariant fit: zlib single-header source with no package manager [S12,S10], D3D11+Metal+GL3.3 coverage for Windows/macOS [S10], a dummy backend purpose-built for command-line tests matching our headless harness [S13], and pooled resource lifetimes. The open item is the capture path (R7/U3) — make the readback spike its first milestone.
2. **SDL2 2.32.10 (+ explicit batching layer)** — lowest total-risk integration: zlib [S15], first-party vendored-CMake on MSVC and macOS [S16], batched-geometry substrate already present (`SDL_RenderGeometry`, 2.0.18+ [S18]), trivially slow-but-sufficient capture [S17], and it also absorbs the platform seam (window/input) the constitution reserves. Costs: no shader access (R4) and the SDL2-vs-SDL3 fork question (R6) must be answered in the ADR.

**Not recommended for the ADR shortlist, with reasons on record:** Direct3D 11 alone (fails macOS outright [S01] — it could serve as sokol's *backend*, which is exactly what recommendation 1 provides); raw OpenGL 3.3 core (viable but strictly dominated by sokol's GL path once an abstraction exists, and carries the unverifiable macOS-policy risk R2/U1); optimized GDI (retain as the null baseline and rollback path — its shader ceiling [S06] is the reason Stage 2 exists).

The decision itself belongs to the architect + owner ADR.

---

## 8. Explicit unknowns and negative controls

Preserved per SPEC ("preserve at least one material UNKNOWN or unsupported candidate fact rather than inferring it"):

- **U1 (negative control, material):** Apple's macOS OpenGL deprecation statement. Expected primary (`developer.apple.com/library/archive/releasenotes/General/RN-osx10.14/index.html`) returned HTTP 404 live on 2026-08-23, and no Wayback Machine snapshot was located under either `library/archive` or `library/content` paths [S20]. The deprecation claim is therefore **not asserted** anywhere in this document; macOS-GL viability rests only on what vendors demonstrably support today [S10,S16]. Owner should verify Apple's current policy from a primary source before any GL-touching ADR.
- **U2:** Uncompressed `SDL2.dll` size. Measuring it requires downloading a release artifact — forbidden by the capsule. Only compressed asset sizes from GitHub's release metadata are cited [S14].
- **U3:** Whether sokol's raw-API escape hatch yields a practical capture path (and its per-backend effort). Conceptually supported by the documented `sg_reset_state_cache()` contract [S13]; practically unverified because building/prototyping is forbidden here.
- **U4:** SDL_ttf licensing/weight was left unfetched; text-via-SDL2 is sketched as a bitmap-font atlas instead. Do not assume SDL_ttf terms without checking them.
- **U5:** Compiled binary weight of a sokol-based painter. Requires a build — forbidden; no numbers claimed.
- **U6:** Headless GL capture on GPU-less CI (osmesa-class) was not researched to a primary source; WARP [S03], SDL software rendering, and sokol's dummy backend [S13] cover the near-term CI need without asserting anything about osmesa.

Unsupported-fact hygiene: GDI driver-level acceleration characteristics across modern GPUs are **not** claimed anywhere (the fetched primaries describe API behavior, not driver internals [S06,S07]).

---

## 9. Source index

See `captures/source-index.json` (machine-validated by the acceptance commands). Summary:

- **Microsoft Learn** (accessed 2026-08-23): S01 D3D11 portal · S02 feature levels · S03 WARP/reference limitations · S04 `ID3D11DeviceContext::Map` · S05 Where is the DirectX SDK? · S06 GDI portal · S07 `AlphaBlend`.
- **Khronos** (accessed 2026-08-23): S08 OpenGL Registry (3.3 core spec link; glcorearb.h) · S09 `glReadPixels` reference page.
- **floooh/sokol** (accessed 2026-08-23): S10 README@master · S11 commits API (pin `7cee0ba1…`, 2026-08-18) · S12 LICENSE (zlib) · S13 sokol_gfx.h @ pin (backend defines, dummy backend, offscreen passes, d3dcompiler_47 dynamic load, **no public readback API found by full-text search**).
- **libsdl-org/SDL** (accessed 2026-08-23): S14 releases API (2.32.10 @ 2025-09-01; SDL3 3.4.14 @ 2026-08-03; asset sizes) · S15 LICENSE.txt@release-2.32.10 (zlib) · S16 README-cmake.md@release-2.32.10 · S17 wiki `SDL_RenderReadPixels` · S18 wiki `SDL_RenderGeometry`.
- **Dav1dde/glad** (accessed 2026-08-23): S19 LICENSE (MIT; Khronos headers Apache-2.0).
- **Apple Inc.**: S20 unretrievable expected primary — recorded as U1, not cited for any claim.
- **Repository evidence** (base-commit tree): `native/client/render_list.hpp`, `native/client/main.cpp` (GDI paint/capture lines), `native/CMakeLists.txt`, `native/README.md`.
