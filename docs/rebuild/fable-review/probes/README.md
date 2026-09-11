# Fable renderer regression probes

Run from Windows PowerShell with the Windows SDK and MSVC C++20 installed:

```powershell
& docs/rebuild/fable-review/probes/run_gpu_test.ps1 -SkipBenchmark
& docs/rebuild/fable-review/probes/run_terrain_bake_test.ps1
```

Both runners find the repository from their own location, set it as the working
directory, and put all generated headers, executables, objects, logs and raw
readbacks in `.ci-artifacts/fable-renderer/probes`. Nothing generated is written
beside these retained sources. Pass `-VcVars` to select another installed MSVC
environment. `-CompileOnly` compiles without executing the probe.

The GPU probe uses the actual D3D11 hardware renderer and synthetic textures
with exact expected pixels. It checks alpha, shared projection, affine near
clipping, sprite/mesh depth, foreground ground-layer ordering, HDC byte parity,
continuous blur, quarter-resolution lights/cloud shadows, resource bounds and
recovery. Remove `-SkipBenchmark` to also run twenty 3440×1440 readback frames;
run that measurement only when other performance tests are idle.

The terrain probe extracts the unchanged production `fable_world` helper and
complete `Renderer` prefix at run time, with a recorded SHA256. Only the world
fixture, landmark layout and unused equipment painter are stand-ins. It uses
actual runtime terrain pixels and the real GPU upload path. Checks cover:

- X/Y bucket transitions retaining the old patch with exactly one worker.
- Exact overlapping RGBA and mesh xyz/height parity (512 texels and 44/35
  mesh steps). Texture comparisons exclude the existing outer eight-texel
  interpolation clamp; that patch edge is far beyond ordinary visible travel.
- Return-to-resident, route and asset-generation retirement, retained-RGBA
  recovery after GPU reset, the 40 MiB CPU envelope and destructor joining.

The logs beside this file preserve the reviewed standalone results. They are
fixture evidence, not gameplay acceptance or full-scene FPS measurements.
The production `fable-world` scenario separately exercises authoritative
movement and full-resolution presentation. Initial scene/source loading may
wait for a CPU bake; same-scene movement polls and retains the previous patch.
