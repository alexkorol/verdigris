"""Resolve explicit import order into one verifiable active runtime catalog."""
from pathlib import Path
import hashlib
import json

from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parent
ORDER = ("inventory", "terrain", "props", "gate", "actors", "bestiary", "weapons", "hero-single", "bestiary-singles", "hero-walk")


def main():
    runtime = ROOT / "../../client/assets/raster/runtime"
    entries, limitations = {}, []
    for name in ORDER:
        report_name = name + ".provenance.json"
        report = json.loads((runtime / report_name).read_text(encoding="utf-8"))
        limitations.extend(report.get("known_limitations", []))
        for asset in report["assets"]:
            entries[asset["name"]] = {**asset, "provenance": report_name}
    for entry in entries.values():
        path = runtime / entry["file"]
        if hashlib.sha256(path.read_bytes()).hexdigest() != entry["sha256"]:
            raise ValueError(f"Runtime hash does not match active provenance: {path}")
        with Image.open(path) as art:
            if art.mode != "RGBA" or list(art.size) != entry["canvas"]:
                raise ValueError(f"Runtime format/dimensions do not match: {path}")
            pixels = np.asarray(art)
        if not np.isin(pixels[:, :, 3], [0, 255]).all():
            raise ValueError(f"World pixels have unintended partial alpha: {path}")
        visible = pixels[:, :, :3][pixels[:, :, 3] > 0]
        if len(np.unique(visible, axis=0)) > 32:
            raise ValueError(f"World sprite exceeds the current 32-color limit: {path}")
    result = {"version": 1, "import_order": list(ORDER), "assets": list(entries.values()),
              "known_limitations": limitations,
              "missing_direction_assets": [name for name in ("wight_nw", "artisan_nw") if name not in entries],
              "walk_sequence": {"frames": [name for name in (f"hero_walk{i}_se" for i in range(4)) if name in entries],
                                "pivot_policy": "fixed source origin, shared scale; never fit individual frame bounds"}}
    (runtime / "catalog.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"Verified {len(entries)} active RGBA sprites, native dimensions, binary alpha, <=32 colors, and provenance hashes.")


if __name__ == "__main__":
    main()
