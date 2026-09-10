"""Compare fresh large-prop reconstructions without replacing runtime PNGs."""
from pathlib import Path
import json

import numpy as np
from PIL import Image, ImageDraw

import import_assets

ROOT = Path(__file__).resolve().parent
RUNTIME = ROOT / "../../client/assets/raster/runtime"
NAMES = ("tree", "hut", "storehut", "column", "gate")


def fit_visible(image: Image.Image, height: int) -> Image.Image:
    image = image.convert("RGBA")
    bounds = image.getchannel("A").getbbox()
    image = image.crop(bounds)
    return image.resize((round(image.width * height / image.height), height), Image.Resampling.NEAREST)


def main():
    experiment = ROOT / "large-props-candidates"
    experiment.mkdir(exist_ok=True)
    props = json.loads((ROOT / "props.json").read_text())
    gate = json.loads((ROOT / "gate.json").read_text())
    # Reconstruct the original settings explicitly. Runtime may already contain
    # an accepted finer candidate, so it cannot serve as the historical baseline.
    baseline_sheets = []
    for original in (props, gate):
        for original_sheet in original["sheets"]:
            sheet = dict(original_sheet)
            sheet["defaults"] = {**original.get("defaults", {}), **sheet.get("defaults", {})}
            sheet["assets"] = [asset for asset in sheet["assets"] if asset["name"] in NAMES]
            if sheet["assets"]:
                baseline_sheets.append(sheet)
    baseline_path = ROOT / "large-props-original.json"
    baseline_path.write_text(json.dumps({"version": 1,
        "output_dir": "large-props-candidates/original", "sheets": baseline_sheets}, indent=2) + "\n")
    import_assets.run(baseline_path, import_assets.DEFAULT_PROJECT)
    reports = {}
    for cell in (3, 2):
        key = f"detail{cell}"
        defaults = {"cell_size": cell, "alpha_mode": "crisp", "max_colors": 32,
                    "canvas": [192, 256], "trim": True, "require_transparency": True,
                    "preserve_scale": True, "acceptance": "experiment_pending_review"}
        sheets = [{"source": props["sheets"][0]["source"], "assets": [
            {k: v for k, v in asset.items() if k != "canvas"}
            for asset in props["sheets"][0]["assets"] if asset["name"] in NAMES]}]
        sheets.append({"source": gate["sheets"][0]["source"], "grid": [1, 1],
                       "assets": [{"name": "gate", "cell": [0, 0], "cell_size": 8}]})
        manifest = {"version": 1, "output_dir": f"large-props-candidates/{key}",
                    "defaults": defaults, "sheets": sheets}
        path = ROOT / f"large-props-{key}.json"
        path.write_text(json.dumps(manifest, indent=2) + "\n")
        reports[key] = import_assets.run(path, import_assets.DEFAULT_PROJECT)
    comparisons = []
    for name in NAMES:
        display_height = 400 if name == "column" else 550
        pane_width, pane_height = 800, 620
        comparison = Image.new("RGB", (pane_width * 3, pane_height), (37, 35, 31))
        old = Image.open(experiment / "original" / (name + ".png")).convert("RGBA")
        old_bounds = old.getchannel("A").getbbox()
        native_heights = {"current": old_bounds[3] - old_bounds[1]}
        for index, key in enumerate(("current", "detail3", "detail2")):
            image = old if key == "current" else Image.open(experiment / key / (name + ".png"))
            bounds = image.getchannel("A").getbbox()
            native_height = bounds[3] - bounds[1]
            native_heights[key] = native_height
            art = fit_visible(image, display_height)
            hero = fit_visible(Image.open(RUNTIME / "hero_se.png"), 240)
            pane = Image.new("RGB", (pane_width, pane_height), (37, 35, 31))
            pane.paste(art, (15, 585 - art.height), art)
            pane.paste(hero, (pane_width - hero.width - 12, 585 - hero.height), hero)
            draw = ImageDraw.Draw(pane)
            draw.text((12, 12), f"{name}: {key}; {native_height} native body px / {display_height}px display", fill=(229, 223, 208))
            draw.text((12, 30), f"Fresh source reconstruction; {display_height/native_height:.2f} display px per native pixel", fill=(190, 182, 164))
            draw.line((0, 586, pane_width, 586), fill=(96, 94, 83))
            pane.save(experiment / f"{name}-{key}-display.png")
            comparison.paste(pane, (index * pane_width, 0))
        comparison.save(experiment / f"{name}-display-comparison.png")
        comparisons.append({"name": name, "display_height": display_height,
                            "native_visible_heights": native_heights,
                            "detail_ratio": {key: round(native_heights[key] / native_heights["current"], 3)
                                             for key in ("detail3", "detail2")}})
    # The original is not a strict integer upscale. Report the engine's own
    # exact-grid check rather than claiming a made-up recovered source pitch.
    from pixel_perfecter import grid
    exact = {}
    for sheet in props["sheets"] + gate["sheets"]:
        with Image.open(ROOT / sheet["source"]) as source:
            for asset in sheet["assets"]:
                if asset["name"] not in NAMES:
                    continue
                box = import_assets.cell_box(sheet, asset, source.size)
                exact[asset["name"]] = grid.detect_nn_scale(np.array(source.crop(box).convert("RGBA")), max_scale=16)
    (experiment / "comparison.json").write_text(json.dumps({
        "runtime_unchanged": True, "baseline": "original props.json and gate.json settings",
        "exact_source_nn_scale": exact,
        "resampling_after_reconstruction": "none", "comparisons": comparisons,
    }, indent=2) + "\n")
    print(json.dumps(comparisons, indent=2))


if __name__ == "__main__":
    main()
