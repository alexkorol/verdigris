#!/usr/bin/env python3
"""Import generated sheets through the real Pixel Respecter reconstructor.

All geometry is integer-valued. Raster scaling uses nearest-neighbor only;
transparent colors remain unpremultiplied RGBA in the exported PNGs.
"""
from __future__ import annotations

import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageOps


DEFAULT_PROJECT = Path(r"Z:\Code\Python\pixel-perfecter")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_engine(root: Path):
    root = root.resolve()
    expected = root / "pixel_perfecter" / "workspace.py"
    if not expected.is_file():
        raise ValueError(f"Pixel Respecter workspace API not found: {expected}")
    sys.path.insert(0, str(root))
    workspace = importlib.import_module("pixel_perfecter.workspace")
    if Path(workspace.__file__).resolve() != expected:
        raise ValueError("A different pixel_perfecter is already imported; use a fresh Python process.")
    return workspace


def engine_provenance(root: Path) -> dict:
    def git(*args):
        result = subprocess.run(["git", "-C", str(root), *args], capture_output=True, text=True, check=False)
        return result.stdout.strip() if result.returncode == 0 else None
    import cv2
    import PIL
    # Git HEAD alone cannot identify the owner's current, uncommitted desktop API.
    return {
        "project": "Pixel Respecter", "api": "pixel_perfecter.workspace.reconstruct",
        "root": root.resolve().as_posix(), "git_head": git("rev-parse", "HEAD"),
        "git_dirty": bool(git("status", "--porcelain")),
        "source_sha256": {p.relative_to(root).as_posix(): digest(p)
                          for p in sorted((root / "pixel_perfecter").glob("*.py"))},
        "dependencies": {"python": sys.version.split()[0], "numpy": np.__version__,
                         "Pillow": PIL.__version__, "opencv": cv2.__version__},
    }


def pair(value, label: str, minimum: int = 1) -> tuple[int, int]:
    if not isinstance(value, (list, tuple)) or len(value) != 2:
        raise ValueError(f"{label} must contain two integers")
    if any(type(v) is not int or v < minimum for v in value):
        raise ValueError(f"{label} values must be integers >= {minimum}")
    return tuple(value)


def cell_box(sheet: dict, asset: dict, size: tuple[int, int]) -> tuple[int, int, int, int]:
    if "source_box" in asset:
        box = asset["source_box"]
        if len(box) != 4 or any(type(v) is not int for v in box):
            raise ValueError("source_box must be integer [left, top, right, bottom]")
        left, top, right, bottom = box
        if not (0 <= left < right <= size[0] and 0 <= top < bottom <= size[1]):
            raise ValueError(f"source_box outside sheet: {box}")
        return tuple(box)
    columns, rows = pair(sheet["grid"], "grid")
    column, row = pair(asset["cell"], "cell", minimum=0)
    if column >= columns or row >= rows:
        raise ValueError(f"Cell {asset['cell']} outside {columns}x{rows} sheet")
    margin_x, margin_y = pair(sheet.get("margin", [0, 0]), "margin", minimum=0)
    gutter_x, gutter_y = pair(sheet.get("gutter", [0, 0]), "gutter", minimum=0)
    available_x = size[0] - 2 * margin_x - (columns - 1) * gutter_x
    available_y = size[1] - 2 * margin_y - (rows - 1) * gutter_y
    if available_x <= 0 or available_y <= 0 or available_x % columns or available_y % rows:
        raise ValueError(f"Sheet {size} is not divisible by grid; specify margins/gutters or source_box")
    width, height = available_x // columns, available_y // rows
    left, top = margin_x + column * (width + gutter_x), margin_y + row * (height + gutter_y)
    return left, top, left + width, top + height


def remove_background(source: np.ndarray, options: dict) -> tuple[np.ndarray, dict]:
    """Apply the owner's connected fill to the border and reviewed local gaps.

    A local window is an explicit source-space correction, never an automatic
    global deletion of a foreground color. Enclosed same-color detail outside
    those windows remains intact.
    """
    from pixel_perfecter.colors import make_border_connected_transparent
    result, chosen = make_border_connected_transparent(
        source, tolerance=options.get("tolerance"), background=options.get("color"))
    windows = []
    for box in options.get("interior_windows", []):
        left, top, right, bottom = cell_box({}, {"source_box": box}, (source.shape[1], source.shape[0]))
        region, _ = make_border_connected_transparent(
            result[top:bottom, left:right], tolerance=options.get("tolerance"), background=chosen)
        removed = int(np.count_nonzero((result[top:bottom, left:right, 3] > 0) & (region[:, :, 3] == 0)))
        result[top:bottom, left:right] = region
        windows.append({"source_box": box, "new_transparent_pixels": removed})
    return result, {
        "api": "pixel_perfecter.colors.make_border_connected_transparent",
        "color": list(chosen), "tolerance": options.get("tolerance"),
        "interior_windows": windows,
        "transparent_pixels_after": int(np.count_nonzero(result[:, :, 3] == 0)),
    }


def normalize(art: Image.Image, options: dict) -> tuple[Image.Image, dict]:
    art = art.convert("RGBA")
    pixels = np.asarray(art).copy()
    floor = options.get("alpha_floor", 1)
    if type(floor) is not int or not 1 <= floor <= 255:
        raise ValueError("alpha_floor must be an integer from 1 to 255")
    pixels[pixels[:, :, 3] < floor] = 0
    art = Image.fromarray(pixels)
    bounds = art.getchannel("A").getbbox()
    if bounds is None:
        raise ValueError("Reconstruction contains no visible pixels")
    if options.get("require_transparency", False) and int(pixels[:, :, 3].min()) == 255:
        raise ValueError("Transparent sprite has fully opaque alpha; obtain a genuine-alpha source")
    trim = options.get("trim", True)
    original_size = art.size
    if trim:
        art = art.crop(bounds)
    else:
        bounds = (0, 0, *art.size)
    canvas = pair(options.get("canvas", art.size), "canvas")
    padding = options.get("padding", 0)
    if type(padding) is not int or padding < 0 or 2 * padding >= min(canvas):
        raise ValueError("padding must be a nonnegative integer smaller than half the canvas")
    fit = pair(options.get("fit", [canvas[0] - 2 * padding, canvas[1] - 2 * padding]), "fit")
    if fit[0] > canvas[0] - 2 * padding or fit[1] > canvas[1] - 2 * padding:
        raise ValueError("fit exceeds canvas padding")
    if options.get("preserve_scale", False):
        # Animation frames share their authored grid scale, including crouches.
        target = art.size
        if art.width > fit[0] or art.height > fit[1]:
            raise ValueError(f"Sprite {art.size} exceeds fit {fit}; enlarge its canvas or choose a shared grid pitch")
    elif options.get("fill", False):
        # Terrain sheets explicitly request square tile normalization.
        target = fit
    else:
        ratio = min(fit[0] / art.width, fit[1] / art.height)
        if not options.get("allow_upscale", False):
            ratio = min(1.0, ratio)
        target = max(1, round(art.width * ratio)), max(1, round(art.height * ratio))
    before_resize = art.size
    if target != art.size:
        art = art.resize(target, Image.Resampling.NEAREST)
    alignment = options.get("align", "bottom_center")
    if alignment not in ("bottom_center", "center", "top_left"):
        raise ValueError("align must be bottom_center, center, or top_left")
    x = padding if alignment == "top_left" else (canvas[0] - art.width) // 2
    y = padding if alignment == "top_left" else ((canvas[1] - art.height) // 2
         if alignment == "center" else canvas[1] - padding - art.height)
    if "anchor_reconstructed" in options:
        anchor = pair(options["anchor_reconstructed"], "anchor_reconstructed", minimum=0)
        x = canvas[0] // 2 - round((anchor[0] - bounds[0]) * art.width / before_resize[0])
        y = canvas[1] - padding - round((anchor[1] - bounds[1]) * art.height / before_resize[1])
        if x < 0 or y < 0 or x + art.width > canvas[0] or y + art.height > canvas[1]:
            raise ValueError("Anchored sprite exceeds canvas; choose a larger shared canvas")
    result = Image.new("RGBA", canvas, (0, 0, 0, 0))
    # No mask here: using the source itself as a mask would multiply alpha twice.
    result.paste(art, (x, y))
    return result, {
        "reconstructed_size": list(original_size), "trim_box": list(bounds),
        "before_resize": list(before_resize), "placed_size": list(art.size),
        "canvas": list(canvas), "content_bounds": [x, y, x + art.width, y + art.height],
        "anchor_px": [canvas[0] // 2, canvas[1] - padding],
        "baseline_offset_px": padding, "resampling": "nearest_neighbor",
    }


def contact_sheet(assets: list[tuple[str, Image.Image]], path: Path, scale: int = 1):
    if type(scale) is not int or not 1 <= scale <= 8:
        raise ValueError("Contact-sheet scale must be an integer from 1 to 8")
    assets = [(name, im.resize((im.width * scale, im.height * scale), Image.Resampling.NEAREST))
              for name, im in assets]
    columns = min(4, len(assets))
    cell_w = max(im.width for _, im in assets) + 20
    cell_h = max(im.height for _, im in assets) + 34
    preview = Image.new("RGB", (columns * cell_w, ((len(assets) + columns - 1) // columns) * cell_h))
    draw = ImageDraw.Draw(preview)
    for y in range(0, preview.height, 8):
        for x in range(0, preview.width, 8):
            shade = 34 if (x // 8 + y // 8) % 2 else 42
            draw.rectangle((x, y, x + 7, y + 7), fill=(shade, shade, shade))
    for index, (name, art) in enumerate(assets):
        x, y = (index % columns) * cell_w, (index // columns) * cell_h
        preview.paste(art, (x + (cell_w - art.width) // 2, y + 8), art)
        draw.text((x + 5, y + cell_h - 20), name, fill=(230, 226, 205))
    path.parent.mkdir(parents=True, exist_ok=True)
    preview.save(path)


def reduce_cycle_colors(images: list[Image.Image], maximum: int) -> tuple[list[Image.Image], dict]:
    """Use the owner's palette reducer once across the complete cycle."""
    if type(maximum) is not int or not 1 <= maximum <= 256:
        raise ValueError("shared_palette_max_colors must be an integer from 1 to 256")
    from pixel_perfecter import palettes
    arrays = [np.asarray(image.convert("RGBA")) for image in images]
    combined = np.concatenate([array.reshape(-1, 4) for array in arrays], axis=0).reshape(1, -1, 4)
    before = np.unique(combined[:, :, :3][combined[:, :, 3] > 0], axis=0)
    reduced = palettes.reduce_colors(combined, maximum).reshape(-1, 4)
    reduced[reduced[:, 3] == 0, :3] = 0
    result, offset = [], 0
    for original in arrays:
        count = original.shape[0] * original.shape[1]
        result.append(Image.fromarray(reduced[offset:offset + count].reshape(original.shape)))
        offset += count
    palette = np.unique(reduced[reduced[:, 3] > 0, :3], axis=0)
    return result, {"api": "pixel_perfecter.palettes.reduce_colors", "max_colors": maximum,
                    "visible_colors_before": len(before), "visible_colors_after": len(palette),
                    "palette_rgb": palette.tolist(), "dithering": "none",
                    "scope": "all reconstructed frames in this manifest"}


def run(manifest_path: Path, project: Path, output_override: Path | None = None,
        preview_path: Path | None = None, preview_scale: int = 1) -> dict:
    manifest_path = manifest_path.resolve()
    manifest = json.loads(manifest_path.read_text(encoding="utf-8-sig"))
    if manifest.get("version") != 1:
        raise ValueError("Manifest version must be 1")
    report_name = manifest.get("report_name", manifest_path.stem + ".provenance.json")
    if Path(report_name).name != report_name or not report_name.endswith(".json"):
        raise ValueError("report_name must be a simple .json filename")
    output = (output_override or manifest_path.parent / manifest["output_dir"]).resolve()
    workspace = load_engine(project)
    shared_maximum = manifest.get("shared_palette_max_colors")
    assets, records, sources, names = [], [], [], set()
    # Reconstruct and validate everything before writing any runtime asset.
    for sheet in manifest["sheets"]:
        path = (manifest_path.parent / sheet["source"]).resolve()
        with Image.open(path) as source:
            source = ImageOps.exif_transpose(source).convert("RGBA")
        source_record = {"source": sheet["source"], "sha256": digest(path), "size": list(source.size)}
        if "border_background" in sheet:
            processed, background_record = remove_background(np.asarray(source), sheet["border_background"])
            source = Image.fromarray(processed)
            source_record["background_removal"] = background_record
        sources.append(source_record)
        for asset in sheet["assets"]:
            name = asset["name"]
            if not isinstance(name, str) or not re.fullmatch(r"[a-zA-Z0-9_-]+", name) or name in names:
                raise ValueError(f"Asset names must be unique safe filename stems: {name}")
            names.add(name)
            options = {**manifest.get("defaults", {}), **sheet.get("defaults", {}), **asset}
            box = cell_box(sheet, asset, source.size)
            crop = np.asarray(source.crop(box)).copy()
            cell_size = options.get("cell_size", 0)
            if type(cell_size) is not int or cell_size < 0:
                raise ValueError(f"{name}: cell_size must be a nonnegative integer; 0 means detected")
            offset = pair(options.get("grid_offset", [0, 0]), "grid_offset", minimum=0)
            result = workspace.reconstruct(crop, workspace.Options(
                cell_size=cell_size, offset_x=offset[0], offset_y=offset[1],
                alpha_mode=options.get("alpha_mode", "preserve"),
                palette=options.get("palette", "none"),
                max_colors=0 if shared_maximum is not None else options.get("max_colors", 0)))
            if "anchor_source" in options:
                anchor = pair(options["anchor_source"], "anchor_source", minimum=0)
                if result.grid_kind not in ("manual", "rigid"):
                    raise ValueError("anchor_source requires a rigid grid")
                options["anchor_reconstructed"] = [round((anchor[i] - result.offset[i]) / result.cell_size)
                                                     for i in range(2)]
            elif "anchor_x_source" in options:
                # Keep body x-position stable while placing each frame's feet
                # on the ground; useful when generated row baselines vary.
                visible_rows = np.flatnonzero(np.any(result.image[:, :, 3] > 0, axis=1))
                options["anchor_reconstructed"] = [
                    round((options["anchor_x_source"] - result.offset[0]) / result.cell_size),
                    int(visible_rows[-1]) + 1 if len(visible_rows) else 0]
            normalized, geometry = normalize(Image.fromarray(result.image), options)
            alpha = np.asarray(normalized)[:, :, 3]
            visible_rgb = np.asarray(normalized)[:, :, :3][alpha > 0]
            assets.append((name, normalized))
            records.append({
                "name": name, "file": f"{name}.png", "source": sheet["source"],
                "acceptance": options.get("acceptance", "preview_inspected_pending_live_game"),
                "source_box": list(box), "cell_size": result.cell_size,
                "grid_offset": list(result.offset), "grid_kind": result.grid_kind,
                "alpha_mode": options.get("alpha_mode", "preserve"),
                "palette": options.get("palette", "none"), "max_colors": options.get("max_colors", 0),
                "visible_colors": int(len(np.unique(visible_rgb, axis=0))),
                "alpha_range": [int(alpha.min()), int(alpha.max())],
                "alpha_partial_pixels": int(np.count_nonzero((alpha > 0) & (alpha < 255))),
                "warnings": result.warnings, **geometry,
            })
            if "anchor_source" in options:
                records[-1]["anchor_source"] = options["anchor_source"]
            if "native_scale_group" in manifest:
                records[-1]["native_scale_group"] = manifest["native_scale_group"]
            print(f"{name}: source {box[2] - box[0]}x{box[3] - box[1]} / cell {result.cell_size}"
                  f" -> {geometry['reconstructed_size']} -> {normalized.width}x{normalized.height}")
    if not assets:
        raise ValueError("Manifest has no assets")
    shared_palette = None
    if shared_maximum is not None:
        images, shared_palette = reduce_cycle_colors([art for _, art in assets], shared_maximum)
        assets = [(name, art) for (name, _), art in zip(assets, images)]
        for (_, art), record in zip(assets, records):
            pixels = np.asarray(art)
            record["max_colors"] = shared_maximum
            record["palette_scope"] = "manifest_cycle"
            record["visible_colors"] = len(np.unique(pixels[:, :, :3][pixels[:, :, 3] > 0], axis=0))
    output.mkdir(parents=True, exist_ok=True)
    for (name, art), record in zip(assets, records):
        target = output / f"{name}.png"
        art.save(target, format="PNG", optimize=False)
        record["sha256"] = digest(target)
    report = {"version": 1, "manifest_sha256": digest(manifest_path),
              "importer_sha256": digest(Path(__file__)), "engine": engine_provenance(project.resolve()),
              "known_limitations": manifest.get("known_limitations", []),
              "sources": sources, "assets": records}
    if shared_palette is not None:
        report["shared_palette"] = shared_palette
    if "native_scale_group" in manifest:
        report["native_scale_group"] = manifest["native_scale_group"]
        report["scale_review"] = manifest.get("scale_review", "Pending native head/torso comparison")
    (output / report_name).write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if preview_path:
        contact_sheet(assets, preview_path, preview_scale)
    print(f"Imported {len(assets)} assets into {output}")
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--project", type=Path, default=Path(os.environ.get("PIXEL_RESPECTER_ROOT", DEFAULT_PROJECT)))
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--contact-sheet", type=Path, help="Optional 1:1 diagnostic preview on a dark checkerboard")
    parser.add_argument("--preview-scale", type=int, choices=range(1, 9), default=1,
                        help="Integer nearest-neighbor scale for diagnostic previews only")
    args = parser.parse_args()
    try:
        run(args.manifest, args.project, args.output_dir, args.contact_sheet, args.preview_scale)
    except (ValueError, OSError, KeyError) as exc:
        print(f"Import failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
