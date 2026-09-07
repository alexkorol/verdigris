#!/usr/bin/env python3
"""VG-QA-001: evidence manifests. A screenshot with no provenance cannot certify."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

REQUIRED = (
    "schema_version",
    "planning_id",
    "base_sha",
    "head_sha",
    "platform",
    "toolchain",
    "commands",
    "assertions",
    "negative_control",
    "artifacts",
    "integration_status",
)


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def validate_manifest(data: dict[str, Any], *, repo: Path | None = None) -> list[str]:
    errors: list[str] = []
    if data.get("template_only"):
        errors.append("template_only records cannot certify a current task")
        return errors
    for key in REQUIRED:
        if key not in data or data[key] in (None, "", []):
            errors.append(f"missing {key}")
    pid = data.get("planning_id") or ""
    if pid and not str(pid).startswith("VG-"):
        errors.append("planning_id must stay a VG planning ID")
    if data.get("task_id"):
        errors.append("task_id must stay null; do not mint TASK numbers")
    commands = data.get("commands") or []
    if isinstance(commands, list):
        for i, cmd in enumerate(commands):
            if not isinstance(cmd, dict) or not cmd.get("run"):
                errors.append(f"commands[{i}] needs run")
            elif cmd.get("exit_code") is None:
                errors.append(f"commands[{i}] needs exit_code")
    assertions = data.get("assertions") or []
    if isinstance(assertions, list) and not assertions:
        errors.append("assertions empty")
    neg = data.get("negative_control") or {}
    if isinstance(neg, dict):
        if not neg.get("name"):
            errors.append("negative_control.name missing")
        if neg.get("observed_result") in (None, "", "NOT_RUN"):
            errors.append("negative_control was not run")
    arts = data.get("artifacts") or []
    if not arts:
        errors.append("no artifacts")
    screenshot_ok = False
    for i, art in enumerate(arts if isinstance(arts, list) else []):
        if not isinstance(art, dict):
            errors.append(f"artifacts[{i}] not an object")
            continue
        path = art.get("path")
        kind = (art.get("kind") or "").lower()
        sha = art.get("sha256") or art.get("content_hash")
        cmd = art.get("produced_by")
        if not path:
            errors.append(f"artifacts[{i}] missing path")
        if not sha and not cmd:
            errors.append(f"artifacts[{i}] has no provenance (sha256/produced_by)")
        if kind in ("screenshot", "png", "bmp", "capture") or (
            isinstance(path, str) and path.lower().endswith((".png", ".bmp"))
        ):
            if sha or cmd:
                screenshot_ok = True
            else:
                errors.append("screenshot without provenance cannot certify")
        if repo is not None and path:
            full = repo / path
            if full.is_file() and sha:
                actual = _sha256_file(full)
                if actual.lower() != str(sha).lower():
                    errors.append(f"artifacts[{i}] sha256 mismatch")
    if arts and not screenshot_ok:
        # A command log alone is allowed; only fail if every visual is naked.
        pass
    return errors


def load_and_validate(path: Path, repo: Path | None = None) -> list[str]:
    data = json.loads(path.read_text(encoding="utf-8"))
    return validate_manifest(data, repo=repo)


def file_sha256(path: Path) -> str:
    return _sha256_file(path)


if __name__ == "__main__":
    import sys

    target = Path(sys.argv[1]) if len(sys.argv) > 1 else None
    if target is None:
        raise SystemExit("usage: evidence_manifest.py <manifest.json>")
    repo = Path(__file__).resolve().parents[4]
    errs = load_and_validate(target, repo=repo)
    if errs:
        print("FAIL")
        for e in errs:
            print(" ", e)
        raise SystemExit(1)
    print("PASS")
