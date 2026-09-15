"""Regression checks for byte-preserving installation and appearance cohort isolation."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("installer", Path(__file__).with_name("install-first-slice-art.py"))
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


class InstallTests(unittest.TestCase):
    def test_update_replaces_whole_identity_without_resampling(self):
        image = Path(__file__).resolve().parents[1] / "client/assets/first-slice/reviewed/frames/female-sprint-back-00.png"
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            manifest, runtime = root / "input.json", root / "runtime"
            def clip(identity, action):
                return dict(identity=identity, action=action, direction="front", frames=[str(image)], frame=[96,96], anchor=[48,80], pixels_per_metre=48, fps=8, loop=True)
            manifest.write_text(json.dumps(dict(accepted=True, clips=[clip("player_female","idle"),clip("player_female","walk"),clip("player_female_club","idle"),clip("pack-wolf","idle")])))
            installer.install(manifest,runtime)
            manifest.write_text(json.dumps(dict(accepted=True, clips=[clip("player_female","sprint")])))
            installer.install(manifest,runtime)
            text = (runtime / "manifest.tsv").read_text()
            self.assertNotIn("player_female\tidle",text)
            self.assertNotIn("player_female\twalk",text)
            self.assertNotIn("player_female_club",text)
            self.assertIn("pack-wolf\tidle",text)
            self.assertIn("player_female\tsprint",text)
            for row in text.splitlines():
                if not row.startswith("#"):
                    self.assertEqual((runtime / (row.split()[-1]+".png")).read_bytes(),image.read_bytes())
            invalid=clip("player_female","idle")
            invalid["pixels_per_metre"]=147
            manifest.write_text(json.dumps(dict(accepted=True,clips=[invalid])))
            with self.assertRaises(ValueError):installer.install(manifest,runtime)
            self.assertEqual((runtime / "manifest.tsv").read_text(),text)


if __name__ == "__main__":
    unittest.main()
