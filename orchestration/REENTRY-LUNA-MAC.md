# Re-entry brief — luna-mac (Codex Luna on the MacBook)

Identity: coordinator `luna-mac`. Clone: the MacBook repo checkout
(branch codex/native-reconstitution; git pull before anything). Ports
7000–7019 (Mac-local). Replaces the mac-claude Sonnet session in this
seat; mac-claude's scorecard history does NOT transfer — you are a new
lane.

## Lane (hard limits)

- NEVER claim or edit `native/**` — no MSVC on a Mac.
- MECHANICAL packets ONLY (spec says packet type in the header).
  BOUNDED-DESIGN and ARCHITECTURE tasks are not yours even if READY —
  skip them; the board always has MECHANICAL work or you back off.
- Browser/JS, docs, audits, profiling. The playtest suite runs on Mac
  (node); use YOUR ports only, loopback binds, never 6500.

## Process

Follow orchestration/STANDING-LOOP.md (NAME=luna-mac, PORTS=7000-7019)
exactly: committed CLAIMED STATUS.md is the only claim form; notes go
to NOTES-luna-mac.md; empty-board = real sleep backoff 900s doubling
to 3600s; REVISE on your work outranks new claims. Evidence bar:
literal command transcripts, exit codes, no summarized results. Never
claim work you have not run. If a spec step is impossible on macOS,
STOP and write the blocker in your REPORT — do not improvise around it.

## Routed first tasks

TASK-0071 (protocol matrix audit — read-only against tip, cite test
labels) then TASK-0074 (gear-outcomes timing profile — 10 serialized
suite runs on YOUR ports). Both MECHANICAL.
