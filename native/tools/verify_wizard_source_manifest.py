#!/usr/bin/env python3
"""TASK-0166 verifier: validate the WIZARD source/provenance manifest.

Checks that every artifact named in native/client/assets/wizard/source_manifest.json
  - exists at WIZARD/<sourcePath>,
  - has a matching sha256 and byte size,
  - raster artifacts carry width/height dimensions,
and that every required module family is covered.

Exit codes: 0 = valid; 1 = validation failure (negative control: run with
`--corrupt` to inject a fake hash mismatch and observe exit 1).
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
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
RASTER_EXT = (".png", ".jpg", ".jpeg", ".webp")


def sha256(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-root", default=DEFAULT_SOURCE_ROOT)
    ap.add_argument("--corrupt", action="store_true",
                    help="negative control: tamper one hash so verification must fail")
    args = ap.parse_args()

    if not os.path.isfile(MANIFEST):
        print(f"FAIL: manifest missing: {MANIFEST}")
        return 1
    with open(MANIFEST, encoding="utf-8") as f:
        m = json.load(f)

    errors: list[str] = []
    if not m.get("sourceCommit"):
        errors.append("manifest lacks sourceCommit provenance")

    families = m.get("families", {})
    for fam, min_count in REQUIRED_FAMILIES.items():
        arts = families.get(fam, {}).get("artifacts", [])
        if len(arts) < min_count:
            errors.append(f"family '{fam}' has {len(arts)} artifacts, need >= {min_count}")

    checked = 0
    for fam_name, fam in families.items():
        for art in fam.get("artifacts", []):
            rel = art.get("sourcePath")
            if not rel:
                errors.append(f"[{fam_name}] artifact without sourcePath")
                continue
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

    if errors:
        print(f"VERIFY FAIL ({len(errors)} problems, {checked} artifacts scanned):")
        for e in errors[:40]:
            print("  -", e)
        return 1
    print(f"VERIFY OK: {checked} artifacts across {len(families)} families, "
          f"WIZARD commit {m.get('sourceCommit')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
