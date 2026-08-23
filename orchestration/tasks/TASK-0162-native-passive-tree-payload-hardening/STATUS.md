task: TASK-0162
state: CLAIMED
lane: ox-pc-bc
model: openrouter/stealth/ox-alpha
base_commit: dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17
branch: worker/verdigris/pc/ox-pc-bc
---

CLAIMED: TASK-0162 (native passive-tree payload hardening) on worker branch
worker/verdigris/pc/ox-pc-bc. Preflight proved: clean tree, HEAD
51a025c0d104cd0000aa5a8a994bf40e052233f2 in sync with its upstream tracking
ref, and the immutable SPEC base dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17 is a
strict ancestor of HEAD (TASK-0156's accepted mirror work is already
contained). Work will be confined to the owned paths:
native/client/remote_session.cpp, native/tests/session_tests.cpp, and this task
folder. No server, wire-protocol, UI, save, balance, or content authority is
touched; port 6500 is never used (loopback capsule 7160-7179 only).
