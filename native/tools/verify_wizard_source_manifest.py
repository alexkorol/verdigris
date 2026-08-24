#!/usr/bin/env python3
"""TASK-0166 verifier: validate the WIZARD source/provenance manifest.

Checks that every artifact named in native/client/assets/wizard/source_manifest.json
  - exists at WIZARD/<sourcePath>,
  - has a matching sha256 and byte size,
  - raster artifacts carry width/height dimensions,
  - carries an honest per-entry "tracked" flag: entries marked tracked:true must
    resolve in the pinned sourceCommit's tree of the WIZARD git repo, and entries
    marked tracked:false must NOT (the pin is an immutable commit, so both
    directions are timeless facts),
  - no (family, sourcePath) pair appears twice,
  - the top-level sourceCommitNote's tracked/total counts match the entries,
  - every required raster family meets its minimum artifact count,
  - every required reference family is present and its launch/readme paths exist,
  - rpg_inventory stagingManifests paths exist and their tracked flags are honest.

Exit codes: 0 = valid; 1 = validation failure. Negative controls (each must
exit 1 against a valid manifest):
  --corrupt          inject a fake hash mismatch on the first artifact
  --corrupt-tracked  flip the first tracked:false entry to tracked:true
                     (a false provenance claim the git check must catch)
  --corrupt-dup      duplicate the first artifact entry in memory
                     (the duplicate check must catch it)
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
MANIFEST = os.path.join(REPO, "native", "client", "assets", "wizard", "source_manifest.json")
DEFAULT_SOURCE_ROOT = r"Z:\Code\WIZARD"

REQUIRED_FAMILIES = {
    "framekit": 1,
    "orbs": 6,
    "rpg_inventory": 100,
    "splash": 20,
}
# Reference (code) families: no raster minimum, but they must be present and
# their launch/readme paths must exist under the source root.
REQUIRED_REFERENCE_FAMILIES = (
    "geometric_skilltree",
    "arcane_lattice",
    "cartographer",
    "rp_account_creator",
)
RASTER_EXT = (".png", ".jpg", ".jpeg", ".webp")


def sha256(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def commit_tree_paths(source_root: str, commit: str) -> set[str] | None:
    """Paths tracked at `commit` in the git repo at source_root, or None if the
    repo/commit cannot be read (which the caller must treat as a failure: a
    provenance claim that cannot be checked must not verify OK)."""
    try:
        out = subprocess.run(
            ["git", "-C", source_root, "ls-tree", "-r", "--name-only", "-z", commit],
            capture_output=True, check=True,
        ).stdout
    except (OSError, subprocess.CalledProcessError):
        return None
    return {p.decode("utf-8") for p in out.split(b"\0") if p}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-root", default=DEFAULT_SOURCE_ROOT)
    ap.add_argument("--corrupt", action="store_true",
                    help="negative control: tamper one hash so verification must fail")
    ap.add_argument("--corrupt-tracked", action="store_true",
                    help="negative control: falsely mark an untracked entry tracked:true")
    ap.add_argument("--corrupt-dup", action="store_true",
                    help="negative control: duplicate one artifact entry in memory")
    args = ap.parse_args()

    if not os.path.isfile(MANIFEST):
        print(f"FAIL: manifest missing: {MANIFEST}")
        return 1
    with open(MANIFEST, encoding="utf-8") as f:
        m = json.load(f)

    errors: list[str] = []
    commit = m.get("sourceCommit")
    if not commit:
        errors.append("manifest lacks sourceCommit provenance")

    families = m.get("families", {})

    if args.corrupt_tracked:
        flipped = False
        for fam in families.values():
            for art in fam.get("artifacts", []):
                if art.get("tracked") is False:
                    art["tracked"] = True
                    flipped = True
                    break
            if flipped:
                break
        if not flipped:
            print("corrupt-tracked: no tracked:false entry available to flip")

    if args.corrupt_dup:
        for fam in families.values():
            arts = fam.get("artifacts")
            if arts:
                arts.append(dict(arts[0]))
                break

    for fam, min_count in REQUIRED_FAMILIES.items():
        arts = families.get(fam, {}).get("artifacts", [])
        if len(arts) < min_count:
            errors.append(f"family '{fam}' has {len(arts)} artifacts, need >= {min_count}")

    for fam in REQUIRED_REFERENCE_FAMILIES:
        entry = families.get(fam)
        if entry is None:
            errors.append(f"required reference family missing: {fam}")
            continue
        for key in ("launch", "readme"):
            rel = entry.get(key)
            if not rel:
                errors.append(f"reference family '{fam}' lacks a {key} path")
                continue
            p = os.path.join(args.source_root, rel.replace("/", os.sep))
            if not os.path.isfile(p):
                errors.append(f"reference family '{fam}' {key} path missing: {rel}")

    tree = commit_tree_paths(args.source_root, commit) if commit else None
    if commit and tree is None:
        errors.append(
            f"cannot read commit {commit} from git repo at {args.source_root}: "
            "tracked-provenance claims are unverifiable")

    checked = 0
    n_tracked = 0
    n_untracked = 0
    seen: set[tuple[str, str]] = set()
    for fam_name, fam in families.items():
        for art in fam.get("artifacts", []):
            rel = art.get("sourcePath")
            if not rel:
                errors.append(f"[{fam_name}] artifact without sourcePath")
                continue
            if (fam_name, rel) in seen:
                errors.append(f"duplicate entry: [{fam_name}] {rel}")
                continue
            seen.add((fam_name, rel))
            tracked = art.get("tracked")
            if not isinstance(tracked, bool):
                errors.append(f"[{fam_name}] {rel}: missing boolean 'tracked' flag")
            elif tree is not None:
                if tracked:
                    n_tracked += 1
                    if rel not in tree:
                        errors.append(
                            f"[{fam_name}] {rel}: marked tracked but absent from commit {commit}")
                else:
                    n_untracked += 1
                    if rel in tree:
                        errors.append(
                            f"[{fam_name}] {rel}: marked tracked:false but present in commit {commit}")
            p = os.path.join(args.source_root, rel.replace("/", os.sep))
            if not os.path.isfile(p):
                errors.append(f"missing source file: {rel}")
                continue
            digest = sha256(p)
            want = art.get("sha256", "")
            if args.corrupt and checked == 0:
                want = "0" * 64
            if digest != want:
                errors.append(f"hash mismatch: {rel}")
            if art.get("bytes") != os.path.getsize(p):
                errors.append(f"size mismatch: {rel}")
            ext = os.path.splitext(rel)[1].lower()
            if ext in RASTER_EXT and not art.get("dimensions"):
                errors.append(f"raster without dimensions: {rel}")
            checked += 1

    note = m.get("sourceCommitNote", "")
    if not note:
        errors.append("manifest lacks sourceCommitNote scoping the provenance claim")
    else:
        nums = re.match(r"sourceCommit pin is scoped: (\d+) of (\d+) ", note)
        total = len(seen)
        if not nums:
            errors.append("sourceCommitNote does not state 'N of M' tracked counts")
        elif tree is not None and (int(nums.group(1)), int(nums.group(2))) != (n_tracked, total):
            errors.append(
                f"sourceCommitNote claims {nums.group(1)}/{nums.group(2)} tracked "
                f"but entries say {n_tracked}/{total}")

    staging = families.get("rpg_inventory", {}).get("stagingManifests", [])
    for sm in staging:
        rel = sm.get("path")
        if not rel:
            errors.append("rpg_inventory stagingManifests entry without path")
            continue
        p = os.path.join(args.source_root, rel.replace("/", os.sep))
        if not os.path.isfile(p):
            errors.append(f"staging manifest missing: {rel}")
        tracked = sm.get("tracked")
        if not isinstance(tracked, bool):
            errors.append(f"staging manifest {rel}: missing boolean 'tracked' flag")
        elif tree is not None and tracked != (rel in tree):
            errors.append(f"staging manifest {rel}: tracked flag contradicts commit {commit}")

    if errors:
        print(f"VERIFY FAIL ({len(errors)} problems, {checked} artifacts scanned):")
        for e in errors[:40]:
            print("  -", e)
        return 1
    print(f"VERIFY OK: {checked} artifacts across {len(families)} families "
          f"({n_tracked} tracked at commit, {n_untracked} loose-file sha256-only, "
          f"{len(staging)} staging manifests), WIZARD commit {commit}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
