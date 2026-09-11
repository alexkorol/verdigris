"""Resolve explicit import order into one verifiable active runtime catalog."""
from pathlib import Path
import hashlib
import json
import re

from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parent
ORDER = ("inventory", "terrain", "props", "gate", "actors", "bestiary", "weapons", "hero-single", "bestiary-singles", "hero-walk", "hero-directions", "environment-singles", "large-props", "hero-strike-se", "hit-spark", "hero-walk-sw", "hero-walk-nw", "hero-walk-ne", "hero-strike-nw", "hero-strike-sw", "terrain-quiet", "raider-walk-sw", "hero-strike-ne", "raider-strike-sw", "ground-dust", "slash-trail", "raider-death-sw", "raider-walk-ne", "raider-walk-nw", "raider-walk-se", "wight-walk-sw", "terrain-quiet-stone", "wall-stone-cutaway")


def collect_cycles(entries, action, family="hero"):
    cycles = {}
    directions=("n","ne","e","se","s","sw","w","nw") if family in ("hero_male","hero_female") else ("se", "sw", "nw", "ne")
    for direction in directions:
        numbered = sorted((int(match.group(1)), name) for name in entries
                          if (match := re.fullmatch(rf"{family}_{action}(\d+)_{direction}", name)))
        if not numbered:
            continue
        if [number for number, _ in numbered] != list(range(len(numbered))):
            raise ValueError(f"{action} cycle {direction} has a missing frame")
        frames = [name for _, name in numbered]
        if len({tuple(entries[name]["canvas"]) for name in frames}) != 1:
            raise ValueError(f"{action} cycle {direction} has inconsistent canvases")
        if len({entries[name]["cell_size"] for name in frames}) != 1:
            groups = {entries[name].get("native_scale_group") for name in frames}
            if len(groups) != 1 or None in groups:
                raise ValueError(f"{action} cycle {direction} has unreviewed mixed source resolutions")
        if any(entries[name]["before_resize"] != entries[name]["placed_size"] for name in frames):
            raise ValueError(f"{action} cycle {direction} independently resizes poses")
        cycles[direction] = {"frames": frames, "frame_count": len(frames),
            "canvas": entries[frames[0]]["canvas"],
            "pivot_policy": "fixed source origin, shared scale; never fit individual frame bounds"}
    return cycles


def main():
    runtime = ROOT / "../../client/assets/raster/runtime"
    entries, limitations = {}, []
    for name in (*ORDER,"hero-lineage-20260910"):
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
        color_limit=96 if entry["provenance"]=="hero-lineage-20260910.provenance.json" else 32
        if len(np.unique(visible, axis=0)) > color_limit:
            raise ValueError(f"World sprite exceeds its {color_limit}-color limit: {path}")
    cycles = collect_cycles(entries, "walk")
    deaths = collect_cycles(entries, "death", "raider")
    for direction, sequence in deaths.items():
        pivots = {tuple(entries[name]["anchor_px"]) for name in sequence["frames"]}
        if len(pivots) != 1:
            raise ValueError(f"raider death cycle {direction} has inconsistent pivots")
        sequence["pivot_px"] = list(pivots.pop())
        sequence["playback"] = "one-shot; retain the final settled frame"
    if len(cycles) > 1:
        limitations = [note for note in limitations if note != "Only the SE walk direction has distinct walking frames."]
    lineage={family:{action:collect_cycles(entries,action,family) for action in ("walk","strike")}
             for family in ("hero_male","hero_female")}
    for family,actions in lineage.items():
        for action,clips in actions.items():
            if len(clips)!=8 or any(clip["frame_count"]!=(2 if action=="walk" else 3) for clip in clips.values()):
                raise ValueError(f"Incomplete eight-direction lineage cycle: {family} {action}")
    result = {"version": 1, "import_order": [*ORDER,"hero-lineage-20260910"], "assets": list(entries.values()),
              "lineage_sequences":lineage,
              "known_limitations": limitations,
              "missing_direction_assets": [name for name in ("wight_nw", "artisan_nw") if name not in entries],
              "walk_sequence": {"frames": [name for name in (f"hero_walk{i}_se" for i in range(4)) if name in entries],
                                "pivot_policy": "fixed source origin, shared scale; never fit individual frame bounds"},
              "walk_sequences": cycles,
              "strike_sequences": collect_cycles(entries, "strike"),
              "monster_walk_sequences": {"raider": collect_cycles(entries, "walk", "raider"),
                                         "wight": collect_cycles(entries, "walk", "wight")},
              "monster_strike_sequences": {"raider": collect_cycles(entries, "strike", "raider")},
              "monster_death_sequences": {"raider": deaths},
              "missing_walk_directions": [direction for direction in ("se", "sw", "nw", "ne") if direction not in cycles]}
    (runtime / "catalog.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"Verified {len(entries)} active RGBA sprites, native dimensions, binary alpha, scoped palette limits, complete lineage cycles, and provenance hashes.")


if __name__ == "__main__":
    main()
