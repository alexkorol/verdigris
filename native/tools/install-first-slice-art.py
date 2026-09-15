"""Install visually accepted clips without resampling, cropping, or renaming frames by guess.

Input JSON: {"accepted": true, "clips": [{"identity": "player_male",
"action": "idle", "direction": "front", "frames": ["relative/frame.png"],
"frame": [96,96], "anchor": [48,80], "pixels_per_metre":48, "fps":8,
"loop":true}]}. Paths resolve beside the input JSON. Each frame is copied byte
for byte. This gate verifies technical facts; the explicit accepted flag records
the preceding visual review, it does not replace it.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path
from PIL import Image


def cohort(identity):
    for family in ("player_male", "player_female"):
        if identity == family or identity.startswith(family + "_"):
            return family
    return identity


def install(source, runtime):
    data = json.loads(source.read_text(encoding="utf-8"))
    if data.get("accepted") is not True:
        raise ValueError("Only explicitly visually accepted art can enter runtime")
    rows, images, seen = {}, {}, set()
    for clip in data["clips"]:
        identity, action, direction = (clip[k] for k in ("identity", "action", "direction"))
        if any(not re.fullmatch(r"[a-z0-9_-]+", v) for v in (identity, action, direction)):
            raise ValueError("Unsafe clip identifier")
        key = (identity, action, direction)
        if key in seen:
            raise ValueError(f"Duplicate clip: {key}")
        seen.add(key)
        width, height = clip["frame"]
        ax, ay = clip["anchor"]
        ppm, fps = clip["pixels_per_metre"], clip["fps"]
        if not (0 < width <= 1024 and 0 < height <= 1024 and 0 <= ax <= width and 0 <= ay <= height and ppm == 48 and 0 < fps <= 60):
            raise ValueError(f"Invalid geometry or cadence: {key}")
        if not isinstance(clip["loop"], bool) or not 1 <= len(clip["frames"]) <= 64:
            raise ValueError(f"Invalid cycle: {key}")
        names = []
        for index, filename in enumerate(clip["frames"]):
            path = (source.parent / filename).resolve()
            with Image.open(path) as image:
                if image.size != (width, height) or image.mode != "RGBA":
                    raise ValueError(f"Incorrect dimensions or missing RGBA: {path}")
                alpha = image.getchannel("A")
                if alpha.getextrema() != (0, 255) or set(alpha.getdata()) != {0, 255}:
                    raise ValueError(f"Expected reconstructed binary true alpha: {path}")
                if not alpha.getbbox():
                    raise ValueError(f"Empty frame: {path}")
            blob = path.read_bytes()
            name = "fs_" + "_".join(key) + f"_{index:02d}_" + hashlib.sha256(blob).hexdigest()[:10]
            names.append(name)
            images[name] = blob
        rows[key] = "\t".join(map(str, [*key, fps, int(clip["loop"]), width, height, ax, ay, ppm, *names]))
    manifest = runtime / "manifest.tsv"
    if manifest.exists():
        replaced_identities = {cohort(key[0]) for key in rows}
        for line in manifest.read_text(encoding="utf-8").splitlines():
            if line and not line.startswith("#"):
                key = tuple(line.split()[:3])
                # An appearance update replaces the entire character cohort.
                # Keeping old actions while importing one new clip would stitch
                # two different character sheets together again.
                if cohort(key[0]) not in replaced_identities:
                    rows[key] = line
    runtime.mkdir(parents=True, exist_ok=True)
    for name, blob in images.items():
        (runtime / (name + ".png")).write_bytes(blob)
    temporary = runtime / "manifest.tsv.new"
    temporary.write_text("# identity action direction fps loop width height anchor_x anchor_y pixels_per_metre frames...\n" + "\n".join(rows[k] for k in sorted(rows)) + "\n", encoding="utf-8")
    temporary.replace(manifest)
    print(json.dumps({"installed_frames":len(images), "manifest_clips":len(rows), "runtime":str(runtime)}))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--runtime", type=Path, default=Path(__file__).resolve().parents[1] / "client/assets/first-slice/runtime")
    args = parser.parse_args()
    install(args.manifest.resolve(), args.runtime.resolve())
