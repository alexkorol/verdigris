#!/usr/bin/env python3
"""TASK-0167 verifier for native WIZARD Framekit raster pack."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
PACK_DIR = os.path.join(REPO, "native", "client", "assets", "wizard", "framekit")
MANIFEST = os.path.join(PACK_DIR, "manifest.json")


def sha256(path: str) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def png_info(path: str) -> tuple[int, int, int]:
    with open(path, "rb") as handle:
        sig = handle.read(8)
        if sig != b"\x89PNG\r\n\x1a\n":
            raise ValueError("not png")
        length = struct.unpack(">I", handle.read(4))[0]
        chunk = handle.read(4)
        if chunk != b"IHDR":
            raise ValueError("missing IHDR")
        data = handle.read(length)
        width, height, bit_depth, color_type = struct.unpack(">IIBB", data[:10])
        return width, height, color_type


def has_alpha(path: str) -> bool:
    width, height, color_type = png_info(path)
    if color_type in (4, 6):
        return True
    if color_type == 3:
        # indexed: scan for tRNS chunk
        with open(path, "rb") as handle:
            handle.seek(8)
            while True:
                header = handle.read(8)
                if len(header) < 8:
                    break
                length, ctype = struct.unpack(">I4s", header)
                if ctype == b"tRNS":
                    return True
                handle.seek(length + 4, os.SEEK_CUR)
    return False


def slice_bounds_ok(width: int, height: int, slice_vals: list[int]) -> bool:
    if len(slice_vals) != 4:
        return False
    top, right, bottom, left = slice_vals
    if min(slice_vals) < 0:
        return False
    if top + bottom >= height or left + right >= width:
        return False
    return True


def check_entry(diags, pack_dir: str, entry: dict, corrupt: bool, checked: int) -> int:
    rel = entry.get("path")
    if not rel:
        diags.append("entry missing path")
        return checked
    path = os.path.join(pack_dir, rel.replace("/", os.sep))
    if not os.path.isfile(path):
        diags.append(f"missing asset: {rel}")
        return checked
    digest = sha256(path)
    want = entry.get("sha256", "")
    if corrupt and checked == 0:
        want = "0" * 64
    if digest != want:
        diags.append(f"hash mismatch: {rel}")
    if entry.get("bytes") != os.path.getsize(path):
        diags.append(f"size mismatch: {rel}")
    dims = entry.get("dimensions", {})
    try:
        width, height, color_type = png_info(path)
        if dims.get("w") != width or dims.get("h") != height:
            diags.append(f"dimension mismatch: {rel}")
        if dims.get("mode") == "RGBA" and color_type not in (4, 6):
            if not has_alpha(path):
                diags.append(f"missing alpha: {rel}")
    except ValueError as exc:
        diags.append(f"png invalid {rel}: {exc}")
    slice_vals = entry.get("slice")
    if slice_vals is not None:
        if not slice_bounds_ok(dims.get("w", width), dims.get("h", height), slice_vals):
            diags.append(f"slice bounds invalid: {rel}")
        sidecar = entry.get("sidecar")
        if sidecar:
            sidecar_path = os.path.join(pack_dir, sidecar.replace("/", os.sep))
            if os.path.isfile(sidecar_path):
                with open(sidecar_path, encoding="utf-8") as handle:
                    meta = json.load(handle)
                if meta.get("slice") != slice_vals:
                    diags.append(f"sidecar slice mismatch: {rel}")
    return checked + 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--corrupt", action="store_true")
    args = parser.parse_args()

    if not os.path.isfile(MANIFEST):
        print(f"FAIL manifest missing: {MANIFEST}")
        return 1

    with open(MANIFEST, encoding="utf-8") as handle:
        manifest = json.load(handle)

    diags: list[str] = []
    if manifest.get("pack") != "framekit":
        diags.append("unexpected pack id")
    if not manifest.get("provenance", {}).get("wizard_commit"):
        diags.append("missing provenance")

    checked = 0
    for entry in manifest.get("slices", []):
        checked = check_entry(diags, PACK_DIR, entry, args.corrupt, checked)
    for entry in manifest.get("sprites", []):
        checked = check_entry(diags, PACK_DIR, entry, args.corrupt, checked)

    contact = manifest.get("contact_sheet")
    if not contact:
        diags.append("contact_sheet required")
    else:
        contact_path = os.path.join(PACK_DIR, contact.replace("/", os.sep))
        if not os.path.isfile(contact_path):
            diags.append(f"missing contact sheet: {contact}")

    if len(manifest.get("slices", [])) < 2:
        diags.append("need panel and slot slices")

    if diags:
        print(f"VERIFY FAIL ({len(diags)} problems, {checked} entries checked):")
        for item in diags[:20]:
            print("  -", item)
        return 1

    print(f"VERIFY OK: {checked} framekit entries, contact_sheet={contact}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
