# Mac handoff: ARPG reference archive

Captured on 2026-09-16 from the Windows development repositories and worktrees.

**COMPLETE: all archive refs and sampled downloads verified.**

The new Unity project is unnamed. The owner retired the old game name because
it encouraged unwanted artistic assumptions. This checkout is a historical
reference library: its names, instructions, art experiments, and design docs
are not automatic requirements for the new Unity project. Keep the Unity
project in a separate sibling directory and select references deliberately.

## Get the published reference and this handoff

```sh
git clone --filter=blob:none --branch codex/mac-handoff-20260916 https://github.com/alexkorol/verdigris.git arpg-reference
cd arpg-reference
git fetch origin
```

This handoff branch starts from published `master` at `a684f9a9f55cb7a95e3122a8f4b75fb5ccfff51a`.
It adds only this guide and the archive inventory; it does not merge unfinished
gameplay changes. Authentication to GitHub may be required.

## Useful locations in the published reference

| Location | Contents |
| --- | --- |
| `native/client/assets/first-slice/` | Published player, monster, NPC, scenery, and related first-slice art |
| `native/client/assets/wizard/` | Imported UI frame pieces, item art, orbs, and splash resources |
| `native/client/assets/fonts/`, `production/`, `raster/`, `effects/`, `menu/` | Other runtime presentation assets |
| `native/tools/` | Asset preparation, export, validation, and capture tools |
| `docs/product/` | Historical product and system decisions; review before adopting |
| `docs/reference/`, `docs/art-review/`, `docs/execution/` | Reference studies, review notes, and evidence |
| `docs/rebuild/HANDOFF.md` | Historical native development handoff |

## Recover editable and unpublished first-slice artwork

```sh
git worktree add --detach ../arpg-art-source origin/codex/archive-20260916/wip-verdigris-first-slice-art
```

Snapshot commit: `e3639106dff51da3a674ceedaf93861ea5224ef2`.
The original paths are retained, including ignored `.blend` and `.blend1`
sources, animation frames, previews, masks, manifests, scripts, raw candidates,
and rejected iterations under `native/client/assets/first-slice/`.
An archived candidate is not an approved art direction or a finished asset.

For the earlier Diablo reference study and raster experiments:

```sh
git worktree add --detach ../arpg-reference-study origin/codex/archive-20260916/wip-verdigris-diablo-study
```

## Recover WIZARD source material

```sh
git clone --filter=blob:none --branch codex/archive-20260916/wip-WIZARD https://github.com/alexkorol/WIZARD.git arpg-wizard-source
```

Snapshot commit: `b23009650d75a6575415c27cac138b1fe211b918`.
Useful areas include `tools/rpg_inventory/` (item art, source images, moodboards,
review sheets), `tools/verdigris_splash/` (world scene and source tools),
`tools/wizard_orbs/`, `tools/gui_framekit/`, `tools/cartographer/`,
`tools/arcane_lattice/`, and `tools/geometric_skilltree/`.
WIZARD's other already-published branches remain available on its origin.

The sprite-overhaul repository is also published:
https://github.com/alexkorol/delaford-sprite-overhaul

## Recover outer-workspace notes and sprite queue

```sh
git worktree add --detach ../arpg-workspace-notes origin/codex/archive-20260916/wip-workspace-notes-and-sprites
```

Look under `legacy-workspace/`. This preserves notes and source material that
were outside the real game Git repository, including `sprite_gen_queue/` and
`sprite_samples/`.

## Scope and verification

- 51 worktree/workspace snapshots; 17,420 selected files
  (4,130,878,545 source bytes, before Git deduplication/compression).
- 64 previously local-only committed tips have
  separate archive refs; their original commit identities are preserved.
- Archive branches use `codex/archive-20260916/`. `wip-` means a preservation
  snapshot. `committed-` preserves an existing local commit, not a new release.
- Every newly captured file was checked against its Git blob and recorded with
  SHA-256. Deletions were checked against each resulting snapshot tree.
- Remote upload progress and verification are recorded in
  `handoff/upload-status.json`. Do not assume an archive branch is available
  until that status records it as uploaded. Completion requires exact remote
  ref checks and six representative file downloads with matching hashes.
- Source worktrees, staged changes, existing branches, and local saves were
  left in place. Default branches were not merged or force-pushed.
- Build output, dependency caches, Python bytecode, local test state, temporary
  commit messages, and generated CI logs were excluded. Per-path exclusions
  are recorded in the inventory. No stashes existed in the inspected clones.
- This is a recoverability/archive check. Gameplay and visual acceptance were
  not rerun or claimed for unfinished snapshots.

`handoff/archive-manifest.json` maps every snapshot to its branch, commit,
original worktree, captured file hashes, and exclusions. Use it to locate
older work without assuming the newest candidate is the best reference.

## Unity setup context

The Windows Unity MCP server and matching Editor package are version 10.2.0.
Unity Hub was downloading Editor 6000.6.0f1 and Web build support when setup
was checked. The Mac installation and Editor connection have not been verified
from this Windows task. Local Codex credentials/configuration and installed
Python environments are not part of this game archive.

The source conversations remain available at:
- https://chatgpt.com/c/6aa8923f-4a7c-83e9-94a2-930660ace467
- https://chatgpt.com/c/6aa9a8d3-449c-83ea-b0e9-65bc6b80507a
