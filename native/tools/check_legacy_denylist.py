"""Reject denied legacy identifiers in new native production sources.

The native gate intentionally does not scan the historical browser tree. It
does scan source and data/configuration formats that can be copied into the
native product, and it compares identifier tokens rather than raw spelling so
that ``startingCoins``, ``starting-coins``, and ``STARTING_COINS`` are treated
as the same identifier.
"""

from __future__ import annotations

import argparse
import fnmatch
import json
import pathlib
import re
from collections.abc import Iterable, Mapping, Sequence


ROOT = pathlib.Path(__file__).resolve().parents[2]
CONFIG = ROOT / "config" / "legacy-denylist.json"
NATIVE = ROOT / "native"
EXCLUDED_PARTS = {"tests", "tools"}

# C/C++ plus the native data/config formats called out by the archaeology
# audit. Markdown and Python are deliberately not production data formats;
# the checker itself is under native/tools and is excluded above.
EXTENSIONS = frozenset(
    {
        ".c",
        ".cc",
        ".cmake",
        ".cpp",
        ".cxx",
        ".h",
        ".hpp",
        ".js",
        ".json",
        ".ps1",
        ".toml",
        ".txt",
        ".yaml",
        ".yml",
    }
)

_ACRONYM_BOUNDARY = re.compile(r"(?<=[A-Z])(?=[A-Z][a-z])")
_CAMEL_BOUNDARY = re.compile(r"(?<=[a-z0-9])(?=[A-Z])")
_NON_IDENTIFIER = re.compile(r"[^A-Za-z0-9]+")


def identifier_tokens(value: str) -> tuple[str, ...]:
    """Return case-folded identifier words for source or a denylist term."""

    separated = _ACRONYM_BOUNDARY.sub(" ", value)
    separated = _CAMEL_BOUNDARY.sub(" ", separated)
    separated = _NON_IDENTIFIER.sub(" ", separated)
    return tuple(part.casefold() for part in separated.split() if part)


def _compact(tokens: Sequence[str]) -> str:
    return "".join(tokens)


def _term_matches(tokens: Sequence[str], term_tokens: Sequence[str]) -> bool:
    """Match a token sequence, including a fully joined identifier.

    A token-sequence match catches separators and camel/Pascal transitions.
    The compact comparison catches a lowercase joined spelling such as
    ``bronzedagger``. Both comparisons are token-boundary aware: ``ore`` does
    not match the unrelated word ``score``.
    """

    if not term_tokens or not tokens:
        return False
    width = len(term_tokens)
    compact_term = _compact(term_tokens)
    for index in range(len(tokens) - width + 1):
        window = tuple(tokens[index : index + width])
        if window == tuple(term_tokens):
            return True
        if width > 1 and tokens[index] == compact_term:
            return True
    return any(token == compact_term for token in tokens)


def _normalised_terms(values: Iterable[str]) -> tuple[tuple[str, tuple[str, ...]], ...]:
    terms: list[tuple[str, tuple[str, ...]]] = []
    seen: set[tuple[str, ...]] = set()
    for value in values:
        tokens = identifier_tokens(value)
        if tokens and tokens not in seen:
            terms.append((value, tokens))
            seen.add(tokens)
    return tuple(terms)


def _path_matches(path: str, pattern: str) -> bool:
    pattern = pattern.replace("\\", "/")
    return path == pattern or fnmatch.fnmatchcase(path, pattern)


def _allowlisted_terms(
    path: str, allowlist: Sequence[Mapping[str, object]]
) -> set[tuple[str, ...]]:
    allowed: set[tuple[str, ...]] = set()
    for entry in allowlist:
        raw_path = entry.get("path")
        raw_identifiers = entry.get("identifiers")
        reason = entry.get("reason")
        if (
            not isinstance(raw_path, str)
            or not isinstance(raw_identifiers, list)
            or not isinstance(reason, str)
            or not reason.strip()
        ):
            raise ValueError(
                "allowlist entries require path, identifiers, and a non-empty reason"
            )
        if _path_matches(path, raw_path):
            for identifier in raw_identifiers:
                if not isinstance(identifier, str):
                    raise ValueError("allowlist identifiers must be strings")
                tokens = identifier_tokens(identifier)
                if tokens:
                    allowed.add(tokens)
    return allowed


def matches_text(
    text: str,
    path: str,
    identifiers: Sequence[str],
    allowlist: Sequence[Mapping[str, object]] = (),
) -> list[str]:
    """Return denied terms found in ``text`` unless explicitly allowlisted."""

    tokens = identifier_tokens(text)
    allowed = _allowlisted_terms(path, allowlist)
    matches: list[str] = []
    for term, term_tokens in _normalised_terms(identifiers):
        if term_tokens in allowed:
            continue
        if _term_matches(tokens, term_tokens):
            matches.append(term)
    return matches


def _load_config() -> dict[str, object]:
    config = json.loads(CONFIG.read_text(encoding="utf-8"))
    if not isinstance(config, dict):
        raise ValueError("denylist config must be an object")
    identifiers = config.get("identifiers")
    allowlist = config.get("allowlist", [])
    if not isinstance(identifiers, list) or not all(
        isinstance(value, str) for value in identifiers
    ):
        raise ValueError("denylist identifiers must be a list of strings")
    if not isinstance(allowlist, list):
        raise ValueError("denylist allowlist must be a list")
    return config


def _is_exempt(path: pathlib.Path, config: Mapping[str, object]) -> bool:
    relative = path.relative_to(ROOT).as_posix()
    native_relative = path.relative_to(NATIVE).as_posix()
    if any(part in EXCLUDED_PARTS for part in path.relative_to(NATIVE).parts):
        return True
    exemptions = config.get("exemptions", [])
    if not isinstance(exemptions, list):
        raise ValueError("denylist exemptions must be a list")
    return any(
        isinstance(pattern, str)
        and (
            _path_matches(relative, pattern.rstrip("/"))
            or relative.startswith(pattern)
            or _path_matches(native_relative, pattern.rstrip("/"))
        )
        for pattern in exemptions
    )


def scan_tree() -> list[str]:
    config = _load_config()
    identifiers = config["identifiers"]
    allowlist = config.get("allowlist", [])
    assert isinstance(identifiers, list)
    assert isinstance(allowlist, list)
    failures: list[str] = []
    for path in sorted(NATIVE.rglob("*")):
        if not path.is_file() or path.suffix.casefold() not in EXTENSIONS:
            continue
        if _is_exempt(path, config):
            continue
        relative = path.relative_to(ROOT).as_posix()
        text = path.read_text(encoding="utf-8", errors="replace")
        for term in matches_text(text, relative, identifiers, allowlist):
            failures.append(f"{relative} contains denied identifier {term!r}")
    return failures


def _self_test(inject_variant: bool) -> int:
    config = _load_config()
    identifiers = config["identifiers"]
    allowlist = config.get("allowlist", [])
    assert isinstance(identifiers, list)
    assert isinstance(allowlist, list)

    variants = (
        ("bronzeDagger", "bronze dagger"),
        ("BRONZE_DAGGER", "bronze dagger"),
        ("bronze-dagger", "bronze dagger"),
        ("bronzedagger", "bronze dagger"),
        ("startingCoins", "starting coins"),
        ("StartingCoins", "starting coins"),
        ("starting-coins", "starting coins"),
        ("STARTING_COINS", "starting coins"),
        ("zone:fenmire", "fenmire"),
        ("dungeon:barrow-depths", "barrow depths"),
        ("town:old-wood", "old wood"),
        ("legacyRelicId", "legacy relic id"),
        ("legacyTile", "legacy tile"),
        ("LEGACY_MODE", "legacy mode"),
    )
    for fixture, expected_term in variants:
        found = matches_text(fixture, "native/self-test/fixture.txt", identifiers, allowlist)
        if expected_term not in found:
            print(f"self-test: expected denied variant was missed: {fixture}")
            return 1

    if matches_text("score scoreboard", "native/self-test/negative.txt", identifiers, allowlist):
        print("self-test: generic token false positive")
        return 1

    # Exercise the explicit path/identifier allowlist contract independently
    # of the current (intentionally empty) production exception set.
    synthetic_allowlist = [
        {
            "path": "native/self-test/allowed.txt",
            "identifiers": ["ore"],
            "reason": "fixture-only proof of a scoped, justified exception",
        }
    ]
    if matches_text("ore", "native/self-test/allowed.txt", identifiers, synthetic_allowlist):
        print("self-test: explicit allowlist was not honored")
        return 1
    if not matches_text("ore", "native/self-test/not-allowed.txt", identifiers, synthetic_allowlist):
        print("self-test: allowlist leaked outside its path")
        return 1

    if inject_variant:
        failures = matches_text(
            "a deliberately injected bronzeDagger variant",
            "native/self-test/injected.txt",
            identifiers,
            allowlist,
        )
        print("self-test negative fixture: expected failure")
        for term in failures:
            print(f"native/self-test/injected.txt contains denied identifier {term!r}")
        return 1 if failures else 0

    print("legacy denylist self-test: PASS")
    return 0


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true", help="run matcher fixtures")
    parser.add_argument(
        "--inject-variant",
        "--negative-fixture",
        dest="inject_variant",
        action="store_true",
        help="make --self-test report a deliberate denied fixture (exit 1)",
    )
    args = parser.parse_args(argv)
    if args.inject_variant and not args.self_test:
        parser.error("--inject-variant requires --self-test")
    if args.self_test:
        return _self_test(args.inject_variant)

    failures = scan_tree()
    if failures:
        print("\n".join(failures))
        return 1
    print("native legacy denylist: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
