import argparse
import json
import re
import sys
from pathlib import Path

SUPPORTED_SCHEMA_VERSION = 1
ENVELOPE_KEYS = ("items", "kind", "schema_version")

ENUM_CODE_BY_ENUM = {
    "encounter_family": "E_UNKNOWN_FAMILY",
    "exit_kind": "E_UNKNOWN_EXIT_KIND",
    "visual_role": "E_UNKNOWN_ROLE",
    "visual_slot": "E_UNKNOWN_SLOT",
    "zone_layout": "E_UNKNOWN_LAYOUT",
    "zone_template": "E_UNKNOWN_TEMPLATE",
}


def value_repr(value):
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


class Diagnostics:
    def __init__(self):
        self.errors = []
        self.warnings = []

    def error(self, file_name, path, code, message):
        self.errors.append((str(file_name), str(path), code, message))

    def warning(self, file_name, path, code, message):
        self.warnings.append((str(file_name), str(path), code, message))

    def sorted_errors(self):
        return sorted(self.errors)

    def sorted_warnings(self):
        return sorted(self.warnings)


class ContentValidator:
    def __init__(self, root):
        self.root = Path(root)
        self.diags = Diagnostics()
        self.quiet = False
        self.enums = {}
        self.entities = {}
        self.composites = {}
        self.seed_files = {}
        self.id_pattern = None
        self.id_max_length = 0
        self.name_min_length = 0
        self.name_max_length = 0
        self.file_for_kind = {}
        self.kind_for_file = {}
        self.zone_ids = set()
        self.ids_by_entity = {}
        self.zone_locations = {}
        self.item_counts = {}
        self.schema_ok = False

    def schema_diagnostic(self, path, code, message):
        self.diags.error("schema.json", path, code, message)

    def load_json(self, path, display_name):
        try:
            with open(path, "r", encoding="utf-8") as handle:
                return json.load(handle)
        except FileNotFoundError:
            self.diags.error(display_name, "$", "E_FILE_MISSING", "file not found")
        except UnicodeDecodeError:
            self.diags.error(display_name, "$", "E_JSON_PARSE", "file is not valid UTF-8")
        except json.JSONDecodeError as exc:
            self.diags.error(
                display_name,
                "$",
                "E_JSON_PARSE",
                "invalid JSON: {} at line {} column {}".format(exc.msg, exc.lineno, exc.colno),
            )
        return None

    def type_spec_is_valid(self, spec, seen):
        if spec.startswith("enum:"):
            return spec.split(":", 1)[1] in self.enums
        if spec.startswith("reference:"):
            return spec.split(":", 1)[1] in self.entities
        if spec.startswith("array:"):
            inner = spec.split(":", 1)[1]
            if inner in seen:
                return True
            seen.add(inner)
            return inner in self.composites and self.composite_spec_is_valid(inner, self.composites[inner], seen)
        if spec in ("identifier", "string"):
            return True
        if spec == "slot_role_map":
            if "slot_role_map" not in self.composites:
                return False
            map_spec = self.composites["slot_role_map"]
            keys = map_spec.get("keys", "")
            values = map_spec.get("values", "")
            return (
                keys.startswith("enum:")
                and keys.split(":", 1)[1] in self.enums
                and values.startswith("enum:")
                and values.split(":", 1)[1] in self.enums
            )
        if spec in self.composites:
            if spec in seen:
                return True
            seen.add(spec)
            return self.composite_spec_is_valid(spec, self.composites[spec], seen)
        return False

    def composite_spec_is_valid(self, name, spec, seen):
        fields = spec.get("fields")
        required = spec.get("required_fields", [])
        if not isinstance(fields, dict) or not isinstance(required, list):
            self.schema_diagnostic(
                "$.composite_types.{}".format(name),
                "E_SCHEMA_INVALID",
                "composite '{}' needs a fields object and a required_fields list".format(name),
            )
            return False
        ok = True
        for field_name in sorted(fields.keys()):
            field_spec = fields[field_name]
            if not isinstance(field_spec, str) or not self.type_spec_is_valid(field_spec, seen):
                self.schema_diagnostic(
                    "$.composite_types.{}.fields.{}".format(name, field_name),
                    "E_SCHEMA_INVALID",
                    "unrecognized field type {}".format(value_repr(field_spec)),
                )
                ok = False
        for field_name in required:
            if field_name not in fields:
                self.schema_diagnostic(
                    "$.composite_types.{}.required_fields".format(name),
                    "E_SCHEMA_INVALID",
                    "required field '{}' is not declared in fields".format(field_name),
                )
                ok = False
        return ok

    def load_schema(self):
        doc = self.load_json(self.root / "schema.json", "schema.json")
        if doc is None:
            return False
        if not isinstance(doc, dict):
            self.schema_diagnostic("$", "E_BAD_TYPE", "schema must be a JSON object")
            return False
        version = doc.get("schema_version")
        if version != SUPPORTED_SCHEMA_VERSION:
            self.schema_diagnostic(
                "$.schema_version",
                "E_SCHEMA_VERSION",
                "unsupported schema version {}; this validator supports {}".format(
                    value_repr(version), SUPPORTED_SCHEMA_VERSION
                ),
            )
            return False
        sections = {
            "enums": dict,
            "entities": dict,
            "composite_types": dict,
            "seed_files": dict,
            "identifier_rules": dict,
            "display_name_rules": dict,
        }
        for section in sorted(sections.keys()):
            if not isinstance(doc.get(section), sections[section]):
                self.schema_diagnostic(
                    "$.{}".format(section),
                    "E_SCHEMA_INVALID",
                    "schema section '{}' must be an object".format(section),
                )
                return False
        self.enums = doc["enums"]
        self.entities = doc["entities"]
        self.composites = doc["composite_types"]
        self.seed_files = doc["seed_files"]
        id_rules = doc["identifier_rules"]
        name_rules = doc["display_name_rules"]
        pattern = id_rules.get("pattern")
        if not isinstance(pattern, str):
            self.schema_diagnostic("$.identifier_rules.pattern", "E_SCHEMA_INVALID", "identifier pattern must be a string")
            return False
        try:
            self.id_pattern = re.compile(pattern)
        except re.error as exc:
            self.schema_diagnostic("$.identifier_rules.pattern", "E_SCHEMA_INVALID", "identifier pattern is not a valid regex: {}".format(exc))
            return False
        if not isinstance(id_rules.get("max_length"), int) or isinstance(id_rules.get("max_length"), bool):
            self.schema_diagnostic("$.identifier_rules.max_length", "E_SCHEMA_INVALID", "max_length must be an integer")
            return False
        if not isinstance(name_rules.get("min_length"), int) or isinstance(name_rules.get("min_length"), bool):
            self.schema_diagnostic("$.display_name_rules.min_length", "E_SCHEMA_INVALID", "min_length must be an integer")
            return False
        if not isinstance(name_rules.get("max_length"), int) or isinstance(name_rules.get("max_length"), bool):
            self.schema_diagnostic("$.display_name_rules.max_length", "E_SCHEMA_INVALID", "max_length must be an integer")
            return False
        self.id_max_length = id_rules["max_length"]
        self.name_min_length = name_rules["min_length"]
        self.name_max_length = name_rules["max_length"]
        for enum_name in sorted(self.enums.keys()):
            members = self.enums[enum_name]
            if not isinstance(members, list) or not all(isinstance(member, str) for member in members):
                self.schema_diagnostic("$.enums.{}".format(enum_name), "E_SCHEMA_INVALID", "enum must be a list of strings")
                return False
        ok = True
        for entity_name in sorted(self.entities.keys()):
            entity = self.entities[entity_name]
            if not isinstance(entity, dict) or not isinstance(entity.get("fields"), dict):
                self.schema_diagnostic("$.entities.{}".format(entity_name), "E_SCHEMA_INVALID", "entity must declare a fields object")
                ok = False
                continue
            required = entity.get("required_fields", [])
            if not isinstance(required, list):
                self.schema_diagnostic("$.entities.{}.required_fields".format(entity_name), "E_SCHEMA_INVALID", "required_fields must be a list")
                ok = False
                continue
            for field_name in sorted(entity["fields"].keys()):
                field_spec = entity["fields"][field_name]
                if not isinstance(field_spec, str) or not self.type_spec_is_valid(field_spec, set()):
                    self.schema_diagnostic(
                        "$.entities.{}.fields.{}".format(entity_name, field_name),
                        "E_SCHEMA_INVALID",
                        "unrecognized field type {}".format(value_repr(field_spec)),
                    )
                    ok = False
            for field_name in required:
                if field_name not in entity["fields"]:
                    self.schema_diagnostic(
                        "$.entities.{}.required_fields".format(entity_name),
                        "E_SCHEMA_INVALID",
                        "required field '{}' is not declared in fields".format(field_name),
                    )
                    ok = False
        for composite_name in sorted(self.composites.keys()):
            if composite_name == "slot_role_map":
                continue
            ok = self.composite_spec_is_valid(composite_name, self.composites[composite_name], set()) and ok
        if "slot_role_map" in self.composites:
            map_spec = self.composites["slot_role_map"]
            keys = map_spec.get("keys", "")
            values = map_spec.get("values", "")
            if not (
                isinstance(keys, str)
                and keys.startswith("enum:")
                and keys.split(":", 1)[1] in self.enums
                and isinstance(values, str)
                and values.startswith("enum:")
                and values.split(":", 1)[1] in self.enums
            ):
                self.schema_diagnostic(
                    "$.composite_types.slot_role_map",
                    "E_SCHEMA_INVALID",
                    "slot_role_map needs 'keys' and 'values' referencing declared enums",
                )
                ok = False
        for kind in sorted(self.seed_files.keys()):
            rel_path = self.seed_files[kind]
            if not isinstance(rel_path, str):
                self.schema_diagnostic("$.seed_files.{}".format(kind), "E_SCHEMA_INVALID", "seed file path must be a string")
                ok = False
                continue
            if kind not in self.entities:
                self.schema_diagnostic("$.seed_files.{}".format(kind), "E_SCHEMA_INVALID", "seed file kind '{}' has no entity".format(kind))
                ok = False
                continue
            self.kind_for_file[rel_path] = kind
            self.file_for_kind[kind] = rel_path
        if not ok:
            return False
        self.schema_ok = True
        return True

    def check_identifier(self, file_name, path, value):
        if not isinstance(value, str):
            self.diags.error(file_name, path, "E_BAD_TYPE", "expected a string identifier, got {}".format(type(value).__name__))
            return False
        if len(value) > self.id_max_length or not self.id_pattern.match(value):
            self.diags.error(
                file_name,
                path,
                "E_ID_FORMAT",
                "identifier {} violates pattern {!r} with max_length {}".format(
                    value, self.id_pattern.pattern, self.id_max_length
                ),
            )
            return False
        return True

    def check_display_name(self, file_name, path, value):
        if not isinstance(value, str):
            self.diags.error(file_name, path, "E_BAD_TYPE", "expected a string display name, got {}".format(type(value).__name__))
            return False
        length = len(value)
        if length < self.name_min_length or length > self.name_max_length:
            self.diags.error(
                file_name,
                path,
                "E_NAME_LENGTH",
                "display name length {} outside allowed range [{}, {}]".format(length, self.name_min_length, self.name_max_length),
            )
            return False
        return True

    def check_enum(self, file_name, path, enum_name, value):
        members = self.enums[enum_name]
        if not isinstance(value, str) or value not in members:
            self.diags.error(
                file_name,
                path,
                ENUM_CODE_BY_ENUM.get(enum_name, "E_UNKNOWN_ENUM_MEMBER"),
                "value {} is not a member of enum {} (accepted: {})".format(
                    value if isinstance(value, str) else value_repr(value), enum_name, ", ".join(sorted(members))
                ),
            )
            return False
        return True

    def check_field_value(self, file_name, path, spec, value):
        if spec == "identifier":
            return self.check_identifier(file_name, path, value)
        if spec == "string":
            return self.check_display_name(file_name, path, value)
        if spec.startswith("enum:"):
            return self.check_enum(file_name, path, spec.split(":", 1)[1], value)
        if spec.startswith("reference:"):
            return self.check_identifier(file_name, path, value)
        if spec.startswith("array:"):
            if not isinstance(value, list):
                self.diags.error(file_name, path, "E_BAD_TYPE", "expected an array")
                return False
            inner = spec.split(":", 1)[1]
            ok = True
            for index, entry in enumerate(value):
                entry_path = "{}[{}]".format(path, index)
                ok = self.check_composite(file_name, entry_path, inner, self.composites[inner], entry) and ok
            return ok
        if spec == "slot_role_map":
            map_spec = self.composites["slot_role_map"]
            key_enum = map_spec["keys"].split(":", 1)[1]
            value_enum = map_spec["values"].split(":", 1)[1]
            if not isinstance(value, dict):
                self.diags.error(file_name, path, "E_BAD_TYPE", "expected an object mapping slots to roles")
                return False
            ok = True
            for key in sorted(value.keys()):
                slot_ok = self.check_enum(file_name, "{}.{}".format(path, key), key_enum, key)
                role_ok = self.check_enum(file_name, "{}.{}".format(path, key), value_enum, value[key])
                ok = slot_ok and role_ok and ok
            return ok
        if spec in self.composites:
            return self.check_composite(file_name, path, spec, self.composites[spec], value)
        self.diags.error(file_name, path, "E_SCHEMA_INVALID", "unrecognized field type {}".format(value_repr(spec)))
        return False

    def check_composite(self, file_name, path, name, spec, value):
        if not isinstance(value, dict):
            self.diags.error(file_name, path, "E_BAD_TYPE", "expected an object for composite '{}'".format(name))
            return False
        fields = spec["fields"]
        required = spec.get("required_fields", fields.keys())
        allowed = set(fields.keys())
        ok = True
        for key in sorted(set(value.keys()) - allowed):
            self.diags.error(
                file_name,
                "{}.{}".format(path, key),
                "E_UNKNOWN_FIELD",
                "field '{}' is not defined for '{}'".format(key, name),
            )
            ok = False
        for key in required:
            if key not in value:
                self.diags.error(
                    file_name,
                    "{}.{}".format(path, key),
                    "E_MISSING_FIELD",
                    "required field '{}' of '{}' is missing".format(key, name),
                )
                ok = False
        for key in sorted(allowed & set(value.keys())):
            ok = self.check_field_value(file_name, "{}.{}".format(path, key), fields[key], value[key]) and ok
        return ok

    def check_envelope(self, doc, file_name, expected_kind):
        if not isinstance(doc, dict):
            self.diags.error(file_name, "$", "E_BAD_TYPE", "seed envelope must be a JSON object")
            return None
        fatal = False
        for key in sorted(set(doc.keys()) - set(ENVELOPE_KEYS)):
            self.diags.error(file_name, "$.{}".format(key), "E_UNKNOWN_FIELD", "unknown envelope field '{}'".format(key))
        for key in ENVELOPE_KEYS:
            if key not in doc:
                self.diags.error(file_name, "$.{}".format(key), "E_MISSING_FIELD", "missing envelope field '{}'".format(key))
                fatal = True
        version = doc.get("schema_version")
        if "schema_version" in doc and version != SUPPORTED_SCHEMA_VERSION:
            self.diags.error(
                file_name,
                "$.schema_version",
                "E_SCHEMA_VERSION",
                "unsupported seed schema version {}; this validator supports {}".format(
                    value_repr(version), SUPPORTED_SCHEMA_VERSION
                ),
            )
            fatal = True
        kind = doc.get("kind")
        if "kind" in doc and kind != expected_kind:
            self.diags.error(
                file_name,
                "$.kind",
                "E_FILE_KIND",
                "expected kind '{}' for this seed file, got {}".format(expected_kind, value_repr(kind)),
            )
            fatal = True
        items = doc.get("items")
        if "items" in doc and not isinstance(items, list):
            self.diags.error(file_name, "$.items", "E_BAD_TYPE", "'items' must be an array")
            fatal = True
        if fatal:
            return None
        return items

    def check_item(self, item, index, kind, file_name):
        path = "items[{}]".format(index)
        if not isinstance(item, dict):
            self.diags.error(file_name, path, "E_BAD_TYPE", "expected a '{}' object".format(kind))
            return None
        entity = self.entities[kind]
        fields = entity["fields"]
        required = entity["required_fields"]
        allowed = set(fields.keys())
        ok = True
        for key in sorted(set(item.keys()) - allowed):
            self.diags.error(
                file_name,
                "{}.{}".format(path, key),
                "E_UNKNOWN_FIELD",
                "unknown field '{}' for entity '{}'".format(key, kind),
            )
            ok = False
        for key in required:
            if key not in item:
                self.diags.error(
                    file_name,
                    "{}.{}".format(path, key),
                    "E_MISSING_FIELD",
                    "required field '{}' missing on '{}'".format(key, kind),
                )
                ok = False
        for key in sorted(allowed & set(item.keys())):
            ok = self.check_field_value(file_name, "{}.{}".format(path, key), fields[key], item[key]) and ok
        item_id = item.get("id")
        if not ok or not isinstance(item_id, str) or not self.id_pattern.match(item_id) or len(item_id) > self.id_max_length:
            return None
        return item_id

    def register_ids(self, docs):
        for kind in sorted(docs.keys()):
            file_name = self.file_for_kind[kind]
            owner = {}
            members = set()
            kept = 0
            for index, item in enumerate(docs[kind]):
                item_id = self.check_item(item, index, kind, file_name)
                if item_id is None:
                    continue
                kept += 1
                members.add(item_id)
                owner[item_id] = (file_name, "items[{}]".format(index))
            self.item_counts[kind] = kept
            self.ids_by_entity[kind] = members
            if kind == "zone":
                self.zone_ids = members
                for item_index, item in enumerate(docs[kind]):
                    if isinstance(item, dict) and isinstance(item.get("id"), str):
                        self.zone_locations[item["id"]] = (file_name, "items[{}]".format(item_index))

    def check_duplicates(self, docs):
        seen_global = {}
        for kind in sorted(docs.keys()):
            file_name = self.file_for_kind[kind]
            for index, item in enumerate(docs[kind]):
                if not isinstance(item, dict):
                    continue
                item_id = item.get("id")
                if not isinstance(item_id, str):
                    continue
                if item_id in seen_global:
                    first_file, first_path = seen_global[item_id]
                    self.diags.error(
                        file_name,
                        "items[{}]".format(index),
                        "E_DUPLICATE_ID",
                        "duplicate id '{}' first defined at {}:{}".format(item_id, first_file, first_path),
                    )
                else:
                    seen_global[item_id] = (file_name, "items[{}]".format(index))

    def check_zone_exit_refs(self, docs):
        zones = docs.get("zone")
        if zones is None:
            return
        file_name = self.file_for_kind["zone"]
        for index, zone in enumerate(zones):
            exits = zone.get("exits") if isinstance(zone, dict) else None
            if not isinstance(exits, list):
                continue
            for edge_index, edge in enumerate(exits):
                if not isinstance(edge, dict):
                    continue
                target = edge.get("to")
                if isinstance(target, str) and target not in self.zone_ids:
                    self.diags.error(
                        file_name,
                        "items[{}].exits[{}].to".format(index, edge_index),
                        "E_UNKNOWN_ZONE_REF",
                        "exit leads to unknown zone id '{}'".format(target),
                    )

    def check_duplicate_exits(self, docs):
        zones = docs.get("zone")
        if zones is None:
            return
        file_name = self.file_for_kind["zone"]
        for index, zone in enumerate(zones):
            exits = zone.get("exits") if isinstance(zone, dict) else None
            if not isinstance(exits, list):
                continue
            seen_edges = set()
            for edge_index, edge in enumerate(exits):
                if not isinstance(edge, dict):
                    continue
                signature = value_repr(edge)
                if signature in seen_edges:
                    self.diags.error(
                        file_name,
                        "items[{}].exits[{}]".format(index, edge_index),
                        "E_DUPLICATE_EXIT",
                        "duplicate exit {} within one zone".format(signature),
                    )
                seen_edges.add(signature)

    def check_reference_fields(self, docs):
        for entity_name in sorted(self.entities.keys()):
            fields = self.entities[entity_name]["fields"]
            if entity_name not in docs:
                continue
            file_name = self.file_for_kind[entity_name]
            for field_name in sorted(fields.keys()):
                spec = fields[field_name]
                if not spec.startswith("reference:"):
                    continue
                target_entity = spec.split(":", 1)[1]
                targets = self.ids_by_entity.get(target_entity, set())
                code = "E_UNKNOWN_ZONE_REF" if target_entity == "zone" else "E_UNKNOWN_REFERENCE"
                for index, item in enumerate(docs[entity_name]):
                    if not isinstance(item, dict):
                        continue
                    target = item.get(field_name)
                    if isinstance(target, str) and target not in targets:
                        self.diags.error(
                            file_name,
                            "items[{}].{}".format(index, field_name),
                            code,
                            "'{}' references unknown '{}' id '{}'".format(field_name, target_entity, target),
                        )

    def check_reachability(self, docs):
        zones = docs.get("zone")
        if zones is None or not self.zone_ids:
            return
        adjacency = {zone_id: set() for zone_id in self.zone_ids}
        for zone in zones:
            source = zone.get("id") if isinstance(zone, dict) else None
            if not isinstance(source, str) or source not in adjacency:
                continue
            exits = zone.get("exits")
            if isinstance(exits, list):
                for edge in exits:
                    if isinstance(edge, dict) and isinstance(edge.get("to"), str) and edge["to"] in adjacency:
                        adjacency[source].add(edge["to"])
        root = min(self.zone_ids)
        reached = {root}
        frontier = [root]
        while frontier:
            current = frontier.pop()
            for neighbor in sorted(adjacency[current]):
                if neighbor not in reached:
                    reached.add(neighbor)
                    frontier.append(neighbor)
        file_name = self.file_for_kind["zone"]
        for zone_id in sorted(self.zone_ids - reached):
            _, zone_path = self.zone_locations.get(zone_id, (file_name, zone_id))
            self.diags.warning(
                file_name,
                zone_path,
                "W_UNREACHABLE_ZONE",
                "zone '{}' is not reachable from graph root '{}'".format(zone_id, root),
            )

    def run(self):
        if not self.load_schema():
            return self.finish()
        docs = {}
        for rel_path in sorted(self.kind_for_file.keys()):
            kind = self.kind_for_file[rel_path]
            raw = self.load_json(self.root / rel_path, rel_path)
            if raw is None:
                continue
            items = self.check_envelope(raw, rel_path, kind)
            if items is None:
                continue
            docs[kind] = items
        self.register_ids(docs)
        self.check_duplicates(docs)
        self.check_zone_exit_refs(docs)
        self.check_duplicate_exits(docs)
        self.check_reference_fields(docs)
        self.check_reachability(docs)
        return self.finish()

    def finish(self):
        warnings = self.diags.sorted_warnings()
        errors = self.diags.sorted_errors()
        if not self.quiet:
            for file_name, path, code, message in warnings:
                print("WARNING {}:{} {}: {}".format(file_name, path, code, message))
            for file_name, path, code, message in errors:
                print("ERROR {}:{} {}: {}".format(file_name, path, code, message))
        if errors:
            summary = "FAIL errors={} warnings={}".format(len(errors), len(warnings))
            exit_code = 1
        else:
            counts = " ".join("{}={}".format(kind, self.item_counts.get(kind, 0)) for kind in sorted(self.item_counts))
            prefix = "schema={} ".format(SUPPORTED_SCHEMA_VERSION) if self.schema_ok else ""
            suffix = " " + counts if counts else ""
            summary = "OK {}errors=0 warnings={}{}".format(prefix, len(warnings), suffix)
            exit_code = 0
        print(summary)
        return exit_code


def main(argv):
    parser = argparse.ArgumentParser(
        prog="validate_content.py",
        description="Deterministic dependency-free validator for the Verdigris native content seam.",
    )
    parser.add_argument(
        "--root",
        default=str(Path(__file__).resolve().parent),
        help="content root directory containing schema.json and seeds/",
    )
    parser.add_argument("--quiet", action="store_true", help="suppress individual diagnostics; print only the final summary line")
    args = parser.parse_args(argv)
    validator = ContentValidator(args.root)
    validator.quiet = args.quiet
    return validator.run()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
