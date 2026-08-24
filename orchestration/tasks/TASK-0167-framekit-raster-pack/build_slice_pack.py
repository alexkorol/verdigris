#!/usr/bin/env python3
"""TASK-0167: adopt real WIZARD Framekit art as native raster slices.

Reads (read-only):
  Z:/Code/WIZARD/tools/gui_framekit/assets/textures/{panel,slot}.png
  Z:/Code/WIZARD/tools/gui_framekit/assets/sprites/orb-{vitality,mana,essence}.png
  <runway>/native/client/assets/wizard/source_manifest.json  (hash provenance)

Writes into owned paths:
  native/client/assets/wizard/framekit/*.png
  native/client/assets/wizard/framekit/nine_slice.json
  native/client/assets/wizard/framekit/adoption_manifest.json
  orchestration/tasks/TASK-0167-framekit-raster-pack/contact_sheet.png

Deterministic: no timestamps beyond the pinned date string.
"""
from __future__ import annotations

import hashlib
import io
import json
import os
import sys

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
WORKTREE = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
OUT_DIR = os.path.join(WORKTREE, "native", "client", "assets", "wizard", "framekit")
RUNWAY_MANIFEST = r"Z:\Code\.worktrees\verdigris\owner-demo-runway\native\client\assets\wizard\source_manifest.json"
SOURCE_ROOT = r"Z:\Code\WIZARD"

GENERATED_AT = "2026-08-24"
DPI_SCALE = 4

SOURCES = {
    "panel": "tools/gui_framekit/assets/textures/panel.png",
    "slot": "tools/gui_framekit/assets/textures/slot.png",
    "orb-vitality": "tools/gui_framekit/assets/sprites/orb-vitality.png",
    "orb-mana": "tools/gui_framekit/assets/sprites/orb-mana.png",
    "orb-essence": "tools/gui_framekit/assets/sprites/orb-essence.png",
}

PANEL_SLICES = [
    ("panel_corner_tl.png", (0, 0, 12, 12)),
    ("panel_corner_tr.png", (36, 0, 12, 12)),
    ("panel_corner_bl.png", (0, 36, 12, 12)),
    ("panel_corner_br.png", (36, 36, 12, 12)),
    ("panel_edge_top.png", (12, 0, 24, 12)),
    ("panel_edge_bottom.png", (12, 36, 24, 12)),
    ("panel_edge_left.png", (0, 12, 12, 24)),
    ("panel_edge_right.png", (36, 12, 12, 24)),
    ("panel_fill.png", (12, 12, 24, 24)),
]


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def png_bytes(img: Image.Image) -> bytes:
    buf = io.BytesIO()
    img.save(buf, "PNG")
    return buf.getvalue()


def load_provenance() -> dict[str, dict]:
    with open(RUNWAY_MANIFEST, encoding="utf-8") as f:
        m = json.load(f)
    out = {}
    for art in m["families"]["framekit"]["artifacts"]:
        out[art["sourcePath"]] = art
    return out


def main() -> int:
    prov = load_provenance()
    os.makedirs(OUT_DIR, exist_ok=True)

    src_data: dict[str, bytes] = {}
    src_img: dict[str, Image.Image] = {}
    artifacts: list[dict] = []

    for name, rel in SOURCES.items():
        p = os.path.join(SOURCE_ROOT, rel.replace("/", os.sep))
        data = open(p, "rb").read()
        digest = sha256_bytes(data)
        rec = prov.get(rel)
        if not rec:
            print(f"FAIL: {rel} not listed in source_manifest.json")
            return 1
        if digest != rec["sha256"] or len(data) != rec["bytes"]:
            print(f"FAIL: provenance drift for {rel}")
            return 1
        img = Image.open(io.BytesIO(data))
        img.load()
        if img.mode != "RGBA":
            print(f"FAIL: {rel} is {img.mode}, expected RGBA")
            return 1
        src_data[name] = data
        src_img[name] = img

    def add(file: str, data: bytes, src_rel: str, adoption: str, extra: dict) -> None:
        img = Image.open(io.BytesIO(data))
        entry = {
            "file": file,
            "sourcePath": src_rel,
            "adoption": adoption,
            "sha256": sha256_bytes(data),
            "bytes": len(data),
            "width": img.width,
            "height": img.height,
            "mode": img.mode,
            "sourceSha256": prov[src_rel]["sha256"],
        }
        entry.update(extra)
        artifacts.append(entry)
        with open(os.path.join(OUT_DIR, file), "wb") as f:
            f.write(data)

    add("panel_frame.png", src_data["panel"], SOURCES["panel"], "verbatim", {})
    add("slot_frame.png", src_data["slot"], SOURCES["slot"], "verbatim", {})
    for key, fn in (("orb-vitality", "orb_vitality.png"),
                    ("orb-mana", "orb_mana.png"),
                    ("orb-essence", "orb_essence.png")):
        add(fn, src_data[key], SOURCES[key], "verbatim", {"role": "circular-frame"})
    for file, (x, y, w, h) in PANEL_SLICES:
        crop = src_img["panel"].crop((x, y, x + w, y + h))
        add(file, png_bytes(crop), SOURCES["panel"], "derived-slice", {
            "sliceRect": [x, y, w, h],
            "parentWidth": src_img["panel"].width,
            "parentHeight": src_img["panel"].height,
        })

    panel_w, panel_h = src_img["panel"].size
    slot_w, slot_h = src_img["slot"].size
    nine_slice = {
        "schemaVersion": 1,
        "task": "TASK-0167",
        "sourceFamily": "framekit",
        "generatedAt": GENERATED_AT,
        "dpiScale": DPI_SCALE,
        "panels": {
            "panel": {
                "texture": "panel_frame.png",
                "width": panel_w,
                "height": panel_h,
                "margins": {"top": 12, "right": 12, "bottom": 12, "left": 12},
                "slices": {
                    "corner_top_left": {"file": "panel_corner_tl.png", "rect": [0, 0, 12, 12],
                                        "anchor": ["left", "top"], "stretch": "none"},
                    "corner_top_right": {"file": "panel_corner_tr.png", "rect": [36, 0, 12, 12],
                                         "anchor": ["right", "top"], "stretch": "none"},
                    "corner_bottom_left": {"file": "panel_corner_bl.png", "rect": [0, 36, 12, 12],
                                           "anchor": ["left", "bottom"], "stretch": "none"},
                    "corner_bottom_right": {"file": "panel_corner_br.png", "rect": [36, 36, 12, 12],
                                            "anchor": ["right", "bottom"], "stretch": "none"},
                    "edge_top": {"file": "panel_edge_top.png", "rect": [12, 0, 24, 12],
                                 "anchor": ["top"], "stretch": "horizontal"},
                    "edge_bottom": {"file": "panel_edge_bottom.png", "rect": [12, 36, 24, 12],
                                    "anchor": ["bottom"], "stretch": "horizontal"},
                    "edge_left": {"file": "panel_edge_left.png", "rect": [0, 12, 12, 24],
                                  "anchor": ["left"], "stretch": "vertical"},
                    "edge_right": {"file": "panel_edge_right.png", "rect": [36, 12, 12, 24],
                                   "anchor": ["right"], "stretch": "vertical"},
                    "fill": {"file": "panel_fill.png", "rect": [12, 12, 24, 24],
                             "anchor": ["center"], "stretch": "both"},
                },
            },
            "slot": {
                "texture": "slot_frame.png",
                "width": slot_w,
                "height": slot_h,
                "margins": {"top": 12, "right": 12, "bottom": 12, "left": 12},
                "slices": {},
            },
        },
        "sprites": {
            "orb_vitality": {"file": "orb_vitality.png", "width": src_img["orb-vitality"].width,
                             "height": src_img["orb-vitality"].height,
                             "anchor": ["center"], "role": "circular-frame"},
            "orb_mana": {"file": "orb_mana.png", "width": src_img["orb-mana"].width,
                         "height": src_img["orb-mana"].height,
                         "anchor": ["center"], "role": "circular-frame"},
            "orb_essence": {"file": "orb_essence.png", "width": src_img["orb-essence"].width,
                            "height": src_img["orb-essence"].height,
                            "anchor": ["center"], "role": "circular-frame"},
        },
    }
    with open(os.path.join(OUT_DIR, "nine_slice.json"), "w", encoding="utf-8") as f:
        json.dump(nine_slice, f, indent=2, sort_keys=True)
        f.write("\n")

    manifest = {
        "schemaVersion": 1,
        "task": "TASK-0167",
        "generatedAt": GENERATED_AT,
        "sourceRoot": "Z:/Code/WIZARD",
        "sourceCommit": "66a5d9ff6810e886c1bd08cbeaaf83cabf92aae9",
        "artifacts": sorted(artifacts, key=lambda a: a["file"]),
    }
    with open(os.path.join(OUT_DIR, "adoption_manifest.json"), "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)
        f.write("\n")

    sheet = os.path.join(HERE, "contact_sheet.png")
    render_contact_sheet(manifest, nine_slice, sheet)
    print(f"OK: adopted {len(artifacts)} artifacts -> {OUT_DIR}")
    return 0


def nine_patch(imgs: dict[str, Image.Image], w: int, h: int) -> Image.Image:
    m = 12
    out = Image.new("RGBA", (w, h))
    cw = w - 2 * m
    ch = h - 2 * m
    out.paste(imgs["tl"], (0, 0))
    out.paste(imgs["tr"], (w - m, 0))
    out.paste(imgs["bl"], (0, h - m))
    out.paste(imgs["br"], (w - m, h - m))
    out.paste(imgs["t"].resize((cw, m), Image.NEAREST), (m, 0))
    out.paste(imgs["b"].resize((cw, m), Image.NEAREST), (m, h - m))
    out.paste(imgs["l"].resize((m, ch), Image.NEAREST), (0, m))
    out.paste(imgs["r"].resize((m, ch), Image.NEAREST), (w - m, m))
    out.paste(imgs["f"].resize((cw, ch), Image.NEAREST), (m, m))
    return out


def render_contact_sheet(manifest: dict, nine_slice: dict, out_path: str) -> None:
    arts = {a["file"]: a for a in manifest["artifacts"]}
    names = sorted(arts)
    cell_w, cell_h = 140, 110
    label_h = 18
    cols = 4
    rows = (len(names) + cols - 1) // cols
    recon_h = 210
    W = cols * cell_w + 20
    H = rows * (cell_h + label_h) + recon_h + 48
    sheet = Image.new("RGBA", (W, H), (13, 12, 10, 255))
    d = ImageDraw.Draw(sheet)
    try:
        font = ImageFont.truetype("consola.ttf", 10)
        title_font = ImageFont.truetype("consolab.ttf", 14)
    except OSError:
        font = ImageFont.load_default()
        title_font = font

    d.text((10, 8), "TASK-0167 Framekit raster slice pack - adopted artifacts (WIZARD gui_framekit)",
           fill=(201, 169, 106, 255), font=title_font)

    for i, name in enumerate(names):
        cx = 10 + (i % cols) * cell_w
        cy = 32 + (i // cols) * (cell_h + label_h)
        a = arts[name]
        img = Image.open(os.path.join(OUT_DIR, name)).convert("RGBA")
        scale = max(1, min((cell_w - 12) // img.width, (cell_h - 12) // img.height))
        disp = img.resize((img.width * scale, img.height * scale), Image.NEAREST)
        dx = cx + (cell_w - disp.width) // 2
        dy = cy + (cell_h - disp.height) // 2
        checker = Image.new("RGBA", disp.size, (60, 56, 50, 255))
        cd = ImageDraw.Draw(checker)
        for yy in range(0, disp.height, 8):
            for xx in range(0, disp.width, 8):
                if (xx // 8 + yy // 8) % 2 == 0:
                    cd.rectangle([xx, yy, xx + 7, yy + 7], fill=(80, 76, 68, 255))
        checker.alpha_composite(disp)
        d.rectangle([cx, cy, cx + cell_w - 2, cy + cell_h - 2], outline=(138, 113, 70, 255))
        sheet.alpha_composite(checker, (dx, dy))
        color = (76, 169, 154, 255) if a["adoption"] == "derived-slice" else (233, 228, 216, 255)
        d.text((cx + 4, cy + cell_h + 3), name, fill=color, font=font)

    y0 = rows * (cell_h + label_h) + 52
    d.text((10, y0 - 24), "nine-slice reconstruction proof (edges/fill stretched):",
           fill=(201, 169, 106, 255), font=title_font)
    p = {k: Image.open(os.path.join(OUT_DIR, f)).convert("RGBA")
         for k, f in {
             "tl": "panel_corner_tl.png", "tr": "panel_corner_tr.png",
             "bl": "panel_corner_bl.png", "br": "panel_corner_br.png",
             "t": "panel_edge_top.png", "b": "panel_edge_bottom.png",
             "l": "panel_edge_left.png", "r": "panel_edge_right.png",
             "f": "panel_fill.png"}.items()}
    big = nine_patch(p, 220, 160)
    d.rectangle([9, y0 - 1, 9 + 220, y0 + 160], outline=(138, 113, 70, 255))
    sheet.alpha_composite(big, (10, y0))
    d.text((240, y0 + 4), "panel_frame.png @ 220x160", fill=(233, 228, 216, 255), font=font)
    slot = Image.open(os.path.join(OUT_DIR, "slot_frame.png")).convert("RGBA")
    slot_big = slot.resize((96, 96), Image.NEAREST)
    sx, sy = 240, y0 + 30
    d.rectangle([sx - 1, sy - 1, sx + 96, sy + 96], outline=(138, 113, 70, 255))
    sheet.alpha_composite(slot_big, (sx, sy))
    d.text((sx + 104, sy + 40), "slot_frame.png @ 96x96 (whole-texture nine-slice)",
           fill=(233, 228, 216, 255), font=font)

    sheet.convert("RGB").save(out_path, "PNG")


if __name__ == "__main__":
    sys.exit(main())
