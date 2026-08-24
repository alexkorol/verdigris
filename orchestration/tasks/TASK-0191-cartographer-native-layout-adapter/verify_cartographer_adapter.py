#!/usr/bin/env python3
"""TASK-0191: verify owner_demo_zones seeds align with cartographer adapter."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
ZONES = ROOT / "native/content/seeds/owner_demo_zones.json"
MAPGEN = Path("Z:/Code/WIZARD/tools/cartographer/core/mapgen.js")

TEMPLATE_TO_MAPGEN = {
    "wilds": "wilds",
    "marsh": "wilds",
    "grove": "wilds",
    "dungeon": "dungeon",
    "crypt": "dungeon",
}

THEMES = {
    "wilds": {"forest", "moor", "swamp", "ash", "tundra"},
    "marsh": {"swamp"},
    "grove": {"forest"},
    "dungeon": {"crypt", "fortress", "sewer", "prison"},
    "crypt": {"crypt", "fortress", "sewer", "prison"},
}

LAYOUT_SPAWN_MIN = {
    "clearings": 4,
    "gauntlet": 6,
    "warren": 8,
}


def main() -> int:
    data = json.loads(ZONES.read_text(encoding="utf-8"))
    nodes = data["nodes"]
    ids = {node["id"] for node in nodes}

    for node in nodes:
        template_id = node["template_id"]
        layout = node["layout"]
        theme = node["theme"]
        if template_id not in TEMPLATE_TO_MAPGEN:
            print(f"FAIL: unknown template {template_id}")
            return 1
        if theme not in THEMES[template_id]:
            print(f"FAIL: theme {theme} not allowed for {template_id}")
            return 1
        if layout not in LAYOUT_SPAWN_MIN:
            print(f"FAIL: unknown layout {layout}")
            return 1
        for gate in node.get("gates", []):
            if gate["to"] not in ids:
                print(f"FAIL: gate to missing zone {gate['to']}")
                return 1

    combat = [
        n for n in nodes
        if n.get("classification") in {"generated_combat", "optional_branch", "village_defense"}
    ]
    if not MAPGEN.is_file():
        print("SKIP: WIZARD mapgen.js not found; graph checks only")
        print("TASK-0191 verify_cartographer_adapter: PASS (graph only)")
        return 0

    js = r"""
const MapGen = require(%s);
const zones = %s;
const templateToZone = %s;
const layoutMin = %s;
let failures = 0;
for (const node of zones) {
  const zone = templateToZone[node.template_id] || 'dungeon';
  const map = MapGen.generate({ zone, theme: node.theme, seed: node.seed });
  if (!map.entrance || !map.exit) { failures++; continue; }
  if (!map.spawns || map.spawns.length < layoutMin[node.layout]) failures++;
}
if (failures) { console.error('mapgen failures', failures); process.exit(1); }
console.log('mapgen connectivity OK for', zones.length, 'zones');
""" % (
        json.dumps(str(MAPGEN)),
        json.dumps(combat),
        json.dumps(TEMPLATE_TO_MAPGEN),
        json.dumps(LAYOUT_SPAWN_MIN),
    )

    proc = subprocess.run(
        ["node", "-e", js],
        capture_output=True,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        print(proc.stdout)
        print(proc.stderr)
        return proc.returncode

    print(proc.stdout.strip())
    print("TASK-0191 verify_cartographer_adapter: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
