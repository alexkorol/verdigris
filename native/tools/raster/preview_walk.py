"""Inspect the four SE walk frames at a fixed integer scale and origin."""
from pathlib import Path
import json

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent
RUNTIME = ROOT / "../../client/assets/raster/runtime"


def main():
    names = [f"hero_walk{i}_se" for i in range(4)]
    frames = [Image.open(RUNTIME / (name + ".png")).convert("RGBA") for name in names]
    if len({frame.size for frame in frames}) != 1:
        raise ValueError("Walk frames must share their complete canvas")
    report = json.loads((RUNTIME / "hero-walk.provenance.json").read_text(encoding="utf-8"))
    if {entry["cell_size"] for entry in report["assets"]} != {14}:
        raise ValueError("Walk frames must share the reviewed source grid")
    scale, duration = 4, 140
    width, height = frames[0].size
    preview_frames = []
    strip = Image.new("RGB", (width * scale * 4, height * scale + 26), (32, 31, 29))
    strip_draw = ImageDraw.Draw(strip)
    native_strip = Image.new("RGB", (width * 4, height + 20), (32, 31, 29))
    native_draw = ImageDraw.Draw(native_strip)
    for i, (name, frame) in enumerate(zip(names, frames)):
        enlarged = frame.resize((width * scale, height * scale), Image.Resampling.NEAREST)
        preview = Image.new("RGB", (width * scale, height * scale + 26), (32, 31, 29))
        preview.paste(enlarged, (0, 0), enlarged)
        draw = ImageDraw.Draw(preview)
        draw.line((0, height * scale, width * scale, height * scale), fill=(93, 103, 92))
        draw.text((6, height * scale + 8), f"Frame {i} / fixed origin", fill=(230, 225, 208))
        preview_frames.append(preview)
        strip.paste(preview, (i * width * scale, 0))
        native_strip.paste(frame, (i * width, 0), frame)
        native_draw.text((i * width + 3, height + 5), f"Frame {i}", fill=(230, 225, 208))
    strip.save(ROOT / "hero-walk-strip-4x.png")
    native_strip.save(ROOT / "hero-walk-strip-1x.png")
    preview_frames[0].save(ROOT / "hero-walk-preview.gif", save_all=True,
                           append_images=preview_frames[1:], duration=duration,
                           loop=0, optimize=False, disposal=2)
    print(f"Wrote fixed-origin 1x/4x strips and {duration}ms/frame GIF for {len(frames)} poses.")


if __name__ == "__main__":
    main()
