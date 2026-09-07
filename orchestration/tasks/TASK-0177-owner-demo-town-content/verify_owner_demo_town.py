#!/usr/bin/env python3
"""TASK-0177 validator for owner_demo_town.json."""

import json
import re
import sys
from pathlib import Path

SCHEMA_VERSION = 1
KIND = "owner_demo_town"
CLASSIFICATION = "non_combat_settlement"
ID_PATTERN = re.compile(r"^[a-z][a-z0-9]*(-[a-z0-9]+)*$")
REQUIRED_NPC_ROLES = frozenset(
    {"elder", "weapons_tools_trainer", "armor_ritual_merchant"}
)
REQUIRED_FACILITY_KINDS = frozenset(
    {
        "shop",
        "storage",
        "guidance",
        "expedition_access",
        "house_investment",
    }
)
ALLOWED_NPC_ACTIONS = frozenset(
    {"talk", "trade", "bank", "examine", "guidance", "expedition"}
)
ALLOWED_EXIT_KINDS = frozenset({"gate", "walk", "stairs"})


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


def validate_position(diags, path, value):
    if not isinstance(value, dict):
        diags.error(path, "E_BAD_TYPE", "position must be object")
        return False
    for key in ("x", "y"):
        if key not in value:
            diags.error(f"{path}.{key}", "E_MISSING_FIELD", "missing coordinate")
            continue
        if not isinstance(value[key], int) or value[key] < 0:
            diags.error(f"{path}.{key}", "E_BAD_TYPE", "coordinate must be non-negative int")
    return True


def validate_town(doc, diags, zones_path=None):
    if doc.get("schema_version") != SCHEMA_VERSION:
        diags.error("schema_version", "E_SCHEMA_VERSION", "unexpected schema version")
    if doc.get("kind") != KIND:
        diags.error("kind", "E_FILE_KIND", "unexpected envelope kind")
    if doc.get("classification") != CLASSIFICATION:
        diags.error("classification", "E_BAD_CLASSIFICATION", "town must be non-combat")
    if not validate_identifier(diags, "id", doc.get("id")):
        return
    display = doc.get("display_name")
    if not isinstance(display, str) or not display.strip():
        diags.error("display_name", "E_NAME_LENGTH", "display_name required")
    crisis = doc.get("crisis_direction")
    if not isinstance(crisis, str) or len(crisis.strip()) < 12:
        diags.error("crisis_direction", "E_MISSING_FIELD", "crisis direction required")

    npcs = doc.get("npcs")
    if not isinstance(npcs, list) or not npcs:
        diags.error("npcs", "E_MISSING_FIELD", "npcs required")
        return

    npc_ids = set()
    roles_seen = set()
    positions = set()
    for index, npc in enumerate(npcs):
        base = f"npcs[{index}]"
        if not isinstance(npc, dict):
            diags.error(base, "E_BAD_TYPE", "npc must be object")
            continue
        nid = npc.get("id")
        if validate_identifier(diags, f"{base}.id", nid):
            if nid in npc_ids:
                diags.error(f"{base}.id", "E_DUPLICATE_ID", "duplicate npc id")
            npc_ids.add(nid)
        role = npc.get("role")
        if not isinstance(role, str):
            diags.error(f"{base}.role", "E_MISSING_FIELD", "role required")
        else:
            roles_seen.add(role)
        name = npc.get("display_name")
        if not isinstance(name, str) or not name.strip():
            diags.error(f"{base}.display_name", "E_NAME_LENGTH", "npc name required")
        validate_position(diags, f"{base}.position", npc.get("position"))
        pos = npc.get("position")
        if isinstance(pos, dict) and isinstance(pos.get("x"), int) and isinstance(pos.get("y"), int):
            key = (pos["x"], pos["y"])
            if key in positions:
                diags.error(f"{base}.position", "E_DUPLICATE_POSITION", "npc positions collide")
            positions.add(key)
        services = npc.get("services")
        if not isinstance(services, list) or not services:
            diags.error(f"{base}.services", "E_MISSING_FIELD", "npc services required")
        else:
            for svc_index, svc in enumerate(services):
                if not isinstance(svc, str):
                    diags.error(f"{base}.services[{svc_index}]", "E_BAD_TYPE", "service must be string")
        actions = npc.get("actions")
        if not isinstance(actions, list) or not actions:
            diags.error(f"{base}.actions", "E_MISSING_FIELD", "npc actions required")
        else:
            for act_index, action in enumerate(actions):
                if action not in ALLOWED_NPC_ACTIONS:
                    diags.error(
                        f"{base}.actions[{act_index}]",
                        "E_UNKNOWN_ACTION",
                        f"unknown action {action!r}",
                    )

    for role in REQUIRED_NPC_ROLES:
        if role not in roles_seen:
            diags.error("npcs", "E_MISSING_ROLE", f"missing npc role {role}")

    facilities = doc.get("facilities")
    if not isinstance(facilities, list) or not facilities:
        diags.error("facilities", "E_MISSING_FIELD", "facilities required")
        return

    facility_kinds = set()
    facility_ids = set()
    for index, facility in enumerate(facilities):
        base = f"facilities[{index}]"
        if not isinstance(facility, dict):
            diags.error(base, "E_BAD_TYPE", "facility must be object")
            continue
        fid = facility.get("id")
        if validate_identifier(diags, f"{base}.id", fid):
            if fid in facility_ids:
                diags.error(f"{base}.id", "E_DUPLICATE_ID", "duplicate facility id")
            facility_ids.add(fid)
        kind = facility.get("kind")
        if not isinstance(kind, str):
            diags.error(f"{base}.kind", "E_MISSING_FIELD", "facility kind required")
        else:
            facility_kinds.add(kind)
        anchor = facility.get("anchor_npc")
        if not validate_identifier(diags, f"{base}.anchor_npc", anchor):
            pass
        elif anchor not in npc_ids:
            diags.error(f"{base}.anchor_npc", "E_UNKNOWN_NPC_REF", "anchor npc not found")
        name = facility.get("display_name")
        if not isinstance(name, str) or not name.strip():
            diags.error(f"{base}.display_name", "E_NAME_LENGTH", "facility name required")

    for kind in REQUIRED_FACILITY_KINDS:
        if kind not in facility_kinds:
            diags.error("facilities", "E_MISSING_SERVICE", f"missing facility kind {kind}")

    exits = doc.get("exits")
    if not isinstance(exits, list) or not exits:
        diags.error("exits", "E_MISSING_FIELD", "exits required")
        return

    exit_ids = set()
    zone_refs = set()
    for index, exit_def in enumerate(exits):
        base = f"exits[{index}]"
        if not isinstance(exit_def, dict):
            diags.error(base, "E_BAD_TYPE", "exit must be object")
            continue
        eid = exit_def.get("id")
        if validate_identifier(diags, f"{base}.id", eid):
            if eid in exit_ids:
                diags.error(f"{base}.id", "E_DUPLICATE_ID", "duplicate exit id")
            exit_ids.add(eid)
        label = exit_def.get("label")
        if not isinstance(label, str) or not label.strip():
            diags.error(f"{base}.label", "E_MISSING_FIELD", "exit label required")
        to_zone = exit_def.get("to_zone")
        if validate_identifier(diags, f"{base}.to_zone", to_zone):
            zone_refs.add(to_zone)
        kind = exit_def.get("kind")
        if kind not in ALLOWED_EXIT_KINDS:
            diags.error(f"{base}.kind", "E_UNKNOWN_EXIT_KIND", f"unknown exit kind {kind!r}")
        validate_position(diags, f"{base}.position", exit_def.get("position"))

    if zones_path and Path(zones_path).is_file():
        zones_doc = load_json(zones_path)
        zone_ids = {node["id"] for node in zones_doc.get("nodes", []) if isinstance(node, dict)}
        for ref in sorted(zone_refs):
            if ref not in zone_ids:
                diags.error("exits", "E_UNKNOWN_ZONE_REF", f"exit references missing zone {ref}")


def main(argv):
    root = Path(__file__).resolve().parents[3]
    town_path = root / "native" / "content" / "seeds" / "owner_demo_town.json"
    zones_path = root / "native" / "content" / "seeds" / "owner_demo_zones.json"
    negative = "--negative" in argv

    if negative:
        doc = load_json(town_path)
        doc["npcs"] = [n for n in doc["npcs"] if n.get("role") != "elder"]
        diags = Diags()
        validate_town(doc, diags)
        if not diags.errors:
            print("FAIL negative control did not fail")
            return 1
        print("PASS negative_control")
        return 0

    diags = Diags()
    doc = load_json(town_path)
    validate_town(doc, diags, zones_path if zones_path.is_file() else None)
    if diags.errors:
        for path, code, message in diags.sorted():
            print(f"FAIL {path} {code}: {message}")
        return 1
    npc_count = len(doc.get("npcs", []))
    facility_count = len(doc.get("facilities", []))
    exit_count = len(doc.get("exits", []))
    print(
        f"OK town={doc.get('id')} npcs={npc_count} facilities={facility_count} exits={exit_count}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
