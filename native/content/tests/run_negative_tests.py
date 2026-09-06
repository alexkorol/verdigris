import copy
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

CONTENT_ROOT = Path(__file__).resolve().parents[1]
VALIDATOR = CONTENT_ROOT / "validate_content.py"

SEED_FILE_NAMES = {"zones": "seeds/zones.json", "encounters": "seeds/encounters.json"}


def load_seeds():
    seeds = {}
    for kind, rel_path in sorted(SEED_FILE_NAMES.items()):
        with open(CONTENT_ROOT / rel_path, "r", encoding="utf-8") as handle:
            seeds[kind] = json.load(handle)
    return seeds


def run_validator(root, extra_args=()):
    completed = subprocess.run(
        [sys.executable, str(VALIDATOR), "--root", str(root)] + list(extra_args),
        capture_output=True,
        encoding="utf-8",
    )
    return completed.returncode, completed.stdout, completed.stderr


def make_temp_root():
    temp_root = Path(tempfile.mkdtemp(prefix="verdigris-content-negative-"))
    shutil.copyfile(CONTENT_ROOT / "schema.json", temp_root / "schema.json")
    (temp_root / "seeds").mkdir()
    return temp_root


def write_seed(temp_root, kind, payload):
    target = temp_root / SEED_FILE_NAMES[kind]
    if isinstance(payload, str):
        target.write_text(payload, encoding="utf-8")
        return
    target.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def build_cases(seeds):
    def zones():
        return copy.deepcopy(seeds["zones"])

    def encounters():
        return copy.deepcopy(seeds["encounters"])

    def append_duplicate_exit(doc):
        edge = copy.deepcopy(doc["items"][0]["exits"][0])
        doc["items"][0]["exits"].append(edge)

    def isolate_mire(doc):
        for item in doc["items"]:
            item["exits"] = [edge for edge in item["exits"] if edge.get("to") != "example-mire-one"]

    cases = [
        ("unknown_visual_role", "zones", lambda d: d["items"][0]["visual_roles"].__setitem__("floor", "terrain.lava"), ["E_UNKNOWN_ROLE"]),
        ("unknown_visual_slot", "zones", lambda d: d["items"][0]["visual_roles"].__setitem__("turrets", "terrain.floor"), ["E_UNKNOWN_SLOT"]),
        ("duplicate_zone_id", "zones", lambda d: d["items"][1].__setitem__("id", d["items"][0]["id"]), ["E_DUPLICATE_ID"]),
        ("duplicate_encounter_id", "encounters", lambda d: d["items"][1].__setitem__("id", d["items"][0]["id"]), ["E_DUPLICATE_ID"]),
        ("cross_collection_id_collision", "zones", lambda d: d["items"][0].__setitem__("id", "example-encounter-one"), ["E_DUPLICATE_ID"]),
        ("exit_to_unknown_zone", "zones", lambda d: d["items"][0]["exits"][0].__setitem__("to", "example-nowhere"), ["E_UNKNOWN_ZONE_REF"]),
        ("encounter_references_unknown_zone", "encounters", lambda d: d["items"][0].__setitem__("zone", "example-nowhere"), ["E_UNKNOWN_ZONE_REF"]),
        ("exit_to_encounter_id_type_mismatch", "zones", lambda d: d["items"][0]["exits"][0].__setitem__("to", "example-encounter-two"), ["E_REFERENCE_TYPE_MISMATCH"]),
        ("encounter_zone_type_mismatch", "encounters", lambda d: d["items"][0].__setitem__("zone", "example-encounter-two"), ["E_REFERENCE_TYPE_MISMATCH"]),
        ("unreachable_encounter_zone", "zones", isolate_mire, ["E_UNREACHABLE_ENCOUNTER"]),
        ("string_schema_version_linkage", "encounters", lambda d: d.__setitem__("schema_version", "1"), ["E_SCHEMA_VERSION"]),
        ("unknown_zone_template", "zones", lambda d: d["items"][0].__setitem__("template_id", "volcano"), ["E_UNKNOWN_TEMPLATE"]),
        ("unknown_zone_layout", "zones", lambda d: d["items"][0].__setitem__("layout", "labyrinth"), ["E_UNKNOWN_LAYOUT"]),
        ("unknown_exit_kind", "zones", lambda d: d["items"][0]["exits"][0].__setitem__("kind", "teleport"), ["E_UNKNOWN_EXIT_KIND"]),
        ("unknown_encounter_family", "encounters", lambda d: d["items"][0].__setitem__("family", "horde"), ["E_UNKNOWN_FAMILY"]),
        ("bad_seed_schema_version", "zones", lambda d: d.__setitem__("schema_version", 2), ["E_SCHEMA_VERSION"]),
        ("wrong_envelope_kind", "zones", lambda d: d.__setitem__("kind", "encounter"), ["E_FILE_KIND"]),
        ("missing_required_field", "zones", lambda d: (d["items"][0].pop("display_name"), None)[1], ["E_MISSING_FIELD"]),
        ("unknown_item_field", "zones", lambda d: d["items"][0].__setitem__("flavor", "sour"), ["E_UNKNOWN_FIELD"]),
        ("malformed_identifier", "zones", lambda d: d["items"][0].__setitem__("id", "Bad_Id"), ["E_ID_FORMAT"]),
        ("empty_display_name", "zones", lambda d: d["items"][0].__setitem__("display_name", ""), ["E_NAME_LENGTH"]),
        ("duplicate_exit_edge", "zones", append_duplicate_exit, ["E_DUPLICATE_EXIT"]),
        ("unknown_envelope_field", "zones", lambda d: d.__setitem__("extra", True), ["E_UNKNOWN_FIELD"]),
        ("malformed_json", "zones", "{ this is not json", ["E_JSON_PARSE"]),
    ]
    return cases


def build_schema_cases():
    cases = [
        ("unknown_schema_top_level_field", lambda d: d.__setitem__("future_section", {}), ["E_UNKNOWN_FIELD"]),
        ("unknown_identifier_rule_key", lambda d: d["identifier_rules"].__setitem__("min_length", 1), ["E_UNKNOWN_FIELD"]),
        ("unknown_display_name_rule_key", lambda d: d["display_name_rules"].__setitem__("pattern", ".*"), ["E_UNKNOWN_FIELD"]),
        ("unknown_entity_key", lambda d: d["entities"]["zone"].__setitem__("extra_policy", True), ["E_UNKNOWN_FIELD"]),
        ("unknown_composite_key", lambda d: d["composite_types"]["exit"].__setitem__("extra", 1), ["E_UNKNOWN_FIELD"]),
        ("unknown_slot_role_map_key", lambda d: d["composite_types"]["slot_role_map"].__setitem__("extra", 1), ["E_UNKNOWN_FIELD"]),
    ]
    return cases


def load_schema_doc():
    with open(CONTENT_ROOT / "schema.json", "r", encoding="utf-8") as handle:
        return json.load(handle)


def write_schema(temp_root, doc):
    (temp_root / "schema.json").write_text(json.dumps(doc, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def pack_zone(zone_id, exits_to, extra=None):
    item = {
        "id": zone_id,
        "display_name": "Pack Zone {}".format(zone_id),
        "template_id": "dungeon",
        "layout": "warren",
        "exits": [{"to": target, "kind": "walk"} for target in exits_to],
        "visual_roles": {"floor": "terrain.floor", "walls": "terrain.blocking"},
    }
    if extra:
        item.update(extra)
    return item


def pack_encounter(encounter_id, zone_ref):
    return {
        "id": encounter_id,
        "display_name": "Pack Encounter {}".format(encounter_id),
        "family": "skirmish",
        "zone": zone_ref,
        "visual_roles": {"actors": "actor.combatant"},
    }


def pack_envelope(kind, items):
    return {"schema_version": 1, "kind": kind, "items": items}


def build_pack_cases(seeds):
    def positive_seed_override(doc):
        doc["items"][4]["exits"].append({"to": "pack-alpha", "kind": "walk"})

    return [
        {
            "name": "pack_positive_merge",
            "seed_override": positive_seed_override,
            "packs": {
                "packs/pack_a_zones.json": pack_envelope(
                    "zone",
                    [
                        pack_zone("pack-alpha", ["example-mire-one", "pack-beta"]),
                        pack_zone("pack-beta", ["pack-alpha"]),
                    ],
                ),
                "packs/pack_b_encounters.json": pack_envelope(
                    "encounter", [pack_encounter("pack-encounter-one", "pack-beta")]
                ),
            },
            "args": lambda tr: [
                "--pack", "zone={}".format(tr / "packs" / "pack_a_zones.json"),
                "--pack", "encounter={}".format(tr / "packs" / "pack_b_encounters.json"),
            ],
            "rc": 0,
            "codes": [],
            "must_contain": ["OK", "zone=7", "encounter=4"],
            "must_not_contain": ["WARNING", "ERROR"],
        },
        {
            # VG-TOOLS-001 negative control: two unrelated packs silently
            # reusing one runtime id must be rejected when loaded together.
            "name": "pack_negative_control_duplicate_id",
            "seed_override": None,
            "packs": {
                "packs/pack_a_zones.json": pack_envelope("zone", [pack_zone("pack-shared-zone", ["example-mire-one"])]),
                "packs/pack_b_zones.json": pack_envelope("zone", [pack_zone("pack-shared-zone", ["example-mire-one"])]),
            },
            "args": lambda tr: [
                "--pack", "zone={}".format(tr / "packs" / "pack_a_zones.json"),
                "--pack", "zone={}".format(tr / "packs" / "pack_b_zones.json"),
            ],
            "rc": 1,
            "codes": ["E_DUPLICATE_ID"],
            "must_contain": ["packs/pack_a_zones.json", "packs/pack_b_zones.json"],
            "must_not_contain": [],
        },
        {
            "name": "pack_collides_with_declared_seed",
            "seed_override": None,
            "packs": {
                "packs/pack_a_zones.json": pack_envelope("zone", [pack_zone("example-hall-one", ["example-mire-one"])]),
            },
            "args": lambda tr: ["--pack", "zone={}".format(tr / "packs" / "pack_a_zones.json")],
            "rc": 1,
            "codes": ["E_DUPLICATE_ID"],
            "must_contain": ["seeds/zones.json", "packs/pack_a_zones.json"],
            "must_not_contain": [],
        },
        {
            "name": "pack_dangling_reference",
            "seed_override": None,
            "packs": {
                "packs/pack_a_zones.json": pack_envelope("zone", [pack_zone("pack-alpha", ["pack-nowhere"])]),
            },
            "args": lambda tr: ["--pack", "zone={}".format(tr / "packs" / "pack_a_zones.json")],
            "rc": 1,
            "codes": ["E_UNKNOWN_ZONE_REF"],
            "must_contain": ["packs/pack_a_zones.json:items[0].exits[0].to"],
            "must_not_contain": [],
        },
        {
            "name": "pack_unknown_field",
            "seed_override": None,
            "packs": {
                "packs/pack_a_zones.json": pack_envelope(
                    "zone", [pack_zone("pack-alpha", ["example-mire-one"], extra={"flavor": "sour"})]
                ),
            },
            "args": lambda tr: ["--pack", "zone={}".format(tr / "packs" / "pack_a_zones.json")],
            "rc": 1,
            "codes": ["E_UNKNOWN_FIELD"],
            "must_contain": ["packs/pack_a_zones.json:items[0].flavor"],
            "must_not_contain": [],
        },
        {
            "name": "pack_wrong_kind",
            "seed_override": None,
            "packs": {
                "packs/pack_a_zones.json": pack_envelope(
                    "encounter", [pack_encounter("pack-encounter-one", "example-mire-one")]
                ),
            },
            "args": lambda tr: ["--pack", "zone={}".format(tr / "packs" / "pack_a_zones.json")],
            "rc": 1,
            "codes": ["E_FILE_KIND"],
            "must_contain": ["packs/pack_a_zones.json"],
            "must_not_contain": [],
        },
        {
            "name": "pack_undeclared_kind_usage",
            "seed_override": None,
            "packs": {},
            "args": lambda tr: ["--pack", "bogus={}".format(tr / "packs" / "pack_a_zones.json")],
            "rc": 2,
            "codes": [],
            "must_contain": ["USAGE ERROR"],
            "must_not_contain": [],
        },
        {
            "name": "pack_reloading_declared_file_usage",
            "seed_override": None,
            "packs": {},
            "args": lambda tr: ["--pack", "zone={}".format(tr / "seeds" / "zones.json")],
            "rc": 2,
            "codes": [],
            "must_contain": ["USAGE ERROR"],
            "must_not_contain": [],
        },
    ]


def main():
    failures = []
    checks = 0

    seeds = load_seeds()

    rc, out, err = run_validator(CONTENT_ROOT)
    checks += 1
    if rc != 0:
        failures.append("positive_control: expected exit 0, got {}: {}{}".format(rc, out, err))
    elif "OK" not in out:
        failures.append("positive_control: missing OK summary in stdout: {}".format(out))
    elif "zone=5" not in out or "encounter=3" not in out:
        failures.append("positive_control: unexpected counts line: {}".format(out.strip()))
    else:
        print("PASS positive_control")

    rc2, out2, _ = run_validator(CONTENT_ROOT)
    checks += 1
    if out != out2 or rc != rc2:
        failures.append("determinism_double_run: outputs differ between identical runs")
    else:
        print("PASS determinism_double_run")

    for name, kind, mutate, expected_codes in build_cases(seeds):
        checks += 1
        temp_root = make_temp_root()
        try:
            if isinstance(mutate, str):
                payload = mutate
            else:
                doc = copy.deepcopy(seeds[kind])
                outcome = mutate(doc)
                payload = outcome if isinstance(outcome, dict) else doc
            for other_kind in sorted(SEED_FILE_NAMES.keys()):
                if other_kind != kind:
                    write_seed(temp_root, other_kind, copy.deepcopy(seeds[other_kind]))
            write_seed(temp_root, kind, payload)
            rc, out, err = run_validator(temp_root)
            problems = []
            if rc != 1:
                problems.append("expected exit 1, got {}".format(rc))
            for code in expected_codes:
                if code not in out:
                    problems.append("missing expected diagnostic {}".format(code))
            rc_again, out_again, _ = run_validator(temp_root)
            if out_again != out or rc_again != rc:
                problems.append("nondeterministic diagnostics across repeated runs")
            if problems:
                failures.append("{}: {}; validator output:\n{}".format(name, "; ".join(problems), out + err))
            else:
                print("PASS {} ({})".format(name, ", ".join(expected_codes)))
        finally:
            shutil.rmtree(temp_root, ignore_errors=True)

    for name, mutate_schema, expected_codes in build_schema_cases():
        checks += 1
        temp_root = make_temp_root()
        try:
            schema_doc = load_schema_doc()
            mutate_schema(schema_doc)
            write_schema(temp_root, schema_doc)
            for other_kind in sorted(SEED_FILE_NAMES.keys()):
                write_seed(temp_root, other_kind, copy.deepcopy(seeds[other_kind]))
            rc, out, err = run_validator(temp_root)
            problems = []
            if rc != 1:
                problems.append("expected exit 1, got {}".format(rc))
            for code in expected_codes:
                if code not in out:
                    problems.append("missing expected diagnostic {}".format(code))
            rc_again, out_again, _ = run_validator(temp_root)
            if out_again != out or rc_again != rc:
                problems.append("nondeterministic diagnostics across repeated runs")
            if problems:
                failures.append("{}: {}; validator output:\n{}".format(name, "; ".join(problems), out + err))
            else:
                print("PASS {} ({})".format(name, ", ".join(expected_codes)))
        finally:
            shutil.rmtree(temp_root, ignore_errors=True)

    for case in build_pack_cases(seeds):
        checks += 1
        temp_root = make_temp_root()
        try:
            zones_doc = copy.deepcopy(seeds["zones"])
            if case["seed_override"] is not None:
                case["seed_override"](zones_doc)
            write_seed(temp_root, "zones", zones_doc)
            write_seed(temp_root, "encounters", copy.deepcopy(seeds["encounters"]))
            for rel_path, payload in sorted(case["packs"].items()):
                target = temp_root / rel_path
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
            extra_args = case["args"](temp_root)
            rc, out, err = run_validator(temp_root, extra_args)
            problems = []
            if rc != case["rc"]:
                problems.append("expected exit {}, got {}".format(case["rc"], rc))
            for code in case["codes"]:
                if code not in out:
                    problems.append("missing expected diagnostic {}".format(code))
            for needle in case["must_contain"]:
                if needle not in out:
                    problems.append("missing expected output fragment {!r}".format(needle))
            for needle in case["must_not_contain"]:
                if needle in out:
                    problems.append("unexpected output fragment {!r}".format(needle))
            rc_again, out_again, _ = run_validator(temp_root, extra_args)
            if out_again != out or rc_again != rc:
                problems.append("nondeterministic diagnostics across repeated runs")
            if problems:
                failures.append("{}: {}; validator output:\n{}".format(case["name"], "; ".join(problems), out + err))
            else:
                label = ", ".join(case["codes"]) if case["codes"] else "exit {}".format(case["rc"])
                print("PASS {} ({})".format(case["name"], label))
        finally:
            shutil.rmtree(temp_root, ignore_errors=True)

    checks += 1
    temp_root = make_temp_root()
    try:
        write_seed(temp_root, "zones", copy.deepcopy(seeds["zones"]))
        rc, out, err = run_validator(temp_root)
        if rc != 1 or "E_FILE_MISSING" not in out:
            failures.append("missing_seed_file: expected exit 1 with E_FILE_MISSING, got {}: {}{}".format(rc, out, err))
        else:
            print("PASS missing_seed_file (E_FILE_MISSING)")
    finally:
        shutil.rmtree(temp_root, ignore_errors=True)

    print("checks={} failures={}".format(checks, len(failures)))
    if failures:
        for failure in failures:
            print("FAIL {}".format(failure))
        return 1
    print("NEGATIVE SUITE PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
