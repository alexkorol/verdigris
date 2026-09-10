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

    def test_shared_cycle_palette_preserves_alpha_and_maps_common_colors_consistently(self):
        colors = [(20, 15, 10, 255), (170, 82, 35, 255), (179, 91, 40, 255),
                  (190, 122, 76, 255), (70, 60, 54, 255), (34, 42, 79, 255)]
        frames = [Image.new("RGBA", (4, 3)) for _ in range(2)]
        for i, color in enumerate(colors):
            frames[0].putpixel((i % 4, i // 4), color)
            frames[1].putpixel((3 - i % 4, 2 - i // 4), color)
        result, record = import_assets.reduce_cycle_colors(frames, 4)
        all_colors = set()
        for original, art in zip(frames, result):
            self.assertEqual(art.size, original.size)
            np.testing.assert_array_equal(np.asarray(art)[:, :, 3], np.asarray(original)[:, :, 3])
            all_colors.update(tuple(pixel[:3]) for pixel in np.asarray(art).reshape(-1, 4) if pixel[3])
        self.assertLessEqual(len(all_colors), 4)
        for i in range(len(colors)):
            self.assertEqual(result[0].getpixel((i % 4, i // 4)),
                             result[1].getpixel((3 - i % 4, 2 - i // 4)))
        self.assertEqual(record["api"], "pixel_perfecter.palettes.reduce_colors")

    def test_reference_palette_uses_visible_colors_and_preserves_native_anchor_alpha(self):
        from pixel_perfecter.palettes import snap_to_palette
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            root = Path(directory)
            (root / "refs").mkdir()
            reference = Image.new("RGBA", (3, 1))
            reference.putdata([(120, 50, 20, 255), (35, 42, 70, 96), (255, 0, 255, 0)])
            reference.save(root / "refs/accepted.png")
            source = Image.new("RGBA", (6, 8), (255, 0, 255, 0))
            source.putpixel((1, 2), (129, 55, 18, 255))
            source.putpixel((4, 6), (34, 39, 68, 64))
            source.save(root / "source.png")
            manifest = {"version": 1, "output_dir": "before", "defaults": {
                "cell_size": 1, "alpha_mode": "preserve", "canvas": [6, 8],
                "trim": False, "preserve_scale": True, "anchor_source": [3, 8]},
                "sheets": [{"source": "source.png", "grid": [1, 1],
                            "assets": [{"name": "pose", "cell": [0, 0]}]}]}
            path = root / "test.json"
            path.write_text(json.dumps(manifest), encoding="utf-8")
            before = import_assets.run(path, import_assets.DEFAULT_PROJECT)
            original = np.asarray(Image.open(root / "before/pose.png").convert("RGBA"))
            self.assertNotIn("palette_reference", before)
            manifest.update(output_dir="after", palette_reference={"source": "refs/accepted.png", "max_colors": 2})
            path.write_text(json.dumps(manifest), encoding="utf-8")
            after = import_assets.run(path, import_assets.DEFAULT_PROJECT)
            result = np.asarray(Image.open(root / "after/pose.png").convert("RGBA"))
            palette = np.array([[35, 42, 70], [120, 50, 20]], dtype=np.uint8)
            expected = snap_to_palette(original, palette)
            expected[expected[:, :, 3] == 0, :3] = 0
            np.testing.assert_array_equal(result, expected)
            np.testing.assert_array_equal(result[:, :, 3], np.asarray(source)[:, :, 3])
            self.assertEqual(after["assets"][0]["anchor_px"], [3, 8])
            for key in ("content_bounds", "trim_box", "canvas", "placed_size", "before_resize"):
                self.assertEqual(before["assets"][0][key], after["assets"][0][key])
            self.assertEqual(after["palette_reference"]["palette_rgb"], palette.tolist())
            self.assertEqual(after["palette_reference"]["sha256"], import_assets.digest(root / "refs/accepted.png"))
            self.assertEqual(after["palette_reference"]["api"], "pixel_perfecter.palettes.snap_to_palette")

    def test_invalid_reference_never_writes_output(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            root = Path(directory)
            Image.new("RGBA", (2, 2)).save(root / "empty.png")
            Image.new("RGB", (2, 2)).save(root / "wrong.jpg")
            colors = Image.new("RGB", (2, 1))
            colors.putdata([(1, 2, 3), (5, 6, 7)])
            colors.save(root / "colors.png")
            for reference, error in [
                (None, "source PNG path"), ({"source": "missing.png"}, "Cannot read"),
                ({"source": "empty.png"}, "no visible colors"),
                ({"source": "wrong.jpg"}, "single-frame PNG"),
                ({"source": "colors.png", "max_colors": 1}, "exceeding"),
                ({"source": "colors.png", "max_colors": True}, "integer"),
                ({"source": "colors.png", "max_colors": 257}, "integer"),
            ]:
                with self.subTest(reference=reference):
                    path = root / "test.json"
                    path.write_text(json.dumps({"version": 1, "output_dir": "out",
                        "palette_reference": reference, "sheets": []}), encoding="utf-8")
                    with self.assertRaisesRegex(ValueError, error):
                        import_assets.run(path, import_assets.DEFAULT_PROJECT)
                    self.assertFalse((root / "out").exists())

    def test_reference_rejects_competing_color_operations(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            root = Path(directory)
            Image.new("RGBA", (2, 2), (120, 50, 20, 255)).save(root / "source.png")
            base = {"version": 1, "output_dir": "out", "palette_reference": {"source": "source.png"},
                    "sheets": [{"source": "source.png", "grid": [1, 1],
                                "assets": [{"name": "pose", "cell": [0, 0]}]}]}
            for extra in ({"shared_palette_max_colors": 32}, {"defaults": {"palette": "db32"}},
                          {"defaults": {"max_colors": 16}}):
                with self.subTest(extra=extra):
                    path = root / "test.json"
                    path.write_text(json.dumps({**base, **extra}), encoding="utf-8")
                    with self.assertRaisesRegex(ValueError, "palette_reference"):
                        import_assets.run(path, import_assets.DEFAULT_PROJECT)
                    self.assertFalse((root / "out").exists())


if __name__ == "__main__":
    unittest.main()
