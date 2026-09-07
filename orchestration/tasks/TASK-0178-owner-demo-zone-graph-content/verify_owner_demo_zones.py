#!/usr/bin/env python3
"""TASK-0178 validator for owner_demo_zones.json."""

import json
import re
import sys
from collections import deque
from pathlib import Path

SCHEMA_VERSION = 1
KIND = "owner_demo_zone_graph"
ID_PATTERN = re.compile(r"^[a-z][a-z0-9]*(-[a-z0-9]+)*$")
ZONE_KINDS = frozenset({"prologue", "town", "combat"})
ENCOUNTER_FAMILIES = frozenset({"elite", "skirmish", "warden"})
TEMPLATES = frozenset({"crypt", "dungeon", "grove", "marsh", "wilds"})
LAYOUTS = frozenset({"clearings", "gauntlet", "warren"})


class Diags:
    def __init__(self):
        self.errors = []

    def error(self, path, code, message):
        self.errors.append((path, code, message))

    def sorted(self):
        return sorted(self.errors)


def load_json(path):
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def validate_identifier(diags, path, value):
    if not isinstance(value, str) or not ID_PATTERN.match(value):
        diags.error(path, "E_ID_FORMAT", "invalid identifier")
        return False
    if len(value) > 64:
        diags.error(path, "E_ID_FORMAT", "identifier too long")
        return False
    return True


def gate_connects(node, target):
    for gate in node.get("gates", []):
        if isinstance(gate, dict) and gate.get("to") == target:
            return True
    return False


def validate_zones(doc, diags, town_doc=None):
    if doc.get("schema_version") != SCHEMA_VERSION:
        diags.error("schema_version", "E_SCHEMA_VERSION", "unexpected schema version")
    if doc.get("kind") != KIND:
        diags.error("kind", "E_FILE_KIND", "unexpected envelope kind")

    nodes = doc.get("nodes")
    if not isinstance(nodes, list) or not nodes:
        diags.error("nodes", "E_MISSING_FIELD", "nodes required")
        return

    node_by_id = {}
    gate_ids = set()
    for index, node in enumerate(nodes):
        base = f"nodes[{index}]"
        if not isinstance(node, dict):
            diags.error(base, "E_BAD_TYPE", "node must be object")
            continue
        nid = node.get("id")
        if not validate_identifier(diags, f"{base}.id", nid):
            continue
        if nid in node_by_id:
            diags.error(f"{base}.id", "E_DUPLICATE_ID", "duplicate node id")
        node_by_id[nid] = node

        zone_kind = node.get("zone_kind")
        if zone_kind not in ZONE_KINDS:
            diags.error(f"{base}.zone_kind", "E_UNKNOWN_ZONE_KIND", f"bad zone_kind {zone_kind!r}")

        display = node.get("display_name")
        if not isinstance(display, str) or not display.strip():
            diags.error(f"{base}.display_name", "E_NAME_LENGTH", "display_name required")

        template = node.get("template_id")
        if template not in TEMPLATES:
            diags.error(f"{base}.template_id", "E_UNKNOWN_TEMPLATE", f"bad template {template!r}")
        layout = node.get("layout")
        if layout not in LAYOUTS:
            diags.error(f"{base}.layout", "E_UNKNOWN_LAYOUT", f"bad layout {layout!r}")

        seed = node.get("seed")
        if not isinstance(seed, int) or seed < 0:
            diags.error(f"{base}.seed", "E_BAD_TYPE", "seed must be non-negative int")
        if zone_kind in {"combat", "prologue"} and seed == 0:
            diags.error(f"{base}.seed", "E_MISSING_SEED", "generated zones need non-zero seed")

        allow_fresh = node.get("allow_fresh")
        lifetime = node.get("lifetime_ticks")
        if not isinstance(allow_fresh, bool):
            diags.error(f"{base}.allow_fresh", "E_BAD_TYPE", "allow_fresh must be bool")
        if not isinstance(lifetime, int) or lifetime < 0:
            diags.error(f"{base}.lifetime_ticks", "E_BAD_TYPE", "lifetime_ticks must be int >= 0")
        if zone_kind == "town" and allow_fresh:
            diags.error(f"{base}.allow_fresh", "E_TOWN_FRESH", "town cannot allow fresh instances")
        if zone_kind == "combat" and not allow_fresh:
            diags.error(f"{base}.allow_fresh", "E_COMBAT_FRESH", "combat zones must allow fresh")
        if zone_kind == "combat" and lifetime == 0:
            diags.error(f"{base}.lifetime_ticks", "E_MISSING_LIFETIME", "combat zones need lifetime")

        family = node.get("encounter_family")
        if zone_kind in {"combat", "prologue"}:
            if family not in ENCOUNTER_FAMILIES:
                diags.error(f"{base}.encounter_family", "E_UNKNOWN_FAMILY", "encounter family required")

        landmark = node.get("landmark")
        if not isinstance(landmark, str) or not landmark.strip():
            diags.error(f"{base}.landmark", "E_MISSING_FIELD", "landmark required")

        gates = node.get("gates")
        if not isinstance(gates, list):
            diags.error(f"{base}.gates", "E_BAD_TYPE", "gates must be array")
            continue
        for gate_index, gate in enumerate(gates):
            gbase = f"{base}.gates[{gate_index}]"
            if not isinstance(gate, dict):
                diags.error(gbase, "E_BAD_TYPE", "gate must be object")
                continue
            gid = gate.get("id")
            if not isinstance(gid, int) or gid <= 0:
                diags.error(f"{gbase}.id", "E_BAD_TYPE", "gate id must be positive int")
            elif gid in gate_ids:
                diags.error(f"{gbase}.id", "E_DUPLICATE_GATE_ID", "duplicate gate id")
            else:
                gate_ids.add(gid)
            label = gate.get("label")
            if not isinstance(label, str) or not label.strip():
                diags.error(f"{gbase}.label", "E_MISSING_FIELD", "gate label required")
            to_zone = gate.get("to")
            if validate_identifier(diags, f"{gbase}.to", to_zone):
                if to_zone == nid:
                    diags.error(f"{gbase}.to", "E_SELF_GATE", "gate cannot target self")
            pos = gate.get("position")
            if not isinstance(pos, dict):
                diags.error(f"{gbase}.position", "E_BAD_TYPE", "gate position required")
            else:
                for key in ("x", "y"):
                    if not isinstance(pos.get(key), int) or pos[key] < 0:
                        diags.error(f"{gbase}.position.{key}", "E_BAD_TYPE", "bad coordinate")

    prologue_id = doc.get("prologue_id")
    town_id = doc.get("town_id")
    if not validate_identifier(diags, "prologue_id", prologue_id):
        prologue_id = None
    if not validate_identifier(diags, "town_id", town_id):
        town_id = None
    if prologue_id and prologue_id not in node_by_id:
        diags.error("prologue_id", "E_UNKNOWN_NODE", "prologue node missing")
    if town_id and town_id not in node_by_id:
        diags.error("town_id", "E_UNKNOWN_NODE", "town node missing")
    if prologue_id and node_by_id.get(prologue_id, {}).get("classification") != "village_defense":
        diags.error("prologue_id", "E_BAD_CLASSIFICATION", "prologue must be village_defense")

    for gate_target in node_by_id.values():
        for gate in gate_target.get("gates", []):
            if not isinstance(gate, dict):
                continue
            target = gate.get("to")
            if isinstance(target, str) and target not in node_by_id:
                diags.error("gates", "E_UNKNOWN_ZONE_REF", f"gate to missing node {target}")

    main_route = doc.get("main_route")
    if not isinstance(main_route, list) or len(main_route) < 4:
        diags.error("main_route", "E_MAIN_ROUTE", "main_route needs prologue,town,two combat zones")
    else:
        for index, step in enumerate(main_route):
            if not validate_identifier(diags, f"main_route[{index}]", step):
                continue
            if step not in node_by_id:
                diags.error(f"main_route[{index}]", "E_UNKNOWN_NODE", "main_route node missing")
        for index in range(len(main_route) - 1):
            src = main_route[index]
            dst = main_route[index + 1]
            if src in node_by_id and not gate_connects(node_by_id[src], dst):
                diags.error(
                    "main_route",
                    "E_DISCONNECTED_ROUTE",
                    f"no gate from {src} to {dst}",
                )

        combat_on_route = [
            step
            for step in main_route
            if node_by_id.get(step, {}).get("zone_kind") == "combat"
        ]
        if len(combat_on_route) < 2:
            diags.error("main_route", "E_COMBAT_COUNT", "main route needs two combat zones")

        for step in combat_on_route:
            node = node_by_id.get(step, {})
            if not node.get("boss"):
                diags.error(f"nodes[{step}].boss", "E_MISSING_BOSS", "main combat needs boss")

    optional = doc.get("optional_branches")
    if not isinstance(optional, list) or not optional:
        diags.error("optional_branches", "E_MISSING_BRANCH", "optional branch required")
    else:
        for index, branch in enumerate(optional):
            base = f"optional_branches[{index}]"
            if not isinstance(branch, dict):
                diags.error(base, "E_BAD_TYPE", "branch must be object")
                continue
            src = branch.get("from")
            dst = branch.get("to")
            if validate_identifier(diags, f"{base}.from", src) and src not in node_by_id:
                diags.error(f"{base}.from", "E_UNKNOWN_NODE", "branch source missing")
            if validate_identifier(diags, f"{base}.to", dst) and dst not in node_by_id:
                diags.error(f"{base}.to", "E_UNKNOWN_NODE", "branch target missing")
            if not branch.get("dead_end"):
                diags.error(f"{base}.dead_end", "E_BRANCH_DEAD_END", "branch must be dead_end")
            label = branch.get("gate_label")
            if not isinstance(label, str) or not label.strip():
                diags.error(f"{base}.gate_label", "E_MISSING_FIELD", "gate_label required")
            if src in node_by_id and dst in node_by_id:
                if not gate_connects(node_by_id[src], dst):
                    diags.error(base, "E_DISCONNECTED_BRANCH", "branch gate missing")
                target_node = node_by_id[dst]
                if not target_node.get("dead_end"):
                    diags.error(f"{base}.to", "E_BRANCH_DEAD_END", "branch target not dead_end")
                outbound = [
                    g.get("to")
                    for g in target_node.get("gates", [])
                    if isinstance(g, dict)
                ]
                if len(outbound) != 1 or outbound[0] != src:
                    diags.error(f"{base}.to", "E_BRANCH_EXIT", "dead-end must return only to branch source")

    if prologue_id and prologue_id in node_by_id:
        seen = set()
        queue = deque([prologue_id])
        while queue:
            current = queue.popleft()
            if current in seen:
                continue
            seen.add(current)
            for gate in node_by_id[current].get("gates", []):
                if isinstance(gate, dict) and isinstance(gate.get("to"), str):
                    queue.append(gate["to"])
        for nid in node_by_id:
            if nid not in seen:
                diags.error("nodes", "W_UNREACHABLE_NODE", f"unreachable node {nid}")

    if town_doc:
        town_exits = town_doc.get("exits", [])
        if isinstance(town_exits, list) and town_id in node_by_id:
            town_node = node_by_id[town_id]
            for exit_def in town_exits:
                if not isinstance(exit_def, dict):
                    continue
                to_zone = exit_def.get("to_zone")
                label = exit_def.get("label")
                if not gate_connects(town_node, to_zone):
                    diags.error("town_sync", "E_TOWN_EXIT_MISMATCH", f"town exit missing gate to {to_zone}")
                else:
                    for gate in town_node.get("gates", []):
                        if gate.get("to") == to_zone and gate.get("label") != label:
                            diags.error(
                                "town_sync",
                                "E_TOWN_LABEL_MISMATCH",
                                f"label mismatch for {to_zone}",
                            )


def main(argv):
    root = Path(__file__).resolve().parents[3]
    zones_path = root / "native" / "content" / "seeds" / "owner_demo_zones.json"
    town_path = root / "native" / "content" / "seeds" / "owner_demo_town.json"
    negative = "--negative" in argv

    zones_doc = load_json(zones_path)
    town_doc = load_json(town_path) if town_path.is_file() else None

    if negative:
        for node in zones_doc.get("nodes", []):
            if node.get("id") == "owner-demo-thornward":
                node["boss"] = False
        diags = Diags()
        validate_zones(zones_doc, diags, town_doc)
        if not diags.errors:
            print("FAIL negative control did not fail")
            return 1
        print("PASS negative_control")
        return 0

    diags = Diags()
    validate_zones(zones_doc, diags, town_doc)
    if diags.errors:
        for path, code, message in diags.sorted():
            print(f"FAIL {path} {code}: {message}")
        return 1
    node_count = len(zones_doc.get("nodes", []))
    combat_count = sum(
        1 for n in zones_doc.get("nodes", []) if n.get("zone_kind") == "combat"
    )
    print(f"OK nodes={node_count} combat={combat_count} route={len(zones_doc.get('main_route', []))}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
