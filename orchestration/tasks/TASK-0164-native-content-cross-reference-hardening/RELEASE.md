# RELEASE — TASK-0164 ox-pc-ae claim (architect, 2026-08-22 15:18 PDT)

The ox-pc-ae claim `949508f5c2d1cb74353f57eed61b1a5c2dd392d9`
is released. The initial process stopped before any write because it applied an
upstream check that is invalid for a new branch. Its one authorized clean
activation recovery then pushed the valid claim and made owned-path
implementation edits, but exited before an implementation commit, STATUS
transition, or handoff when OpenCode auto-rejected a requested external temp
path used only for evidence comparison.

The dirty worktree `Z:\Code\.worktrees\verdigris\ox-pc-ae` is quarantined
and remains forensic evidence. Do not reset, clean, reuse, or count it as fleet
capacity. A fresh worker may re-claim TASK-0164 by replacing the released
STATUS.md on a new branch/worktree from the current pushed program base. It
must implement independently from the accepted TASK-0151 schema/seed and the
TASK-0164 SPEC. It may inspect the failed session evidence, but must not copy
uncommitted output as accepted work.

The replacement lane must receive a dedicated temp directory inside
`Z:\Code\.fleet\tmp\<lane>` and use it for any transient evidence capture;
the user's global temp directory is not an authorized task surface.
