# OI-002 — production renderer dependency

**State:** WAITING_EVIDENCE on TASK-0114 and architect TASK-0088. **Deadline:**
before adding a Stage-2 production dependency; panels and render-list work
continue now.

Decision required: approve the ADR's recommended backend, approve its runner-up,
or explicitly retain optimized GDI for the next milestone.

Recommended choice: approve the ADR's first-ranked cross-platform backend only
if it proves Windows/macOS viability, batching, shaders, deterministic offscreen
capture, plain MSVC+CMake integration, and acceptable dependency weight. Until
then, retain GDI and the render-list contract.

Viable alternatives: the ADR runner-up; or a time-boxed GDI continuation with a
named migration trigger. Acceptance rubric: no simulation coupling, no loss of
headless tests/captures, reproducible clean build, explicit license/provenance,
and an owner-visible scene at both 1920x1080 and 1366x768. No asset generation
is requested. Work that continues: 0079/0093 panels/typography contracts,
reference captures, protocol journeys, and content audits.
