"""Inspect a complete cycle with fixed origin; never align individual poses."""
from pathlib import Path
import argparse
import hashlib
import json

import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("direction", choices=("se", "sw", "nw", "ne"))
    parser.add_argument("--action", choices=("walk", "strike"), default="walk")
    parser.add_argument("--count", type=int, default=8)
    parser.add_argument("--duration", type=int, default=80, help="Milliseconds per frame in the review GIF")
    parser.add_argument("--input-dir", type=Path, default=ROOT / "../../client/assets/raster/runtime")
    parser.add_argument("--output-dir", type=Path, default=ROOT)
    parser.add_argument("--report", type=Path, help="Defaults to hero-walk-DIRECTION.provenance.json")
    args = parser.parse_args()
    if not 2 <= args.count <= 32 or not 20 <= args.duration <= 1000:
        parser.error("Count must be 2..32 and duration 20..1000ms")
    report_path = args.report or args.input_dir / f"hero-{args.action}-{args.direction}.provenance.json"
    provenance = json.loads(report_path.read_text(encoding="utf-8"))
    records = {record["name"]: record for record in provenance["assets"]}
    names = [f"hero_{args.action}{i}_{args.direction}" for i in range(args.count)]
    frames = []
    for name in names:
        path = args.input_dir / f"{name}.png"
        if hashlib.sha256(path.read_bytes()).hexdigest() != records[name]["sha256"]:
            raise ValueError(f"Unrecorded frame change: {name}")
        frames.append(Image.open(path).convert("RGBA"))
    if len({frame.size for frame in frames}) != 1:
        raise ValueError("Walk frames must share the entire canvas")
    if len({records[name]["cell_size"] for name in names}) != 1 and not provenance.get("native_scale_group"):
        raise ValueError("Mixed source resolutions require an explicit reviewed native_scale_group")
    if any(records[name]["before_resize"] != records[name]["placed_size"] for name in names):
        raise ValueError("Review rejects independent resampling of walking poses")
    width, height = frames[0].size
    scale, footer = 4, 26
    preview_frames, review_frames = [], []
    native = Image.new("RGB", (width * args.count, height + 20), (32, 31, 29))
    strip = Image.new("RGB", (width * scale * args.count, height * scale + footer), (32, 31, 29))
    grid = Image.new("RGB", (width * scale * min(4, args.count),
                             (height * scale + footer) * ((args.count + 3) // 4)), (32, 31, 29))
    arrays = [np.asarray(frame) for frame in frames]
    union = np.unique(np.concatenate([array[:, :, :3][array[:, :, 3] > 0] for array in arrays]), axis=0)
    for index, (name, frame, array) in enumerate(zip(names, frames, arrays)):
        enlarged = frame.resize((width * scale, height * scale), Image.Resampling.NEAREST)
        pane = Image.new("RGB", (width * scale, height * scale + footer), (32, 31, 29))
        pane.paste(enlarged, (0, 0), enlarged)
        draw = ImageDraw.Draw(pane)
        draw.line((0, height * scale, width * scale, height * scale), fill=(93, 103, 92))
        draw.text((6, height * scale + 8), f"{args.direction.upper()} {index} / fixed origin", fill=(230, 225, 208))
        preview_frames.append(pane)
        strip.paste(pane, (index * width * scale, 0))
        grid.paste(pane, ((index % 4) * width * scale, (index // 4) * (height * scale + footer)))
        native.paste(frame, (index * width, 0), frame)
        ImageDraw.Draw(native).text((index * width + 3, height + 5), f"Frame {index}", fill=(230, 225, 208))
        next_array = arrays[(index + 1) % len(arrays)]
        review_frames.append({"name": name, "visible_bounds": frame.getchannel("A").getbbox(),
            "alpha_opaque_pixels": int(np.count_nonzero(array[:, :, 3])),
            "changed_pixels_to_next": int(np.count_nonzero(np.any(array != next_array, axis=2))),
            **{key: records[name][key] for key in ("sha256", "source_box", "cell_size", "anchor_px", "anchor_source") if key in records[name]}})
    args.output_dir.mkdir(parents=True, exist_ok=True)
    prefix = args.output_dir / f"hero-{args.action}-{args.direction}"
    native.save(str(prefix) + "-strip-1x.png")
    strip.save(str(prefix) + "-strip-4x.png")
    grid.save(str(prefix) + "-contact-4x.png")
    # One preview palette across the entire loop avoids GIF-only palette shifts.
    shared = strip.quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    indexed = [frame.quantize(palette=shared, dither=Image.Dither.NONE) for frame in preview_frames]
    indexed[0].save(str(prefix) + "-preview.gif", save_all=True, append_images=indexed[1:],
                    duration=args.duration, loop=0, optimize=False, disposal=2)
    review = {"action": args.action, "direction": args.direction, "frame_count": args.count, "duration_ms": args.duration,
              "canvas": [width, height], "unique_frame_hashes": len({records[name]["sha256"] for name in names}),
              "visible_cycle_colors": len(union), "shared_palette": provenance.get("shared_palette"),
              "native_scale_group": provenance.get("native_scale_group"), "scale_review": provenance.get("scale_review"),
              "geometry": "Complete native canvases at a fixed pivot. No per-frame fitting or realignment.",
              "acceptance": sorted({records[name].get("acceptance", "unreviewed") for name in names}),
              "hand_sockets": "Measured separately by the equipment review lane on these runtime pixels.",
              "frames": review_frames}
    Path(str(prefix) + "-review.json").write_text(json.dumps(review, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {args.count} fixed-origin {args.direction.upper()} frames; {len(union)} cycle colors; {args.duration}ms review GIF.")


if __name__ == "__main__":
    main()
