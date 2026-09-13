#!/usr/bin/env python3
"""TASK-0169 verifier for native WIZARD RPG inventory item-art pack."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
PACK_DIR = os.path.join(REPO, "native", "client", "assets", "wizard", "items")
MANIFEST = os.path.join(PACK_DIR, "manifest.json")
ID_PATTERN = re.compile(r"^[a-z][a-z0-9]*(_[a-z0-9]+)*$")
REQUIRED_CATEGORIES = frozenset(
    {"weapon", "armor", "tool", "reagent", "trophy", "recoverable"}
)
MIN_ITEMS = 12


def sha256(path: str) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


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
    items = manifest.get("items")
    if not isinstance(items, list) or len(items) < MIN_ITEMS:
        diags.append(f"need at least {MIN_ITEMS} items")

    categories = set()
    ids = set()
    checked = 0
    for index, item in enumerate(items or []):
        if not isinstance(item, dict):
            diags.append(f"items[{index}] not object")
            continue
        item_id = item.get("id")
        if not isinstance(item_id, str) or not ID_PATTERN.match(item_id):
            diags.append(f"items[{index}] bad id")
        elif item_id in ids:
            diags.append(f"duplicate id {item_id}")
        else:
            ids.add(item_id)
        category = item.get("category")
        if category:
            categories.add(category)
        rel = item.get("path")
        if not rel:
            diags.append(f"items[{index}] missing path")
            continue
        path = os.path.join(PACK_DIR, rel.replace("/", os.sep))
        if not os.path.isfile(path):
            diags.append(f"missing asset: {rel}")
            continue
        digest = sha256(path)
        want = item.get("sha256", "")
        if args.corrupt and checked == 0:
            want = "0" * 64
        if digest != want:
            diags.append(f"hash mismatch: {rel}")
        if item.get("bytes") != os.path.getsize(path):
            diags.append(f"size mismatch: {rel}")
        footprint = item.get("footprint")
        if not isinstance(footprint, dict) or not footprint.get("w") or not footprint.get("h"):
            diags.append(f"items[{index}] footprint required")
        checked += 1

    for cat in REQUIRED_CATEGORIES:
        if cat not in categories:
            diags.append(f"missing category {cat}")

    contact = manifest.get("contact_sheet")
    if not contact:
        diags.append("contact_sheet required")
    else:
        contact_path = os.path.join(PACK_DIR, contact.replace("/", os.sep))
        if not os.path.isfile(contact_path):
            diags.append(f"missing contact sheet: {contact}")

    if diags:
        print(f"VERIFY FAIL ({len(diags)} problems, {checked} items checked):")
        for item in diags[:20]:
            print("  -", item)
        return 1

    print(f"VERIFY OK: {checked} items, categories={sorted(categories)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
