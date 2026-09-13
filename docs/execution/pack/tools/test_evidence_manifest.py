import json
import tempfile
import unittest
from pathlib import Path

from evidence_manifest import validate_manifest

NAKED_SHOT = {
    "schema_version": 1,
    "planning_id": "VG-QA-001",
    "base_sha": "abc",
    "head_sha": "def",
    "platform": "win32",
    "toolchain": "msvc",
    "commands": [{"run": "echo", "exit_code": 0}],
    "assertions": ["x"],
    "negative_control": {"name": "naked shot", "observed_result": "REJECTED"},
    "artifacts": [{"kind": "screenshot", "path": "shot.png"}],
    "integration_status": "NOT_INTEGRATED",
}


class EvidenceManifestTests(unittest.TestCase):
    def test_template_cannot_certify(self):
        data = {"template_only": True, "schema_version": 1}
        errors = validate_manifest(data)
        self.assertTrue(any("template_only" in e for e in errors))

    def test_screenshot_without_provenance_fails(self):
        errors = validate_manifest(NAKED_SHOT)
        self.assertTrue(any("provenance" in e or "screenshot" in e for e in errors))

    def test_minted_task_id_fails(self):
        data = dict(NAKED_SHOT)
        data["task_id"] = "TASK-9999"
        data["artifacts"] = [
            {"kind": "screenshot", "path": "shot.png", "sha256": "aa", "produced_by": "sc"}
        ]
        errors = validate_manifest(data)
        self.assertTrue(any("task_id" in e for e in errors))

    def test_traced_manifest_passes(self):
        data = dict(NAKED_SHOT)
        data["artifacts"] = [
            {
                "kind": "screenshot",
                "path": "docs/execution/captures/art-wave/build-fixtures-960x600.png",
                "sha256": "deadbeef",
                "produced_by": "verdigris_client.exe --scenario build-fixtures",
            }
        ]
        errors = validate_manifest(data)
        self.assertEqual(errors, [])

    def test_sha_mismatch_when_file_present(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            shot = root / "shot.png"
            shot.write_bytes(b"png")
            data = dict(NAKED_SHOT)
            data["artifacts"] = [
                {
                    "kind": "screenshot",
                    "path": "shot.png",
                    "sha256": "00",
                    "produced_by": "scenario",
                }
            ]
            errors = validate_manifest(data, repo=root)
            self.assertTrue(any("mismatch" in e for e in errors))


if __name__ == "__main__":
    unittest.main()
