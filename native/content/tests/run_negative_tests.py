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


def run_validator(root):
    completed = subprocess.run(
        [sys.executable, str(VALIDATOR), "--root", str(root)],
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

    cases = [
        ("unknown_visual_role", "zones", lambda d: d["items"][0]["visual_roles"].__setitem__("floor", "terrain.lava"), ["E_UNKNOWN_ROLE"]),
        ("unknown_visual_slot", "zones", lambda d: d["items"][0]["visual_roles"].__setitem__("turrets", "terrain.floor"), ["E_UNKNOWN_SLOT"]),
        ("duplicate_zone_id", "zones", lambda d: d["items"][1].__setitem__("id", d["items"][0]["id"]), ["E_DUPLICATE_ID"]),
        ("duplicate_encounter_id", "encounters", lambda d: d["items"][1].__setitem__("id", d["items"][0]["id"]), ["E_DUPLICATE_ID"]),
        ("cross_collection_id_collision", "zones", lambda d: d["items"][0].__setitem__("id", "example-encounter-one"), ["E_DUPLICATE_ID"]),
        ("exit_to_unknown_zone", "zones", lambda d: d["items"][0]["exits"][0].__setitem__("to", "example-nowhere"), ["E_UNKNOWN_ZONE_REF"]),
        ("encounter_references_unknown_zone", "encounters", lambda d: d["items"][0].__setitem__("zone", "example-nowhere"), ["E_UNKNOWN_ZONE_REF"]),
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
