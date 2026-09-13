#!/usr/bin/env python3
"""TASK-0168 verifier for native WIZARD orb asset pack."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
PACK_DIR = os.path.join(REPO, "native", "client", "assets", "wizard", "orbs")
MANIFEST = os.path.join(PACK_DIR, "manifest.json")
REQUIRED_LAYERS = frozenset(
    {"color_plate", "orb_mask", "normal_map", "depth_ao_pack", "reserved_stone", "empty_glass"}
)
REQUIRED_STATE_LABELS = frozenset({"full", "low", "reserved", "empty"})


def sha256(path: str) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


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
    if manifest.get("pack") != "wizard_orbs":
        diags.append("unexpected pack id")

    layer_roles = set()
    state_labels = set()
    checked = 0
    for entry in manifest.get("layers", []):
        layer_roles.add(entry.get("role"))
        checked = check_entry(diags, PACK_DIR, entry, args.corrupt, checked)
    for entry in manifest.get("states", []):
        state_labels.add(entry.get("label"))
        checked = check_entry(diags, PACK_DIR, entry, args.corrupt, checked)

    for role in REQUIRED_LAYERS:
        if role not in layer_roles:
            diags.append(f"missing layer role {role}")
    for label in REQUIRED_STATE_LABELS:
        if label not in state_labels:
            diags.append(f"missing state label {label}")

    contact = manifest.get("contact_sheet")
    if not contact:
        diags.append("contact_sheet required")
    else:
        contact_path = os.path.join(PACK_DIR, contact.replace("/", os.sep))
        if not os.path.isfile(contact_path):
            diags.append(f"missing contact sheet: {contact}")

    if diags:
        print(f"VERIFY FAIL ({len(diags)} problems, {checked} entries checked):")
        for item in diags[:20]:
            print("  -", item)
        return 1

    print(f"VERIFY OK: {checked} orb entries, states={sorted(state_labels)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
