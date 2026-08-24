#!/usr/bin/env python3
"""Verdigris wizard splash asset-pack verifier (TASK-0179).

Deterministically proves native/client/assets/wizard/splash/** is one
coherent, unmodified Verdigris splash/menu art pack:

  * splash_meta.json + adoption_manifest.json parse and agree 1:1,
  * every adopted file exists with the manifest's exact sha256, byte count,
    pixel dimensions, and color mode (byte-identical adoption, no
    re-encode, no stray rasters),
  * required roles are present (background, variant, >=2 alpha layers) and
    the title role is explicitly documented as absent-with-reason,
  * alpha rules hold: background/variant rasters are fully opaque; layer
    rasters carry a real alpha channel (min < 255),
  * composition z-orders are unique integers.

Modes:
  (default)   read-only validation of the committed splash tree
  --corrupt   negative control: copy the splash tree to a temp dir, flip a
              byte of the first adopted file, re-run the same checks, and
              expect them to FAIL. Exit 1 when the control trips (the
              verifier caught the corruption), exit 2 if the corrupted tree
              somehow passes (broken control).

Pillow is used for pixel inspection. Exit codes: 0 pass, 1 validation
failures / expected negative-control failure, 2 usage/IO.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import sys
import tempfile
from pathlib import Path

from PIL import Image

SPLASH_RELPATH = Path("native/client/assets/wizard/splash")
REQUIRED_ROLES = {"background", "variant", "layer"}
MIN_LAYER_COUNT = 2


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def pixel_facts(path: Path) -> dict:
    try:
        with Image.open(path) as im:
            mode = im.mode
            w, h = im.size
            alpha = im.getchannel("A").getextrema() if "A" in mode else None
    except Exception as exc:  # unreadable/corrupt raster is a verification failure
        return {"w": None, "h": None, "mode": None, "alpha": None, "error": str(exc)}
    return {"w": w, "h": h, "mode": mode, "alpha": alpha}


def verify(splash_dir: Path) -> list[str]:
    """Return a list of human-readable failures; empty list means pass."""
    failures: list[str] = []

    meta_path = splash_dir / "splash_meta.json"
    adoption_path = splash_dir / "adoption_manifest.json"
    for label, p in (("splash_meta.json", meta_path), ("adoption_manifest.json", adoption_path)):
        if not p.is_file():
            failures.append(f"missing {label}")
    if failures:
        return failures
    try:
        meta = json.loads(meta_path.read_text(encoding="utf-8"))
        adoption = json.loads(adoption_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, UnicodeDecodeError) as exc:
        return [f"manifest parse failure: {exc}"]

    files = adoption.get("files")
    if not isinstance(files, list) or not files:
        return ["adoption_manifest.json has no files list"]

    composition = meta.get("composition")
    if not isinstance(composition, list) or not composition:
        failures.append("splash_meta.json has no composition list")

    # 1:1 agreement between composition and adoption entries.
    comp_by_file = {}
    if isinstance(composition, list):
        for c in composition:
            comp_by_file[c.get("file")] = c
    adopted_names = [f.get("adopted") for f in files]
    if isinstance(composition, list):
        if sorted(filter(None, comp_by_file)) != sorted(filter(None, adopted_names)):
            failures.append("splash_meta.composition and adoption_manifest.files disagree")

    # Required roles + documented title.
    roles = {f.get("role") for f in files}
    missing_roles = REQUIRED_ROLES - roles
    if missing_roles:
        failures.append(f"missing required roles: {sorted(missing_roles)}")
    layer_count = sum(1 for f in files if f.get("role") == "layer")
    if layer_count < MIN_LAYER_COUNT:
        failures.append(f"expected >= {MIN_LAYER_COUNT} layers, found {layer_count}")
    title = meta.get("title")
    if not isinstance(title, dict) or title.get("role") != "title" or "note" not in title:
        failures.append("splash_meta.title must document the title role (present/note)")

    # Z-order sanity.
    zorders = [c.get("zOrder") for c in composition or []]
    if not all(isinstance(z, int) and not isinstance(z, bool) for z in zorders):
        failures.append("composition zOrder values must be integers")
    elif len(set(zorders)) != len(zorders):
        failures.append("composition zOrder values are not unique")

    # Per-file checks.
    seen: set[str] = set()
    for entry in files:
        rel = entry.get("adopted")
        if not rel or "/" in rel and (".." in rel or rel.startswith("/")):
            failures.append(f"bad adopted path: {rel!r}")
            continue
        if rel in seen:
            failures.append(f"duplicate adopted entry: {rel}")
            continue
        seen.add(rel)
        dst = splash_dir / rel
        if not dst.is_file():
            failures.append(f"missing adopted file: {rel}")
            continue
        digest = sha256_of(dst)
        if digest != entry.get("sha256"):
            failures.append(f"sha256 mismatch: {rel}")
        size = dst.stat().st_size
        if size != entry.get("bytes"):
            failures.append(f"byte-count mismatch: {rel} ({size} != {entry.get('bytes')})")
        dims = entry.get("dimensions") or {}
        facts = pixel_facts(dst)
        if facts.get("error") is not None:
            failures.append(f"unreadable raster: {rel} ({facts['error']})")
            continue
        if (facts["w"], facts["h"]) != (dims.get("w"), dims.get("h")):
            failures.append(
                f"dimensions mismatch: {rel} ({facts['w']}x{facts['h']} != "
                f"{dims.get('w')}x{dims.get('h')})")
        if facts["mode"] != dims.get("mode"):
            failures.append(f"mode mismatch: {rel} ({facts['mode']} != {dims.get('mode')})")

        # Alpha rules by role.
        role = entry.get("role")
        amin = facts["alpha"][0] if facts["alpha"] else None
        if role in ("background", "variant"):
            if amin is not None and amin != 255:
                failures.append(f"{role} must be fully opaque: {rel} (alpha min {amin})")
        elif role == "layer":
            if amin is None:
                failures.append(f"layer must carry an alpha channel: {rel}")
            elif amin >= 255:
                failures.append(f"layer alpha channel is fully opaque (min {amin}): {rel}")

        # Composition entry sha agreement.
        comp = comp_by_file.get(rel)
        if comp is not None and comp.get("sourceSha256") not in (None, digest):
            failures.append(f"splash_meta sourceSha256 disagrees with file: {rel}")

    # No untracked rasters hiding in the tree.
    allowed = set(seen) | {"splash_meta.json", "adoption_manifest.json"}
    for p in splash_dir.rglob("*"):
        if p.is_file():
            rel = p.relative_to(splash_dir).as_posix()
            if rel not in allowed:
                failures.append(f"unexpected file in splash tree: {rel}")

    return failures


def run_corrupt_control(splash_dir: Path) -> int:
    tmp = Path(tempfile.mkdtemp(prefix="t0179-corrupt-"))
    try:
        broken = tmp / "splash"
        shutil.copytree(splash_dir, broken)
        target = broken / "adoption_manifest.json"
        first_rel = json.loads(target.read_text(encoding="utf-8"))["files"][0]["adopted"]
        victim = broken / first_rel
        data = bytearray(victim.read_bytes())
        if not data:
            print(f"CORRUPT-CONTROL-ERROR: {first_rel} is empty", file=sys.stderr)
            return 2
        data[0] ^= 0xFF
        victim.write_bytes(bytes(data))
        failures = verify(broken)
        if failures:
            print(f"negative control: corrupted {first_rel}; verifier FAILED as expected")
            for f in failures[:5]:
                print(f"  - {f}")
            return 1
        print("BROKEN CONTROL: corrupted tree still passes verification", file=sys.stderr)
        return 2
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    repo_root = Path(__file__).resolve().parents[2]
    parser.add_argument(
        "--splash-dir", type=Path, default=repo_root / SPLASH_RELPATH,
        help="splash asset directory (default: <repo>/native/client/assets/wizard/splash)")
    parser.add_argument("--corrupt", action="store_true",
                        help="run the negative control on a corrupted temp copy")
    args = parser.parse_args(argv)

    if not args.splash_dir.is_dir():
        print(f"splash dir not found: {args.splash_dir}", file=sys.stderr)
        return 2
    if args.corrupt:
        return run_corrupt_control(args.splash_dir)

    failures = verify(args.splash_dir)
    if failures:
        for f in failures:
            print(f"FAIL: {f}")
        print(f"verify_wizard_splash_assets: {len(failures)} failure(s)")
        return 1
    count = len(json.loads((args.splash_dir / "adoption_manifest.json")
                           .read_text(encoding="utf-8"))["files"])
    print(f"verify_wizard_splash_assets: OK ({count} assets, roles+alpha+sha256 all pass)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
