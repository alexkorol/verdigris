# ADR-001: choose C++20 for the native runtime

**Status:** accepted for the first native workspace

## Decision

Use portable C++20 for the Verdigris runtime, with a CMake project and a small
platform layer. The initial client uses Win32 on Windows and a deterministic
headless/console fallback on other hosts; renderer and networking remain
separate interfaces. SDL3 or another permissively licensed focused library can
replace the platform shell after profiling and packaging evidence.

## Why now

The sprint needs a runnable native proof, not a prolonged language debate. The
workspace has a usable MSVC 2019 Build Tools compiler but no Rust toolchain.
The core is therefore implemented without third-party dependencies and can be
compiled immediately on this Windows checkout. C++ also has mature Windows and
macOS platform/library coverage, deterministic test tooling, low-level control,
and straightforward integration with a custom renderer.

## Trade-off record

Rust remains attractive for memory safety, explicit ownership, deterministic
testing, and AI-assisted defect prevention. It would require installing and
pinning a toolchain here, and its graphics/window ecosystem would add an early
dependency decision. C++ has broader low-level and profiling familiarity and
lets the first proof use a tiny platform shim, but demands stronger ownership
conventions and sanitizers/tests. The core therefore uses value types, explicit
IDs, no global state, and a denylist check; unsafe optimization is deferred.

| Criterion | C++20 | Rust | Sprint choice |
|---|---|---|---|
| Windows/macOS | mature native toolchains | excellent, toolchain install required | C++ |
| Headless deterministic core | direct | direct | tie |
| Shared client/server core | direct library | direct crate | tie |
| Custom 2.5D renderer | broad low-level options | focused modern options | tie |
| Dependency friction now | MSVC already present | rustc/cargo absent | C++ |
| Debug/profiling | mature native profilers | strong tooling, different workflow | C++ |
| AI-assisted maintenance | explicit conventions required | ownership helps | measured C++ |

Do not use Godot, Unreal, Unity, or Bevy as the governing engine. Do not use
assembly or SIMD before profiling demonstrates a real hotspot.
