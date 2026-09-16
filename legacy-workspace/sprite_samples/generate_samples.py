"""
Generate 3 dark bronze-age pixel art sprite revamps using Gemini via OpenRouter,
then clean them up with pixel-perfecter's reconstructor.

Run this from your Windows machine:
    python sprite_samples/generate_samples.py
"""

import base64
import json
import sys
import urllib.request
from io import BytesIO
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).parent.resolve()

# pixel-perfecter lives two levels up from delaford, in Code/Python/
PP_DIR = HERE.parent.parent.parent / "Python" / "pixel-perfecter"
if not PP_DIR.exists():
    # fallback: try sibling of delaford's Code parent
    PP_DIR = Path("Z:/Code/Python/pixel-perfecter")

sys.path.insert(0, str(PP_DIR))
from pixel_perfecter.reconstructor import PixelArtReconstructor

# Load API key from pixel-perfecter .env (robust against quotes / CRLF / 'export ')
_env = (PP_DIR / ".env").read_text()
OPENROUTER_API_KEY = ""
for _line in _env.splitlines():
    _line = _line.strip()
    if _line.startswith("export "):
        _line = _line[len("export "):].strip()
    if _line.startswith("OPENROUTER_API_KEY"):
        _val = _line.split("=", 1)[1].strip()
        # strip matching surrounding quotes and any stray whitespace/quotes
        if len(_val) >= 2 and _val[0] == _val[-1] and _val[0] in "\"'":
            _val = _val[1:-1]
        OPENROUTER_API_KEY = _val.strip().strip("\"'").strip()
        break

# Masked diagnostic so we can debug auth without printing the secret
_k = OPENROUTER_API_KEY
if _k:
    _masked = _k[:8] + "..." + _k[-4:] if len(_k) > 14 else "(too short)"
    print("  [key] len=%d prefix_ok=%s value=%s" % (
        len(_k), str(_k.startswith("sk-or-")), _masked))
else:
    print("  [key] NOT FOUND in .env")

MODEL = "google/gemini-2.5-flash-image"

DARK_BRONZE_PROMPT = (
    "Redesign the sprite in this image as TRUE LOW-RESOLUTION pixel art. "
    "This is the most important constraint: the output must look like it was made "
    "in a pixel art editor at exactly 64x64 pixels — not a photo, not concept art, "
    "not a 'pixel art style' rendering at high resolution.\n\n"
    "RESOLUTION — CRITICAL:\n"
    "- The output image is 64x64 pixels total. That is 64 columns and 64 rows of pixels.\n"
    "- Every single 'pixel' in the image must be ONE solid-color square — no gradients "
    "within a pixel, no sub-pixel detail, no anti-aliasing between any two pixels.\n"
    "- Think: NES sprite, SNES RPG item icon, or classic Diablo 2 inventory icon — "
    "blocky, chunky, visibly pixelated. If you zoom in you see hard squares.\n"
    "- Maximum color palette: 16-32 distinct colors total. No smooth color ramps.\n"
    "- DO NOT produce a high-resolution image that looks like pixel art. "
    "DO NOT render at 512x512 or 1024x1024. Output exactly 64x64 pixels.\n\n"
    "STYLE:\n"
    "- Dark, gritty low-power bronze-age fantasy (Diablo 2 item icons, early Path of Exile)\n"
    "- Palette: aged bronze, dark iron, worn leather, ochre, rust, dark sienna, "
    "charcoal, bone. NO bright fantasy colors, NO saturated blues or greens.\n"
    "- Torchlit atmosphere: warm amber highlights, deep cool shadows, dramatic contrast\n"
    "- Weathered, ancient: scratches, wear marks, aged patina on metals\n\n"
    "TECHNICAL:\n"
    "- Hard pixel edges everywhere — zero blur between adjacent pixels\n"
    "- Transparent or near-black (#0a0a0a) background\n"
    "- 3-4 shading values per surface (highlight, midtone, shadow, deep shadow)\n"
    "- Maintain the same subject type and orientation as the reference sprite"
)


def call_openrouter(sprite_path: Path) -> Image.Image:
    sprite = Image.open(sprite_path).convert("RGBA")
    w, h = sprite.size
    if max(w, h) < 256:
        scale = 256 // max(w, h)
        sprite = sprite.resize((w * scale, h * scale), Image.NEAREST)

    buf = BytesIO()
    sprite.save(buf, format="PNG")
    b64 = base64.b64encode(buf.getvalue()).decode()

    payload = json.dumps({
        "model": MODEL,
        "max_tokens": 4096,
        "messages": [{
            "role": "user",
            "content": [
                {"type": "image_url", "image_url": {"url": "data:image/png;base64," + b64}},
                {"type": "text", "text": DARK_BRONZE_PROMPT},
            ],
        }],
    }).encode()

    req = urllib.request.Request(
        "https://openrouter.ai/api/v1/chat/completions",
        data=payload,
        headers={
            "Content-Type": "application/json",
            "Authorization": "Bearer " + OPENROUTER_API_KEY,
        },
    )

    print("  Calling OpenRouter (Gemini)...")
    with urllib.request.urlopen(req, timeout=120) as resp:
        data = json.loads(resp.read().decode())

    msg = data.get("choices", [{}])[0].get("message", {})

    # Extract image from response
    result_img = None
    for img_entry in msg.get("images", []):
        url = img_entry.get("image_url", {}).get("url", "") if isinstance(img_entry, dict) else str(img_entry)
        if url.startswith("data:"):
            img_bytes = base64.b64decode(url.split(",", 1)[1])
            result_img = Image.open(BytesIO(img_bytes)).convert("RGBA")
            break

    if result_img is None:
        content = msg.get("content", "")
        if isinstance(content, list):
            for part in content:
                if isinstance(part, dict) and part.get("type") == "image_url":
                    url = part["image_url"]["url"]
                    if url.startswith("data:"):
                        img_bytes = base64.b64decode(url.split(",", 1)[1])
                        result_img = Image.open(BytesIO(img_bytes)).convert("RGBA")
                        break

    if result_img is None:
        raise RuntimeError("No image in response. Preview: " + str(msg)[:400])

    return result_img


def reconstruct(img: Image.Image) -> Image.Image:
    arr = np.array(img.convert("RGB"))
    rec = PixelArtReconstructor(image=arr)
    result_arr = rec.run(mode="global")
    return Image.fromarray(result_arr.astype(np.uint8), "RGB")


def make_comparison(orig_path: Path, generated: Image.Image, reconstructed: Image.Image, label: str) -> Image.Image:
    SIZE = 256
    PADDING = 10
    HEADER = 28
    original = Image.open(orig_path).convert("RGBA")
    orig_d = original.resize((SIZE, SIZE), Image.NEAREST)
    gen_d = generated.resize((SIZE, SIZE), Image.LANCZOS if max(generated.size) > SIZE else Image.NEAREST)
    rec_d = reconstructed.resize((SIZE, SIZE), Image.NEAREST)

    total_w = SIZE * 3 + PADDING * 4
    total_h = SIZE + HEADER + PADDING * 2
    canvas = Image.new("RGB", (total_w, total_h), (30, 30, 30))

    def paste(panel, x, y):
        bg = Image.new("RGB", (SIZE, SIZE), (20, 20, 20))
        if panel.mode == "RGBA":
            bg.paste(panel, mask=panel.split()[3])
        else:
            bg.paste(panel)
        canvas.paste(bg, (x, y))

    paste(orig_d, PADDING, HEADER + PADDING)
    paste(gen_d,  PADDING * 2 + SIZE, HEADER + PADDING)
    paste(rec_d,  PADDING * 3 + SIZE * 2, HEADER + PADDING)

    draw = ImageDraw.Draw(canvas)
    draw.text((PADDING, 6), label.upper() + "  |  Original (32x32)", fill=(180, 180, 180))
    draw.text((PADDING * 2 + SIZE, 6), "Gemini generated", fill=(180, 180, 180))
    draw.text((PADDING * 3 + SIZE * 2, 6), "pixel-perfecter cleaned", fill=(180, 180, 180))
    return canvas


def process_sample(label):
    orig = HERE / (label + "_original_32x32.png")
    gen_path = HERE / (label + "_generated.png")
    rec_path = HERE / (label + "_reconstructed.png")
    cmp_path = HERE / (label + "_comparison.png")

    print("")
    print("=" * 50)
    print("Processing: " + label)
    print("=" * 50)

    generated = call_openrouter(orig)
    generated.save(gen_path)
    print("  Saved generated     -> " + gen_path.name)

    reconstructed = reconstruct(generated)
    reconstructed.save(rec_path)
    print("  Saved reconstructed -> " + rec_path.name)

    comparison = make_comparison(orig, generated, reconstructed, label)
    comparison.save(cmp_path)
    print("  Saved comparison    -> " + cmp_path.name)


def main():
    print("Delaford Sprite Sample Generator")
    print("Model: " + MODEL)
    print("pixel-perfecter: " + str(PP_DIR))

    labels = ("weapon", "armor", "terrain")
    failures = []
    for label in labels:
        try:
            process_sample(label)
        except Exception as e:
            print("  ERROR on " + label + ": " + repr(e))
            failures.append(label)

    print("")
    print("=" * 50)
    if failures:
        print("Completed with errors on: " + ", ".join(failures))
    else:
        print("All 3 samples done. Review the *_comparison.png files.")
    print("=" * 50)


if __name__ == "__main__":
    main()
