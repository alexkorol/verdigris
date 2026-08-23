#!/usr/bin/env python3
"""Verdigris native visual-kit packaging proof (TASK-0160).

Proves the committed vector kit -- native/client/assets/svg/*.svg,
native/client/assets/manifest.json and
native/client/assets/generated/visual_kit.h -- forms one reproducible,
dependency-free asset contract:

  * every manifest entry exists on disk as a valid bounded safe SVG,
  * every entry maps to exactly one stable generated symbol, in the same
    order, with matching generator version metadata,
  * the committed manifest and generated header reproduce byte-for-byte
    from the SVG sources alone (the frozen task0147-gen-2 layout emitted by
    orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs).

Modes:
  --check      read-only validation; never writes anything
  --regenerate rewrite manifest.json and generated/visual_kit.h from the SVG
               sources (SVG files are inputs and are never generated here)

Python 3 stdlib only. Exit codes: 0 pass, 1 validation failures, 2 usage/IO.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path, PurePosixPath

GENERATOR_VERSION = "task0147-gen-2"
ASSETS_RELPATH = PurePosixPath("native/client/assets")
SVG_DIR_RELPATH = ASSETS_RELPATH / "svg"
HEADER_RELPATH = ASSETS_RELPATH / "generated" / "visual_kit.h"
MANIFEST_RELPATH = ASSETS_RELPATH / "manifest.json"

VIEW_WIDTH = 64
VIEW_HEIGHT = 64

SVG_NS = "http://www.w3.org/2000/svg"
HEX_COLOR_RE = re.compile(r"^#[0-9a-f]{6}([0-9a-f]{2})?$")
NUMBER_RE = re.compile(r"^(?:\d+(?:\.\d+)?)$")
SYMBOL_ROW_RE = re.compile(
    r'^    \{"(?P<role>[^"]+)", "(?P<motif>[^"]+)", "(?P<source>[^"]+)", '
    r"(?P<width>\d+\.[^,]*f), (?P<height>\d+\.[^,]*f), "
    r"(?P<begin>-?\d+), (?P<end>-?\d+)\},$"
)

ELEMENT_ATTRS = {
    "svg": {"width", "height", "viewBox"},
    "circle": {"cx", "cy", "r", "fill", "stroke", "stroke-width", "stroke-linejoin"},
    "ellipse": {"cx", "cy", "rx", "ry", "fill", "stroke", "stroke-width", "stroke-linejoin"},
    "polygon": {"points", "fill", "stroke", "stroke-width", "stroke-linejoin"},
    "polyline": {"points", "fill", "stroke", "stroke-width", "stroke-linejoin"},
}
FORBIDDEN_SVG_SUBSTRINGS = (
    "<!doctype",
    "<!entity",
    "<?xml-stylesheet",
    "<![cdata[",
    "<script",
    "<image",
    "<foreignobject",
    "xlink:",
    "javascript:",
)


class Report:
    def __init__(self) -> None:
        self.failures: list[tuple[str, str]] = []

    def fail(self, code: str, detail: str) -> None:
        self.failures.append((code, detail))

    @property
    def ok(self) -> bool:
        return not self.failures


def repo_root_from_tool() -> Path:
    return Path(__file__).resolve().parents[2]


def fmt_trim(text: str) -> str:
    if "." in text:
        text = text.rstrip("0").rstrip(".")
    if text == "-0":
        text = "0"
    return text


def cpp_float(text: str) -> str:
    text = fmt_trim(text)
    if "." not in text:
        text += "."
    return text + "f"


def hex_channel_percent(hex_pair: str) -> int:
    value = int(hex_pair, 16)
    whole, remainder = divmod(value * 100, 255)
    if remainder * 2 >= 255:
        whole += 1
    return whole


def percent_text(percent: int) -> str:
    return f"{percent // 100}.{percent % 100:02d}"


def hex_color_parts(color: str) -> tuple[str, str, str, str]:
    digits = color[1:]
    alpha_digits = digits[6:8]
    alpha_percent = hex_channel_percent(alpha_digits) if alpha_digits else 100
    return (
        cpp_float(percent_text(hex_channel_percent(digits[0:2]))),
        cpp_float(percent_text(hex_channel_percent(digits[2:4]))),
        cpp_float(percent_text(hex_channel_percent(digits[4:6]))),
        cpp_float(percent_text(alpha_percent)),
    )


class SvgShape:
    def __init__(
        self,
        kind: str,
        points: list[tuple[str, str]],
        fill: str | None,
        stroke: str | None,
        stroke_width: str | None,
        geometry: dict[str, str],
    ) -> None:
        self.kind = kind
        self.points = points
        self.fill = fill
        self.stroke = stroke
        self.stroke_width = stroke_width
        self.geometry = geometry


class ParsedSvg:
    def __init__(self, relpath: str, shapes: list[SvgShape]) -> None:
        self.relpath = relpath
        self.shapes = shapes


def compute_palette(shapes: list[SvgShape]) -> list[str]:
    seen: list[str] = []
    for shape in shapes:
        for value in (shape.fill, shape.stroke):
            if value is not None and value != "none" and value not in seen:
                seen.append(value)
    return seen


def _local_name(tag: str) -> str:
    return tag.split("}", 1)[1] if tag.startswith("{") else tag


def _namespace_of(tag: str) -> str:
    return tag[1:].split("}", 1)[0] if tag.startswith("{") else ""


def parse_svg(relpath: str, raw: str, report: Report) -> ParsedSvg | None:
    lowered = raw.lower()
    for needle in FORBIDDEN_SVG_SUBSTRINGS:
        if needle in lowered:
            report.fail("UNSAFE_SVG", f"{relpath}: forbidden construct {needle!r}")
            return None
    try:
        root = ET.fromstring(raw)
    except ET.ParseError as error:
        report.fail("MALFORMED_SVG", f"{relpath}: XML parse error: {error}")
        return None

    if _namespace_of(root.tag) != SVG_NS or _local_name(root.tag) != "svg":
        report.fail("UNSAFE_SVG", f"{relpath}: root element must be <svg> in the {SVG_NS} namespace")
        return None
    if f'xmlns="{SVG_NS}"' not in raw:
        report.fail("UNSAFE_SVG", f"{relpath}: missing exact xmlns={SVG_NS} declaration")
        return None
    unexpected_root_attrs = set(root.attrib) - ELEMENT_ATTRS["svg"]
    if unexpected_root_attrs:
        report.fail(
            "UNSAFE_SVG",
            f"{relpath}: unexpected attributes on <svg>: {sorted(unexpected_root_attrs)}",
        )
        return None
    root_attrs = dict(root.attrib)
    unexpected_root_attrs = set(root_attrs) - ELEMENT_ATTRS["svg"]
    if unexpected_root_attrs:
        report.fail(
            "UNSAFE_SVG",
            f"{relpath}: unexpected attributes on <svg>: {sorted(unexpected_root_attrs)}",
        )
        return None
    if (
        root_attrs.get("width") != str(VIEW_WIDTH)
        or root_attrs.get("height") != str(VIEW_HEIGHT)
        or root_attrs.get("viewBox") != f"0 0 {VIEW_WIDTH} {VIEW_HEIGHT}"
    ):
        report.fail(
            "OUT_OF_BOUNDS_SVG",
            f"{relpath}: bounds must be width={VIEW_WIDTH} height={VIEW_HEIGHT} "
            f"viewBox='0 0 {VIEW_WIDTH} {VIEW_HEIGHT}'",
        )
        return None
    if (root.text or "").strip() or any((child.tail or "").strip() for child in root):
        report.fail("UNSAFE_SVG", f"{relpath}: non-whitespace text content")
        return None

    shapes: list[SvgShape] = []
    for element in root:
        name = _local_name(element.tag)
        if _namespace_of(element.tag) != SVG_NS:
            report.fail("UNSAFE_SVG", f"{relpath}: foreign-namespaced element <{element.tag}>")
            return None
        if name not in ELEMENT_ATTRS or name == "svg":
            report.fail("UNSAFE_SVG", f"{relpath}: disallowed element <{name}>")
            return None
        attrs = dict(element.attrib)
        unexpected = set(attrs) - ELEMENT_ATTRS[name]
        if unexpected:
            report.fail("UNSAFE_SVG", f"{relpath}: unexpected attributes on <{name}>: {sorted(unexpected)}")
            return None
        if list(element):
            report.fail("UNSAFE_SVG", f"{relpath}: nested elements under <{name}>")
            return None
        if (element.text or "").strip():
            report.fail("UNSAFE_SVG", f"{relpath}: text content inside <{name}>")
            return None

        fill = attrs.get("fill")
        stroke = attrs.get("stroke")
        for label, color_value in (("fill", fill), ("stroke", stroke)):
            if color_value is not None and color_value != "none" and not HEX_COLOR_RE.match(color_value):
                report.fail("UNSAFE_SVG", f"{relpath}: bad {label} color {color_value!r}")
                return None

        stroke_width = attrs.get("stroke-width")
        if stroke is not None and stroke != "none":
            if stroke_width is None or not NUMBER_RE.match(stroke_width):
                report.fail("MALFORMED_SVG", f"{relpath}: <{name}> missing numeric stroke-width")
                return None
            if not (0.0 <= float(stroke_width) <= float(VIEW_WIDTH)):
                report.fail("OUT_OF_BOUNDS_SVG", f"{relpath}: stroke-width {stroke_width} outside [0,{VIEW_WIDTH}]")
                return None
            if attrs.get("stroke-linejoin") != "round":
                report.fail("UNSAFE_SVG", f"{relpath}: <{name}> stroked shapes must use stroke-linejoin='round'")
                return None
        elif stroke_width is not None or "stroke-linejoin" in attrs:
            report.fail("MALFORMED_SVG", f"{relpath}: <{name}> carries stroke styling without a stroke")
            return None

        geometry: dict[str, str] = {}
        points: list[tuple[str, str]] = []
        if name in ("circle", "ellipse"):
            required = ("cx", "cy", "r") if name == "circle" else ("cx", "cy", "rx", "ry")
            for attr in required:
                lexeme = attrs.get(attr)
                if lexeme is None or not NUMBER_RE.match(lexeme):
                    report.fail("MALFORMED_SVG", f"{relpath}: <{name}> attribute {attr} must be a plain number")
                    return None
                if not (0.0 <= float(lexeme) <= float(VIEW_WIDTH)):
                    report.fail("OUT_OF_BOUNDS_SVG", f"{relpath}: {attr}={lexeme} outside [0,{VIEW_WIDTH}]")
                    return None
                geometry[attr] = lexeme
        else:
            points_attr = attrs.get("points")
            if points_attr is None or not points_attr.strip():
                report.fail("MALFORMED_SVG", f"{relpath}: <{name}> missing points")
                return None
            for token in points_attr.strip().split():
                parts = token.split(",")
                if len(parts) != 2 or not all(NUMBER_RE.match(part) for part in parts):
                    report.fail("MALFORMED_SVG", f"{relpath}: <{name}> malformed vertex {token!r}")
                    return None
                x_lex, y_lex = parts
                if not (0.0 <= float(x_lex) <= float(VIEW_WIDTH)) or not (0.0 <= float(y_lex) <= float(VIEW_WIDTH)):
                    report.fail("OUT_OF_BOUNDS_SVG", f"{relpath}: vertex ({x_lex},{y_lex}) outside viewBox")
                    return None
                points.append((x_lex, y_lex))

        shapes.append(SvgShape(name, points, fill, stroke, stroke_width, geometry))

    if not shapes:
        report.fail("MALFORMED_SVG", f"{relpath}: no drawable shapes")
        return None
    return ParsedSvg(relpath, shapes)


def derive_manifest(entries: list[dict], palettes: dict[str, list[str]]) -> bytes:
    roles: list[dict] = []
    for entry in entries:
        role_entry = next((candidate for candidate in roles if candidate["role"] == entry["role"]), None)
        if role_entry is None:
            role_entry = {"role": entry["role"], "motifs": []}
            roles.append(role_entry)
        role_entry["motifs"].append(
            {
                "motif": entry["motif"],
                "symbol": entry["symbol"],
                "source": entry["source"],
                "palette": palettes[entry["source"]],
            }
        )
    document = {
        "generatorVersion": GENERATOR_VERSION,
        "viewBox": {"width": VIEW_WIDTH, "height": VIEW_HEIGHT},
        "roles": roles,
    }
    return (json.dumps(document, indent=2, ensure_ascii=False) + "\n").encode("utf-8")


HEADER_PROLOGUE_LINES = [
    "#pragma once",
    "",
    "#include <cstdint>",
    "",
    "namespace verdigris::visual_kit {",
    "",
]
HEADER_TYPE_LINES = [
    "enum class ShapeKind : int32_t {",
    "  Polygon,",
    "  Polyline,",
    "  Circle,",
    "  Ellipse,",
    "};",
    "",
    "struct Color {",
    "  float r;",
    "  float g;",
    "  float b;",
    "  float a;",
    "};",
    "",
    "struct Shape {",
    "  ShapeKind kind;",
    "  int32_t point_begin;",
    "  int32_t point_end;",
    "  int32_t fill;",
    "  int32_t stroke;",
    "  float stroke_width;",
    "  float cx;",
    "  float cy;",
    "  float rx;",
    "  float ry;",
    "};",
    "",
    "struct Symbol {",
    "  const char* role;",
    "  const char* motif;",
    "  const char* source;",
    "  float width;",
    "  float height;",
    "  int32_t shape_begin;",
    "  int32_t shape_end;",
    "};",
    "",
]


def derive_header(entries: list[dict], parsed: dict[str, ParsedSvg]) -> bytes:
    colors: list[str] = []
    color_positions: dict[str, int] = {}

    def color_index(color: str | None) -> int:
        if color is None or color == "none":
            return -1
        if color not in color_positions:
            color_positions[color] = len(colors)
            colors.append(color)
        return color_positions[color]

    flat_points: list[str] = []
    flat_shapes: list[list[str]] = []
    symbol_rows: list[tuple[dict, int, int]] = []
    point_cursor = 0
    kind_names = {"polygon": "Polygon", "polyline": "Polyline", "circle": "Circle", "ellipse": "Ellipse"}
    for entry in entries:
        source_svg = parsed[entry["source"]]
        shape_begin = len(flat_shapes)
        for shape in source_svg.shapes:
            point_begin = point_cursor
            if shape.kind in ("polygon", "polyline"):
                extra = ["0", "0", "0", "0"]
                for x_lex, y_lex in shape.points:
                    flat_points.extend((x_lex, y_lex))
                    point_cursor += 1
            elif shape.kind == "circle":
                extra = [shape.geometry["cx"], shape.geometry["cy"], shape.geometry["r"], "0"]
            else:
                extra = [shape.geometry["cx"], shape.geometry["cy"], shape.geometry["rx"], shape.geometry["ry"]]
            stroke_width = shape.stroke_width if shape.stroke and shape.stroke != "none" else "0"
            flat_shapes.append(
                [
                    f"ShapeKind::{kind_names[shape.kind]}",
                    str(point_begin),
                    str(point_cursor),
                    str(color_index(shape.fill)),
                    str(color_index(shape.stroke)),
                    cpp_float(stroke_width),
                    *(cpp_float(value) for value in extra),
                ]
            )
        symbol_rows.append((entry, shape_begin, len(flat_shapes)))

    lines: list[str] = []
    lines.append(
        "// Generated by orchestration/tasks/TASK-0141-procedural-native-visual-kit/"
        f"generate-assets.mjs version {GENERATOR_VERSION}. DO NOT EDIT."
    )
    lines.extend(HEADER_PROLOGUE_LINES)
    lines.append(f'inline constexpr char kKitVersion[] = "{GENERATOR_VERSION}";')
    lines.append("")
    lines.extend(HEADER_TYPE_LINES)
    lines.append("inline constexpr Color kColors[] = {")
    for color in colors:
        r, g, b, a = hex_color_parts(color)
        lines.append(f"    {{{r}, {g}, {b}, {a}}},")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr float kPoints[] = {")
    for index in range(0, len(flat_points), 8):
        chunk = flat_points[index : index + 8]
        lines.append("    " + ", ".join(cpp_float(lexeme) for lexeme in chunk) + ",")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr Shape kShapes[] = {")
    for fields in flat_shapes:
        lines.append("    {" + ", ".join(fields) + "},")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr Symbol kSymbols[] = {")
    for entry, shape_begin, shape_end in symbol_rows:
        lines.append(
            f'    {{"{entry["role"]}", "{entry["motif"]}", "{entry["source"]}", '
            f"{cpp_float(str(VIEW_WIDTH))}, {cpp_float(str(VIEW_HEIGHT))}, {shape_begin}, {shape_end}}},"
        )
    lines.append("};")
    lines.append("")
    lines.append(f"inline constexpr int32_t kSymbolCount = {len(symbol_rows)};")
    lines.append("")
    lines.append("}")
    return ("\n".join(lines) + "\n").encode("utf-8")


def load_entries(root: Path, report: Report) -> list[dict] | None:
    manifest_path = root / MANIFEST_RELPATH
    if not manifest_path.is_file():
        report.fail("MISSING_MANIFEST_FILE", str(MANIFEST_RELPATH))
        return None
    try:
        document = json.loads(manifest_path.read_bytes().decode("utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        report.fail("MALFORMED_MANIFEST", f"{MANIFEST_RELPATH}: {error}")
        return None

    structure_ok = True
    if not isinstance(document, dict) or set(document) != {"generatorVersion", "viewBox", "roles"}:
        report.fail("MALFORMED_MANIFEST", f"{MANIFEST_RELPATH}: top-level keys must be exactly generatorVersion/viewBox/roles")
        structure_ok = False
        document = {}
    version = document.get("generatorVersion")
    if version != GENERATOR_VERSION:
        report.fail("VERSION_MISMATCH", f"{MANIFEST_RELPATH}: generatorVersion {version!r} != expected {GENERATOR_VERSION!r}")
    expected_view_box = {"width": VIEW_WIDTH, "height": VIEW_HEIGHT}
    if document.get("viewBox") != expected_view_box:
        report.fail("MALFORMED_MANIFEST", f"{MANIFEST_RELPATH}: viewBox must be {expected_view_box}")

    roles = document.get("roles") if isinstance(document.get("roles"), list) else []
    entries: list[dict] = []
    seen_role_motif: set[tuple[str, str]] = set()
    seen_symbols: dict[str, int] = {}
    seen_sources: dict[str, int] = {}
    for role_position, role_entry in enumerate(roles):
        if not isinstance(role_entry, dict) or set(role_entry) != {"role", "motifs"} or not isinstance(role_entry["role"], str):
            report.fail("MALFORMED_MANIFEST", f"{MANIFEST_RELPATH}: roles[{role_position}] must be {{role, motifs}} with string role")
            structure_ok = False
            continue
        role = role_entry["role"]
        motifs = role_entry["motifs"] if isinstance(role_entry["motifs"], list) else []
        if not motifs:
            report.fail("MALFORMED_MANIFEST", f"{MANIFEST_RELPATH}: role {role!r} declares no motifs")
            structure_ok = False
        for motif_position, motif_entry in enumerate(motifs):
            prefix = f"{MANIFEST_RELPATH}: role {role!r} motifs[{motif_position}]"
            if not isinstance(motif_entry, dict) or set(motif_entry) != {"motif", "symbol", "source", "palette"}:
                report.fail("MALFORMED_MANIFEST", f"{prefix}: keys must be exactly motif/symbol/source/palette")
                structure_ok = False
                continue
            motif = motif_entry["motif"]
            symbol = motif_entry["symbol"]
            source = motif_entry["source"]
            palette = motif_entry["palette"]
            if not all(isinstance(field, str) and field for field in (motif, symbol, source)):
                report.fail("MALFORMED_MANIFEST", f"{prefix}: motif/symbol/source must be non-empty strings")
                structure_ok = False
                continue
            if not isinstance(palette, list) or not all(isinstance(item, str) and HEX_COLOR_RE.match(item) for item in palette):
                report.fail("MALFORMED_MANIFEST", f"{prefix}: palette must be a list of #rrggbb[#aa] hex strings")
                structure_ok = False
                continue
            normalized_source = PurePosixPath(source).as_posix()
            parts = PurePosixPath(normalized_source).parts
            safe_location = normalized_source.startswith(str(SVG_DIR_RELPATH) + "/") and normalized_source.endswith(".svg")
            if not safe_location or ".." in parts or "." in parts or normalized_source != source:
                report.fail("MALFORMED_MANIFEST", f"{prefix}: source must be a direct child path of {SVG_DIR_RELPATH}/")
                structure_ok = False
                continue
            if (role, motif) in seen_role_motif:
                report.fail("DUPLICATE_ENTRY", f"{prefix}: duplicate role/motif ({role!r}, {motif!r})")
                structure_ok = False
            seen_role_motif.add((role, motif))
            if symbol in seen_symbols:
                report.fail("DUPLICATE_ENTRY", f"{prefix}: symbol {symbol!r} already declared by roles[{seen_symbols[symbol]}]")
                structure_ok = False
            seen_symbols[symbol] = role_position
            if source in seen_sources:
                report.fail("DUPLICATE_ENTRY", f"{prefix}: source {source!r} already declared by roles[{seen_sources[source]}]")
                structure_ok = False
            seen_sources[source] = role_position
            entries.append({"role": role, "motif": motif, "symbol": symbol, "source": normalized_source, "palette": palette})
    if structure_ok and not entries:
        report.fail("MALFORMED_MANIFEST", f"{MANIFEST_RELPATH}: no manifest entries")
    return entries if structure_ok else None


def collect_svgs(root: Path, entries: list[dict] | None, report: Report) -> dict[str, ParsedSvg]:
    svg_dir = root / SVG_DIR_RELPATH
    referenced: set[str] = {entry["source"] for entry in entries} if entries else set()
    present: set[str] = set()
    if svg_dir.is_dir():
        for path in sorted(svg_dir.glob("*.svg")):
            present.add(PurePosixPath(path.relative_to(root).as_posix()).as_posix())
    else:
        report.fail("MISSING_SOURCE", f"{SVG_DIR_RELPATH}: directory missing")

    for missing in sorted(referenced - present):
        report.fail("MISSING_SOURCE", f"{missing}: listed in manifest but absent on disk")
    for stray in sorted(present - referenced):
        report.fail("UNKNOWN_SVG_FILE", f"{stray}: not referenced by any manifest entry")

    parsed: dict[str, ParsedSvg] = {}
    for relpath in sorted(referenced & present):
        try:
            raw = (root / relpath).read_bytes().decode("utf-8")
        except (OSError, UnicodeDecodeError) as error:
            report.fail("UNREADABLE_SVG", f"{relpath}: {error}")
            continue
        result = parse_svg(relpath, raw, report)
        if result is not None:
            parsed[relpath] = result
    return parsed


def parse_header_symbols(header_bytes: bytes, report: Report) -> list[dict] | None:
    try:
        text = header_bytes.decode("utf-8")
    except UnicodeDecodeError as error:
        report.fail("MALFORMED_HEADER", f"{HEADER_RELPATH}: {error}")
        return None
    version_match = re.search(r'^inline constexpr char kKitVersion\[\] = "([^"]*)";$', text, re.M)
    count_match = re.search(r"^inline constexpr int32_t kSymbolCount = (\d+);$", text, re.M)
    symbols_block = re.search(r"^inline constexpr Symbol kSymbols\[\] = \{\n(.*?)^\};$", text, re.M | re.S)
    if not version_match or not count_match or not symbols_block:
        report.fail("MALFORMED_HEADER", f"{HEADER_RELPATH}: missing kKitVersion/kSymbols/kSymbolCount skeleton")
        return None
    if version_match.group(1) != GENERATOR_VERSION:
        report.fail("VERSION_MISMATCH", f"{HEADER_RELPATH}: kKitVersion {version_match.group(1)!r} != expected {GENERATOR_VERSION!r}")
    rows: list[dict] = []
    for line_number, line in enumerate(symbols_block.group(1).splitlines(), start=1):
        if not line.strip():
            continue
        match = SYMBOL_ROW_RE.match(line)
        if not match:
            report.fail("MALFORMED_HEADER", f"{HEADER_RELPATH}: unparseable kSymbols row {line_number}: {line!r}")
            return None
        rows.append(match.groupdict())
    declared_count = int(count_match.group(1))
    if declared_count != len(rows):
        report.fail("MALFORMED_HEADER", f"{HEADER_RELPATH}: kSymbolCount {declared_count} != {len(rows)} parsed rows")
        return None
    return rows


def check_symbol_mapping(entries: list[dict], rows: list[dict], report: Report) -> None:
    row_keys = [(row["role"], row["motif"], row["source"]) for row in rows]
    entry_keys = [(entry["role"], entry["motif"], entry["source"]) for entry in entries]

    for key in entry_keys:
        matches = row_keys.count(key)
        if matches == 0:
            report.fail("UNKNOWN_SYMBOL_MAPPING", f"manifest entry {key} maps to no generated symbol")
        elif matches > 1:
            report.fail("AMBIGUOUS_SYMBOL_MAPPING", f"manifest entry {key} maps to {matches} generated symbols")
    for key in row_keys:
        if key not in entry_keys:
            report.fail("UNKNOWN_HEADER_ROW", f"generated symbol {key} has no manifest entry")
    if not any(code in dict(report.failures) for code in ("UNKNOWN_SYMBOL_MAPPING", "AMBIGUOUS_SYMBOL_MAPPING")):
        for position, (row_key, entry_key) in enumerate(zip(row_keys, entry_keys)):
            if row_key != entry_key:
                report.fail("ORDERING_MISMATCH", f"position {position}: header {row_key} vs manifest {entry_key}")
                break


def first_difference(committed: bytes, derived: bytes, relpath: PurePosixPath) -> str:
    length = min(len(committed), len(derived))
    offset = next((index for index in range(length) if committed[index] != derived[index]), length)
    start = max(0, offset - 24)
    committed_snippet = committed[start : offset + 24].decode("utf-8", "replace")
    derived_snippet = derived[start : offset + 24].decode("utf-8", "replace")
    return (
        f"{relpath}: differs at byte {offset} (committed {len(committed)}B vs derived {len(derived)}B); "
        f"committed ...{committed_snippet!r}... derived ...{derived_snippet!r}..."
    )


def derive_all(entries: list[dict], parsed: dict[str, ParsedSvg], report: Report) -> tuple[bytes, bytes] | None:
    palettes: dict[str, list[str]] = {}
    for entry in entries:
        source_svg = parsed.get(entry["source"])
        if source_svg is None:
            continue
        computed_palette = compute_palette(source_svg.shapes)
        palettes[entry["source"]] = computed_palette
        if computed_palette != entry["palette"]:
            report.fail(
                "PALETTE_MISMATCH",
                f'{entry["source"]}: manifest palette {entry["palette"]} != computed {computed_palette}',
            )
    if len(palettes) != len({entry["source"] for entry in entries}):
        report.fail("INCOMPLETE_KIT", "not every manifest source could be parsed; derivation skipped")
        return None
    manifest_bytes = derive_manifest(entries, palettes)
    header_bytes = derive_header(entries, parsed)
    if manifest_bytes != derive_manifest(entries, palettes) or header_bytes != derive_header(entries, parsed):
        report.fail("NON_DETERMINISTIC_DERIVATION", "two derivation passes disagreed")
    return manifest_bytes, header_bytes


def digest_ledger(paths: list[PurePosixPath], root: Path) -> list[tuple[str, str]]:
    ledger: list[tuple[str, str]] = []
    for relpath in paths:
        ledger.append((str(relpath), hashlib.sha256((root / relpath).read_bytes()).hexdigest()))
    return ledger


def emit_success_digests(root: Path) -> None:
    asset_paths = [MANIFEST_RELPATH, HEADER_RELPATH]
    svg_dir = root / SVG_DIR_RELPATH
    if svg_dir.is_dir():
        asset_paths.extend(
            PurePosixPath(path.relative_to(root).as_posix()) for path in sorted(svg_dir.glob("*.svg"))
        )
    print("digests (sha256):")
    for relpath, digest in digest_ledger(asset_paths, root):
        print(f"  {digest}  {relpath}")


def run_check(root: Path) -> int:
    report = Report()
    entries = load_entries(root, report)
    parsed = collect_svgs(root, entries, report)

    header_path = root / HEADER_RELPATH
    header_bytes: bytes | None = None
    if header_path.is_file():
        try:
            header_bytes = header_path.read_bytes()
        except OSError as error:
            report.fail("UNREADABLE_SVG", f"{HEADER_RELPATH}: {error}")
    else:
        report.fail("STALE_HEADER", f"{HEADER_RELPATH}: missing")

    derived: tuple[bytes, bytes] | None = None
    if entries is not None:
        derived = derive_all(entries, parsed, report)

    if entries is not None and header_bytes is not None:
        rows = parse_header_symbols(header_bytes, report)
        if rows is not None:
            check_symbol_mapping(entries, rows, report)
            for position, row in enumerate(rows):
                if row["width"] != cpp_float(str(VIEW_WIDTH)) or row["height"] != cpp_float(str(VIEW_HEIGHT)):
                    report.fail("MALFORMED_HEADER", f"{HEADER_RELPATH}: symbol row {position} has non-{VIEW_WIDTH}x{VIEW_HEIGHT} dimensions")

    if derived is not None:
        committed_manifest = (root / MANIFEST_RELPATH).read_bytes()
        if committed_manifest != derived[0]:
            report.fail("STALE_MANIFEST", first_difference(committed_manifest, derived[0], MANIFEST_RELPATH))
        if header_bytes is not None and header_bytes != derived[1]:
            report.fail("STALE_HEADER", first_difference(header_bytes, derived[1], HEADER_RELPATH))

    if report.ok:
        emit_success_digests(root)
        print("verify_native_visual_kit: OK (kit reproduces byte-for-byte)")
        return 0
    for code, detail in report.failures:
        print(f"FAIL {code}: {detail}")
    print(f"verify_native_visual_kit: {len(report.failures)} failure(s)")
    return 1


REGEN_BLOCKING_CODES = frozenset(
    {
        "MISSING_MANIFEST_FILE",
        "MALFORMED_MANIFEST",
        "VERSION_MISMATCH",
        "DUPLICATE_ENTRY",
        "MISSING_SOURCE",
        "UNKNOWN_SVG_FILE",
        "UNREADABLE_SVG",
        "MALFORMED_SVG",
        "UNSAFE_SVG",
        "OUT_OF_BOUNDS_SVG",
        "INCOMPLETE_KIT",
        "NON_DETERMINISTIC_DERIVATION",
    }
)


def run_regenerate(root: Path) -> int:
    report = Report()
    entries = load_entries(root, report)
    parsed = collect_svgs(root, entries, report)

    blocking = [detail for code, detail in report.failures if code in REGEN_BLOCKING_CODES]
    if blocking or entries is None:
        for code, detail in report.failures:
            print(f"FAIL {code}: {detail}")
        print("verify_native_visual_kit: regeneration refused; fix the reported contract errors first")
        return 1

    derived = derive_all(entries, parsed, report)
    if derived is None:
        for code, detail in report.failures:
            print(f"FAIL {code}: {detail}")
        print("verify_native_visual_kit: regeneration refused; fix the reported contract errors first")
        return 1
    (root / MANIFEST_RELPATH).write_bytes(derived[0])
    (root / HEADER_RELPATH).write_bytes(derived[1])
    print(f"WROTE {MANIFEST_RELPATH}")
    print(f"WROTE {HEADER_RELPATH}")

    verification = Report()
    verify_entries = load_entries(root, verification)
    verify_parsed = collect_svgs(root, verify_entries, verification)
    if verify_entries is not None:
        verify_derived = derive_all(verify_entries, verify_parsed, verification)
        if verify_derived is not None and verify_derived != derived:
            verification.fail("NON_DETERMINISTIC_DERIVATION", "post-write re-read mismatch")
    if verification.ok:
        emit_success_digests(root)
        print("verify_native_visual_kit: regenerated cleanly")
        return 0
    for code, detail in verification.failures:
        print(f"FAIL {code}: {detail}")
    print(f"verify_native_visual_kit: regeneration produced {len(verification.failures)} failure(s)")
    return 1


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="verify_native_visual_kit.py",
        description="Verify or regenerate the native visual-kit asset contract.",
    )
    mode_group = parser.add_mutually_exclusive_group(required=True)
    mode_group.add_argument("--check", action="store_true", help="read-only validation of the committed kit")
    mode_group.add_argument("--regenerate", action="store_true", help="rewrite manifest.json and visual_kit.h from SVG sources")
    parser.add_argument("--root", type=Path, default=None, help=argparse.SUPPRESS)
    args = parser.parse_args(argv)

    root = args.root.resolve() if args.root is not None else repo_root_from_tool()
    try:
        if args.check:
            return run_check(root)
        return run_regenerate(root)
    except OSError as error:
        print(f"verify_native_visual_kit: IO error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
