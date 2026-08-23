task: TASK-0162
state: REVIEW_REQUESTED
lane: ox-pc-bc
model: openrouter/stealth/ox-alpha
base_commit: dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17
branch: worker/verdigris/pc/ox-pc-bc
claim_commit: 1486f0e02a8f073654518f2c7e29d59354e9ecb4
implementation_head: (frozen at push; see git log of worker/verdigris/pc/ox-pc-bc)
---

CLAIMED: TASK-0162 (native passive-tree payload hardening) on worker branch
worker/verdigris/pc/ox-pc-bc. Preflight proved: clean tree, HEAD
51a025c0d104cd0000aa5a8a994bf40e052233f2 in sync with its upstream tracking
ref, and the immutable SPEC base dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17 is a
strict ancestor of HEAD (TASK-0156's accepted mirror work is already
contained). Work confined to the owned paths: native/client/remote_session.cpp,
native/tests/session_tests.cpp, and this task folder. No server, wire-protocol,
UI, save, balance, or content authority touched; port 6500 never used
(loopback capsule 7160-7179 only).

REVIEW_REQUESTED: implementation hardens the native passive-tree mirror in
native/client/remote_session.cpp — fail-closed validation (schemaVersion must
be the number 2; points.skill and earned sane nonnegative integral numbers;
nodes/conduits arrays under a documented transport entry bound) with the last
valid snapshot preserved untouched on any malformed/incomplete envelope and
exactly one deterministic `passiveTree rejected: <reason>` ProtocolError
diagnostic per rejection; absence remains legal and silent; the single cap is
documented as a transport bound encoding no balance. Focused session tests in
native/tests/session_tests.cpp drive the REAL production parser end to end via
a test-only scripted loopback WebSocket server (capsule 7160-7179): valid
absent/zero/nonzero behavior unchanged across all three call sites, a 20-case
invalid battery (missing fields, wrong types, fractional, negative,
bare-Infinity token, 1e400 inf, int-cast overflow, future schemaVersion,
non-object tree, oversized arrays), byte-stable repeated diagnostics, snapshot
preservation after every invalid frame, and healthy recovery by a valid
refresh. Literal acceptance commands all passed at EXIT=0:
build.ps1 -RunTests (all suites green, denylist PASS),
verdigris_session_tests.exe ("session tests passed", 79 new ptree PASS lines,
zero FAIL), git diff --check clean, git diff --name-only exactly the two owned
source paths. Transcripts in REPORT.md.
