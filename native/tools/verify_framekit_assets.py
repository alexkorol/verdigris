#!/usr/bin/env python3
"""TASK-0167 verifier: validate the Framekit raster slice pack.

Checks native/client/assets/wizard/framekit against adoption_manifest.json
and nine_slice.json:
  - every artifact exists, is RGBA (alpha channel), meets minimum dimensions,
    and matches its recorded sha256 + byte size,
  - verbatim adoptions match their embedded WIZARD source hash,
  - derived slices carry a sliceRect within the parent texture,
  - no stray PNGs outside the manifest,
  - nine_slice.json schema: panel margins/slice rects within texture bounds,
    referenced files exist, sprite entries resolve.

Exit codes: 0 = valid; 1 = failure.
Negative control: `--corrupt` copies the pack to a temp dir, mutates one
artifact, re-verifies, and exits non-zero (the corrupted copy must fail).
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import sys
import tempfile

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
PACK_DIR = os.path.join(REPO, "native", "client", "assets", "wizard", "framekit")
MIN_DIM = 8


def sha256_file(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def verify_pack(pack_dir: str) -> list[str]:
    errors: list[str] = []

    manifest_path = os.path.join(pack_dir, "adoption_manifest.json")
    if not os.path.isfile(manifest_path):
        return [f"missing adoption_manifest.json under {pack_dir}"]
    with open(manifest_path, encoding="utf-8") as f:
        manifest = json.load(f)

    if manifest.get("schemaVersion") != 1:
        errors.append("adoption_manifest.json: bad schemaVersion")
    if not manifest.get("sourceCommit"):
        errors.append("adoption_manifest.json: missing sourceCommit provenance")

    seen: set[str] = set()
    for art in manifest.get("artifacts", []):
        name = art.get("file", "<none>")
        if name in seen:
            errors.append(f"{name}: duplicate artifact entry")
            continue
        seen.add(name)
        p = os.path.join(pack_dir, name)
        if not os.path.isfile(p):
            errors.append(f"{name}: missing file")
            continue
        digest = sha256_file(p)
        if digest != art.get("sha256"):
            errors.append(f"{name}: sha256 mismatch")
        if os.path.getsize(p) != art.get("bytes"):
            errors.append(f"{name}: byte size mismatch")
        try:
            img = Image.open(p)
            img.load()
        except OSError as e:
            errors.append(f"{name}: unreadable image ({e})")
            continue
        if img.mode != "RGBA":
            errors.append(f"{name}: mode {img.mode}, expected RGBA alpha channel")
        if img.width != art.get("width") or img.height != art.get("height"):
            errors.append(f"{name}: dimension mismatch vs manifest")
        if min(img.size) < MIN_DIM:
            errors.append(f"{name}: {img.size} below minimum dimension {MIN_DIM}")
        adoption = art.get("adoption")
        if adoption == "verbatim":
            if digest != art.get("sourceSha256"):
                errors.append(f"{name}: verbatim copy diverges from WIZARD source hash")
        elif adoption == "derived-slice":
            rect = art.get("sliceRect")
            if not isinstance(rect, list) or len(rect) != 4:
                errors.append(f"{name}: derived slice without sliceRect")
            else:
                x, y, w, h = rect
                src_w = art.get("parentWidth", art.get("width"))
                src_h = art.get("parentHeight", art.get("height"))
                if x < 0 or y < 0 or w <= 0 or h <= 0 or x + w > src_w or y + h > src_h:
                    errors.append(f"{name}: sliceRect {rect} out of parent bounds")
        else:
            errors.append(f"{name}: unknown adoption kind '{adoption}'")

    on_disk = {n for n in os.listdir(pack_dir) if n.lower().endswith(".png")}
    for stray in sorted(on_disk - seen):
        errors.append(f"stray png not in adoption_manifest.json: {stray}")

    ns_path = os.path.join(pack_dir, "nine_slice.json")
    if not os.path.isfile(ns_path):
        errors.append("missing nine_slice.json")
        return errors
    with open(ns_path, encoding="utf-8") as f:
        ns = json.load(f)
    if ns.get("schemaVersion") != 1:
        errors.append("nine_slice.json: bad schemaVersion")
    if not ns.get("dpiScale"):
        errors.append("nine_slice.json: missing dpiScale")
    files_in_manifest = {a["file"] for a in manifest.get("artifacts", [])}
    for pname, panel in ns.get("panels", {}).items():
        tex = panel.get("texture")
        if tex not in files_in_manifest:
            errors.append(f"panel '{pname}': texture '{tex}' not adopted")
            continue
        w, h = panel.get("width", 0), panel.get("height", 0)
        m = panel.get("margins", {})
        t, r, b, l = (m.get(k) for k in ("top", "right", "bottom", "left"))
        if None in (t, r, b, l) or min(x for x in (t, r, b, l) if x is not None) < 0:
            errors.append(f"panel '{pname}': invalid margins")
            continue
        if t + b >= h or l + r >= w:
            errors.append(f"panel '{pname}': margins {m} leave no center for {w}x{h}")
        tex_img = Image.open(os.path.join(pack_dir, tex))
        if tex_img.size != (w, h):
            errors.append(f"panel '{pname}': texture size {tex_img.size} != declared {w}x{h}")
        for sname, sl in panel.get("slices", {}).items():
            rect = sl.get("rect")
            fname = sl.get("file")
            if fname not in files_in_manifest:
                errors.append(f"panel '{pname}' slice '{sname}': file '{fname}' not adopted")
                continue
            if not isinstance(rect, list) or len(rect) != 4:
                errors.append(f"panel '{pname}' slice '{sname}': bad rect")
                continue
            x, y, rw, rh = rect
            if x < 0 or y < 0 or rw <= 0 or rh <= 0 or x + rw > w or y + rh > h:
                errors.append(f"panel '{pname}' slice '{sname}': rect {rect} out of texture bounds")
                continue
            piece = Image.open(os.path.join(pack_dir, fname))
            if piece.size != (rw, rh):
                errors.append(
                    f"panel '{pname}' slice '{sname}': file {piece.size} != rect {rw}x{rh}")
            anchor, stretch = sl.get("anchor"), sl.get("stretch")
            if not isinstance(anchor, list) or not anchor:
                errors.append(f"panel '{pname}' slice '{sname}': missing anchor")
            if stretch not in ("none", "horizontal", "vertical", "both"):
                errors.append(f"panel '{pname}' slice '{sname}': bad stretch '{stretch}'")
    for spname, sp in ns.get("sprites", {}).items():
        fname = sp.get("file")
        if fname not in files_in_manifest:
            errors.append(f"sprite '{spname}': file '{fname}' not adopted")
            continue
        img = Image.open(os.path.join(pack_dir, fname))
        if (img.width, img.height) != (sp.get("width"), sp.get("height")):
            errors.append(f"sprite '{spname}': size mismatch vs nine_slice.json")
        if not sp.get("anchor"):
            errors.append(f"sprite '{spname}': missing anchor")

    return errors


def run_corrupt_control() -> int:
    tmp = tempfile.mkdtemp(prefix="fk-corrupt-")
    copy_dir = os.path.join(tmp, "framekit")
    shutil.copytree(PACK_DIR, copy_dir)
    victim = "panel_corner_tl.png"
    img = Image.open(os.path.join(copy_dir, victim)).convert("RGBA")
    px = img.load()
    px[0, 0] = (255, 0, 0, 255)
    img.save(os.path.join(copy_dir, victim), "PNG")
    errors = verify_pack(copy_dir)
    shutil.rmtree(tmp, ignore_errors=True)
    if errors:
        print(f"NEGATIVE CONTROL FAIL (as expected): {len(errors)} problems after corruption:")
        for e in errors[:10]:
            print("  -", e)
        return 1
    print("NEGATIVE CONTROL BROKEN: corruption was NOT detected")
    return 1


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--pack-dir", default=PACK_DIR,
                    help="override pack location (used by tests)")
    ap.add_argument("--corrupt", action="store_true",
                    help="negative control: mutate a temp copy so verification fails")
    args = ap.parse_args()

    if args.corrupt:
        return run_corrupt_control()

    errors = verify_pack(args.pack_dir)
    if errors:
        print(f"VERIFY FAIL ({len(errors)} problems):")
        for e in errors[:40]:
            print("  -", e)
        return 1
    n = len(json.load(open(os.path.join(args.pack_dir, "adoption_manifest.json"),
                           encoding="utf-8"))["artifacts"])
    print(f"VERIFY OK: {n} framekit artifacts, hashes/alpha/dimensions/nine-slice schema valid")
    return 0


if __name__ == "__main__":
    sys.exit(main())
