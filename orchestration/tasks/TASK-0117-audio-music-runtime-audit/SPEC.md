---
task: TASK-0117
title: Native sound and music runtime audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P1
base_commit: 9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4
owner_visible_contribution: defines the missing production audio path for combat feedback, ambience, UI, and music
dependencies: []
owner_input_dependency: music direction, final sounds, composition, licensing, and mix remain owner-only
owned_paths: [orchestration/tasks/TASK-0117-audio-music-runtime-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no playback, downloads, generation, or ports
---

# Outcome and invariants

Produce `FINDINGS.md` and `captures/audio-surfaces.json` mapping any current
sound hooks/assets, authoritative event inputs, spatial/2D buses, priorities,
voice limits, music states/transitions, ambience, UI cues, device lifecycle,
volume/mute/accessibility settings, deterministic test seams, packaging, and
Windows/macOS backend requirements. Audio consumes events; it never becomes
gameplay authority.

# Acceptance and evidence

```powershell
rg -n "audio|sound|music|volume|mute|device|spatial|voice|ambience" native src server docs --glob "*.md" --glob "*.cpp" --glob "*.hpp" --glob "*.js" --glob "*.vue" --glob "*.json"
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0117-audio-music-runtime-audit/captures/audio-surfaces.json','utf8')); console.log('audio surfaces: PASS')"
git diff --check
git diff --name-only
```

Expected: current absence/gaps are explicit and only this folder changes.
Negative control: name a load-bearing combat/UI event with no audio consumer.
Stop before selecting dependencies/assets or inventing music; continue with
backend-neutral interfaces and test strategy.
