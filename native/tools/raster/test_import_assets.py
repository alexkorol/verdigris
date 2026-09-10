"""Focused import checks against the actual external reconstruction project."""
import json
from pathlib import Path
import tempfile
import unittest

import numpy as np
from PIL import Image

import import_assets


class RasterImportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.workspace = import_assets.load_engine(import_assets.DEFAULT_PROJECT)

    def test_real_reconstructor_preserves_coverage_and_ignores_invisible_rgb(self):
        source = np.array([
            [[240, 120, 40, 255], [0, 20, 255, 0], [60, 80, 100, 255], [60, 80, 100, 255]],
            [[240, 120, 40, 255], [0, 20, 255, 0], [60, 80, 100, 255], [60, 80, 100, 255]],
            [[99, 0, 0, 0], [99, 0, 0, 0], [10, 30, 50, 64], [10, 30, 50, 64]],
            [[99, 0, 0, 0], [99, 0, 0, 0], [10, 30, 50, 64], [10, 30, 50, 64]],
        ], dtype=np.uint8)
        result = self.workspace.reconstruct(source, self.workspace.Options(cell_size=2, alpha_mode="preserve"))
        np.testing.assert_array_equal(result.image, np.array([
            [[240, 120, 40, 128], [60, 80, 100, 255]],
            [[0, 0, 0, 0], [10, 30, 50, 64]],
        ], dtype=np.uint8))

    def test_normalization_does_not_double_multiply_alpha(self):
        source = Image.new("RGBA", (4, 4))
        source.putpixel((2, 1), (230, 120, 50, 64))
        normalized, geometry = import_assets.normalize(source, {"canvas": [6, 8]})
        self.assertEqual(normalized.getpixel((2, 7)), (230, 120, 50, 64))
        self.assertEqual(geometry["anchor_px"], [3, 8])
        self.assertEqual(geometry["baseline_offset_px"], 0)

    def test_sheet_import_orders_cells_and_is_reproducible(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            root = Path(directory)
            source = Image.new("RGBA", (16, 8))
            source.paste((140, 90, 20, 255), (2, 2, 6, 6))
            source.paste((10, 150, 110, 255), (10, 2, 14, 6))
            source.save(root / "source.png")
            manifest = root / "test.json"
            manifest.write_text(json.dumps({
                "version": 1, "output_dir": "out", "defaults": {
                    "cell_size": 2, "canvas": [4, 6], "require_transparency": True},
                "sheets": [{"source": "source.png", "grid": [2, 1], "assets": [
                    {"name": "first", "cell": [0, 0]}, {"name": "second", "cell": [1, 0]}]}]
            }), encoding="utf-8")
            first = import_assets.run(manifest, import_assets.DEFAULT_PROJECT)
            second = import_assets.run(manifest, import_assets.DEFAULT_PROJECT)
            self.assertEqual([a["sha256"] for a in first["assets"]],
                             [a["sha256"] for a in second["assets"]])
            self.assertEqual(first["assets"][1]["source_box"], [8, 0, 16, 8])
            with Image.open(root / "out" / "first.png") as result:
                self.assertEqual(result.size, (4, 6))
                self.assertEqual(result.getpixel((1, 5)), (140, 90, 20, 255))
                self.assertEqual(result.getpixel((0, 0))[3], 0)
            self.assertIn("pixel_perfecter/workspace.py", first["engine"]["source_sha256"])

    def test_rejects_unintended_opaque_sprite_and_nonintegral_sheet(self):
        with self.assertRaisesRegex(ValueError, "fully opaque"):
            import_assets.normalize(Image.new("RGBA", (3, 3), (1, 2, 3, 255)),
                                    {"require_transparency": True})
        with self.assertRaisesRegex(ValueError, "not divisible"):
            import_assets.cell_box({"grid": [4, 3]}, {"cell": [0, 0]}, (1254, 1254))

    def test_animation_anchor_keeps_authored_position_and_scale(self):
        standing = Image.new("RGBA", (8, 8))
        standing.paste((140, 90, 20, 255), (3, 1, 5, 7))
        crouched = Image.new("RGBA", (8, 8))
        crouched.paste((140, 90, 20, 255), (3, 4, 5, 7))
        options = {"canvas": [8, 10], "preserve_scale": True, "anchor_reconstructed": [4, 7]}
        stand, stand_geom = import_assets.normalize(standing, options)
        crouch, crouch_geom = import_assets.normalize(crouched, options)
        self.assertEqual(stand_geom["placed_size"], [2, 6])
        self.assertEqual(crouch_geom["placed_size"], [2, 3])
        self.assertEqual(stand.getpixel((3, 9)), crouch.getpixel((3, 9)))

    def test_reviewed_background_window_preserves_same_color_detail_elsewhere(self):
        source = np.full((10, 10, 4), (24, 22, 20, 255), dtype=np.uint8)
        source[3:5, 3:5] = (230, 230, 232, 255)  # Reviewed enclosed background.
        source[7:9, 7:9] = (230, 230, 232, 255)  # Same-color item detail, retained.
        result, record = import_assets.remove_background(source, {
            "color": [231, 231, 233], "tolerance": 68,
            "interior_windows": [[3, 3, 5, 5]],
        })
        self.assertTrue((result[3:5, 3:5, 3] == 0).all())
        self.assertTrue((result[7:9, 7:9, 3] == 255).all())
        np.testing.assert_array_equal(result[:, :, :3], source[:, :, :3])
        self.assertEqual(record["interior_windows"][0]["new_transparent_pixels"], 4)


if __name__ == "__main__":
    unittest.main()
