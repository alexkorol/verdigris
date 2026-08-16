"""Reject denied legacy identifiers in new native production sources."""

from __future__ import annotations

import json
import pathlib
import sys


ROOT = pathlib.Path(__file__).resolve().parents[2]
CONFIG = ROOT / "config" / "legacy-denylist.json"
NATIVE = ROOT / "native"
EXCLUDED = {"tests", "tools"}
EXTENSIONS = {".cpp", ".hpp", ".h", ".cc", ".cxx"}


def main() -> int:
    denylist = json.loads(CONFIG.read_text(encoding="utf-8"))
    needles = [value.casefold() for value in denylist["identifiers"]]
    failures: list[str] = []
    for path in NATIVE.rglob("*"):
        if path.suffix.casefold() not in EXTENSIONS:
            continue
        if any(part in EXCLUDED for part in path.relative_to(NATIVE).parts):
            continue
        text = path.read_text(encoding="utf-8", errors="replace").casefold()
        for needle in needles:
            if needle in text:
                failures.append(f"{path.relative_to(ROOT)} contains denied identifier {needle!r}")
    if failures:
        print("\n".join(failures))
        return 1
    print("native legacy denylist: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
