#!/usr/bin/env python3
"""TASK-0179 verifier for native WIZARD Verdigris splash asset pack."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
PACK_DIR = os.path.join(REPO, "native", "client", "assets", "wizard", "splash")
MANIFEST = os.path.join(PACK_DIR, "manifest.json")
RASTER_EXT = (".png", ".jpg", ".jpeg", ".webp")


def sha256(path: str) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--corrupt", action="store_true", help="negative control")
    args = parser.parse_args()

    if not os.path.isfile(MANIFEST):
        print(f"FAIL manifest missing: {MANIFEST}")
        return 1

    with open(MANIFEST, encoding="utf-8") as handle:
        manifest = json.load(handle)

    errors: list[str] = []
    provenance = manifest.get("provenance", {})
    if not provenance.get("wizard_commit"):
        errors.append("missing wizard_commit provenance")
    if manifest.get("pack") != "verdigris_splash":
        errors.append("unexpected pack id")

    tiers = manifest.get("tiers")
    if not isinstance(tiers, list) or len(tiers) < 2:
        errors.append("need primary and fallback tiers")

    roles = set()
    has_primary = False
    has_fallback = False
    checked = 0
    for index, tier in enumerate(tiers or []):
        if not isinstance(tier, dict):
            errors.append(f"tiers[{index}] not object")
            continue
        tier_id = tier.get("id")
        role = tier.get("role")
        if role:
            roles.add(role)
        if tier_id == "primary":
            has_primary = True
        if tier_id == "fallback":
            has_fallback = True
        rel = tier.get("path")
        if not rel:
            errors.append(f"tier {tier_id} missing path")
            continue
        path = os.path.join(PACK_DIR, rel.replace("/", os.sep))
        if not os.path.isfile(path):
            errors.append(f"missing asset: {rel}")
            continue
        digest = sha256(path)
        want = tier.get("sha256", "")
        if args.corrupt and checked == 0:
            want = "0" * 64
        if digest != want:
            errors.append(f"hash mismatch: {rel}")
        size = os.path.getsize(path)
        if tier.get("bytes") != size:
            errors.append(f"size mismatch: {rel}")
        dims = tier.get("dimensions")
        ext = os.path.splitext(rel)[1].lower()
        if ext in RASTER_EXT and not dims:
            errors.append(f"raster without dimensions: {rel}")
        checked += 1

    if not has_primary or not has_fallback:
        errors.append("primary and fallback tiers required")
    if "background" not in roles:
        errors.append("background role required")

    contact = manifest.get("contact_sheet")
    if not contact:
        errors.append("contact_sheet path required")
    else:
        contact_path = os.path.join(PACK_DIR, contact.replace("/", os.sep))
        if not os.path.isfile(contact_path):
            errors.append(f"missing contact sheet: {contact}")

    composition = manifest.get("composition")
    if not isinstance(composition, dict) or not composition.get("fallback_policy"):
        errors.append("composition metadata incomplete")

    if errors:
        print(f"VERIFY FAIL ({len(errors)} problems, {checked} tiers checked):")
        for item in errors[:20]:
            print("  -", item)
        return 1

    print(
        f"VERIFY OK: {checked} splash tiers, contact_sheet={contact}, "
        f"wizard_commit={provenance.get('wizard_commit')}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
