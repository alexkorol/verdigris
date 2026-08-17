# TASK-0024 review-revision evidence: average luminance of two capture JPEGs.
# Usage: python measure-luminance.py <before.jpg> <after.jpg>
import sys
from PIL import Image

def average_luminance(path):
    image = Image.open(path).convert("L")
    pixels = list(image.getdata())
    return sum(pixels) / len(pixels)

before = average_luminance(sys.argv[1])
after = average_luminance(sys.argv[2])
print(f"before: {before:.2f}  after: {after:.2f}  delta: {after - before:+.2f}")
sys.exit(0 if after >= before else 1)
