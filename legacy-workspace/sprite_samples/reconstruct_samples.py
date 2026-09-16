"""
Reconstruct ChatGPT-generated sprites with pixel-perfecter and build comparisons.

No API calls. Operates on *_generated.png files already saved in this folder
(captured from the ChatGPT web app via receive_images.py + capture_chatgpt.js).

Usage (from your Windows machine, in the sprite_samples folder):
    python reconstruct_samples.py            # process every *_generated.png present
    python reconstruct_samples.py weapon      # process only weapon_generated.png
    python reconstruct_samples.py weapon armor
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).parent.resolve()

# pixel-perfecter lives in Code/Python/ (same resolution as generate_samples.py)
PP_DIR = HERE.parent.parent.parent / "Python" / "pixel-perfecter"
if not PP_DIR.exists():
    PP_DIR = Path("Z:/Code/Python/pixel-perfecter")
sys.path.insert(0, str(PP_DIR))
from pixel_perfecter.reconstructor import PixelArtReconstructor


def reconstruct(img):
    arr = np.array(img.convert("RGB"))
    rec = PixelArtReconstructor(image=arr)
    result_arr = rec.run(mode="global")
    return Image.fromarray(result_arr.astype(np.uint8), "RGB")


def make_comparison(orig_path, generated, reconstructed, label):
    SIZE = 256
    PADDING = 10
    HEADER = 28

    panels = []  # (image, caption)
    if orig_path.exists():
        panels.append((Image.open(orig_path).convert("RGBA").resize((SIZE, SIZE), Image.NEAREST),
                       label.upper() + "  |  Original (32x32)"))
    gen_disp = generated.resize(
        (SIZE, SIZE),
        Image.LANCZOS if max(generated.size) > SIZE else Image.NEAREST)
    panels.append((gen_disp, "ChatGPT generated"))
    panels.append((reconstructed.resize((SIZE, SIZE), Image.NEAREST), "pixel-perfecter cleaned"))

    n = len(panels)
    total_w = SIZE * n + PADDING * (n + 1)
    total_h = SIZE + HEADER + PADDING * 2
    canvas = Image.new("RGB", (total_w, total_h), (30, 30, 30))
    draw = ImageDraw.Draw(canvas)

    for i, (panel, caption) in enumerate(panels):
        x = PADDING * (i + 1) + SIZE * i
        bg = Image.new("RGB", (SIZE, SIZE), (20, 20, 20))
        if panel.mode == "RGBA":
            bg.paste(panel, mask=panel.split()[3])
        else:
            bg.paste(panel)
        canvas.paste(bg, (x, HEADER + PADDING))
        draw.text((x, 6), caption, fill=(180, 180, 180))

    return canvas


def process(label):
    gen_path = HERE / (label + "_generated.png")
    if not gen_path.exists():
        print("  SKIP " + label + " (no " + gen_path.name + ")")
        return False

    orig = HERE / (label + "_original_32x32.png")
    rec_path = HERE / (label + "_reconstructed.png")
    cmp_path = HERE / (label + "_comparison.png")

    print("Processing: " + label)
    generated = Image.open(gen_path).convert("RGBA")
    print("  generated size: %dx%d" % generated.size)

    reconstructed = reconstruct(generated)
    reconstructed.save(rec_path)
    print("  Saved reconstructed -> " + rec_path.name)

    comparison = make_comparison(orig, generated, reconstructed, label)
    comparison.save(cmp_path)
    print("  Saved comparison    -> " + cmp_path.name)
    return True


def main():
    print("Delaford sprite reconstruction (pixel-perfecter: %s)" % PP_DIR)
    args = [a.lower() for a in sys.argv[1:]]
    if args:
        labels = args
    else:
        labels = sorted(p.name[:-len("_generated.png")]
                        for p in HERE.glob("*_generated.png"))
        if not labels:
            print("No *_generated.png files found. Capture some from ChatGPT first.")
            return

    done = 0
    for label in labels:
        if process(label):
            done += 1
    print("")
    print("Done. Reconstructed %d sprite(s). Review the *_comparison.png files." % done)


if __name__ == "__main__":
    main()
