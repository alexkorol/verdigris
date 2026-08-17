"""TASK-0027 HUD-orb luminance proof.

Measures the HP/MP orb disc regions across the evidence captures. The spec's
required item 1: a night capture where the orbs read exactly as bright as at
midday. Sample boxes are fixed screen regions over the orb discs at the
1440x1000 evidence viewport (kept clear of the readout bar).

Usage: python measure-orbs.py
"""

from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent

# (left, top, right, bottom) boxes over the orb discs at 1440x1000.
HP_ORB_BOX = (140, 780, 205, 850)
MP_ORB_BOX = (1250, 780, 1330, 850)

SHOTS = [
    'after-arpg.jpg',                    # midday baseline
    'after-night.jpg',                   # coherent-clock night grade
    'probe-night-real-unpatched.jpg',    # ground truth: real 240 s night
    'probe-night-naive-clock.jpg',       # diagnostic: the 0024 artifact
]


def luminance(region: np.ndarray) -> float:
    if region.size == 0:
        return 0.0
    weighted = (
        0.2126 * region[..., 0]
        + 0.7152 * region[..., 1]
        + 0.0722 * region[..., 2]
    )
    return float(weighted.mean())


def measure(name: str) -> None:
    path = HERE / name
    if not path.exists():
        print(f'{name}: MISSING')
        return
    image = Image.open(path).convert('RGB')
    hp = luminance(np.asarray(image.crop(HP_ORB_BOX), dtype=float))
    mp = luminance(np.asarray(image.crop(MP_ORB_BOX), dtype=float))
    frame = luminance(np.asarray(image, dtype=float))
    print(f'{name}: HP-orb {hp:.2f}  MP-orb {mp:.2f}  frame {frame:.2f}')


for shot_name in SHOTS:
    measure(shot_name)
