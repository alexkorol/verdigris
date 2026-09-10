"""Reproduce only the reviewed opaque width2 tile with pinned upstream code.

Inputs/code/palette are verified before invoking the unmodified function.
Requires Pillow and numpy from the existing Pixel Respecter environment.
No model calls, importer changes, package installs, or default runtime output.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import types
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()

def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=HERE.parent / "quiet-stone-candidates/v1/terrain_quiet_stone.png")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    record = json.loads((HERE / "derivation.json").read_bytes())
    if args.output.exists():
        raise ValueError("Output exists; select a new review path")
    if sha(args.input) != record["input_raw_sha256"]:
        raise ValueError("Input differs from the reviewed native v1 tile")
    upstream = json.loads((HERE / "vendor/UPSTREAM.json").read_bytes())
    for item in upstream["files"]:
        if sha(HERE / "vendor" / item["file"]) != item["raw_sha256"]:
            raise ValueError("Pinned upstream code or license changed")
    package = types.ModuleType("lib"); package.__path__ = []; sys.modules["lib"] = package
    palettes = load("lib.palettes", HERE / "vendor/palettes.py")
    seams = load("published_seamless", HERE / "vendor/seamless.py")
    source = Image.open(args.input).convert("RGBA")
    native = np.asarray(source)
    if source.size != (64, 64) or not np.all(native[:, :, 3] == 255):
        raise ValueError("Expected opaque64x64 native input")
    actual = ["#%02x%02x%02x" % tuple(int(c) for c in color)
              for color in np.unique(native[:, :, :3].reshape(-1, 3), axis=0)]
    if actual != record["palette_hex"]:
        raise ValueError("The exact23-color v1 palette changed")
    palettes.PALETTES["verdigris_v1_fixed"] = actual
    output = seams.edge_match_blend(source, "verdigris_v1_fixed", blend_width=2)
    pixels = np.asarray(output)
    if hashlib.sha256(pixels.tobytes()).hexdigest() != record["output_rgba_pixels_sha256"]:
        raise ValueError("Pixel result differs from the reviewed derivation")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    output.save(args.output, format="PNG")
    print(json.dumps({"output": str(args.output), "raw_sha256": sha(args.output),
                      "pixels_match_reviewed_derivation": True,
                      "png_bytes_match_reviewed_derivation": sha(args.output) == record["output_raw_sha256"]}))

if __name__ == "__main__":
    main()
