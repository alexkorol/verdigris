#!/usr/bin/env python3
"""Emit the 200-ID VG↔TASK registry appendix. Planning IDs stay DRAFT; no TASK mint."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
GOALS = ROOT / "docs/execution/pack/backlog/ATOMIC_GOALS.md"
OUT = ROOT / "docs/execution/CROSSWALK_REGISTRY.md"

CURSOR = {
    "VG-GOV-001": ("new", "BASELINE.md working manifest; package hash unset"),
    "VG-GOV-002": ("new", "owner ruling still required; lanes draft only"),
    "VG-GOV-003": ("extend", "parity scorecard draft; feature-count cannot pass; vital-orbs HUD journey"),
    "VG-GOV-004": ("extend", "this CROSSWALK; TASK-0166/0205 — do not mint"),
    "VG-GOV-005": ("extend", "software sample is the GPU trial; gpu-sample is not an engine port"),
    "VG-GOV-006": ("extend", "death-disconnect HUD; silent extract ack on disconnect cannot pass; core D-106 stays Kimi"),
    "VG-GOV-008": ("new", "pack tools/roadmap.py validate + unittest"),
    "VG-UI-001": ("extend", "pane-stack native Escape; helper depth alone cannot prove; close hint stays in slot"),
    "VG-UI-002": ("extend", "pack-drag occupancy; reject cannot lose/duplicate/silent-equip; sim inventory-move stays Kimi"),
    "VG-UI-003": ("extend", "equipment compare plate is ack-only; pending cannot gold-frame as equipped"),
    "VG-UI-004": ("extend", "stat-explain expandable ATK; dormant cannot fold into Attack; core STAT stays Kimi; close hint stays in slot"),
    "VG-UI-005": ("extend", "route-map zoom/opacity; off-snapshot blip cannot pass; Owner Demo journeys not reimplemented"),
    "VG-UI-006": ("extend", "Owner Demo — do not duplicate"),
    "VG-UI-007": ("extend", "hud-scale-floor + hud-pane-readability + vital-orbs roles; close hint pinned"),
    "VG-UI-008": ("new", "pad-path XInput on tick"),
    "VG-ART-001": ("extend", "visual-target in-game sheet"),
    "VG-ART-002": ("extend", "bronze-stone family"),
    "VG-ART-003": ("extend", "attack-poses; not TASK-0108"),
    "VG-ART-004": ("extend", "kit-chunk village kit"),
    "VG-ART-005": ("extend", "held-item world attachment; paper-doll seat alone cannot pass"),
    "VG-ART-006": ("extend", "weave-vfx; not TASK-0108"),
    "VG-ART-007": ("extend", "Owner Demo — do not duplicate"),
    "VG-ART-008": ("extend", "Owner Demo — do not duplicate"),
    "VG-GPU-001": ("new", "gpu-sample"),
    "VG-GPU-002": ("new", "gpu-packets"),
    "VG-GPU-003": ("new", "shader-bindings"),
    "VG-GPU-004": ("new", "gpu-reference"),
    "VG-GPU-005": ("new", "grounding"),
    "VG-GPU-006": ("new", "material-light"),
    "VG-GPU-007": ("new", "gpu-capture + PNG R/B honesty; swapped still cannot certify"),
    "VG-GPU-008": ("new", "gpu-recover"),
    "VG-SOUND-001": ("new", "sound-adapter"),
    "VG-SOUND-002": ("new", "legal-sounds"),
    "VG-SOUND-003": ("extend", "combat-audio event-id dedup"),
    "VG-SOUND-004": ("extend", "combat-audio warning priority"),
    "VG-SOUND-005": ("extend", "ambience-layer"),
    "VG-SOUND-006": ("new", "audio-prefs"),
    "VG-SOUND-007": ("new", "dense-mix"),
    "VG-SOUND-008": ("new", "music-phase"),
    "VG-PERF-001": ("extend", "frame-budget named machine + paint sections"),
    "VG-PERF-003": ("new", "effect-batch"),
    "VG-PERF-004": ("new", "resource-envelope"),
    "VG-PERF-005": ("new", "loot-label-budget"),
    "VG-PERF-006": ("new", "hitch-warmup"),
    "VG-PERF-007": ("new", "memory-soak"),
    "VG-PERF-008": ("extend", "Owner Demo — do not duplicate"),
    "VG-MOVE-005": ("extend", "pane-focus TASK-0165 wired in client tick"),
    "VG-MOVE-006": ("new", "remap-binds isolated profile; owner Documents refused"),
    "VG-MOVE-008": ("new", "input-latency p50/p95 input-to-present; command time is not photon"),
    "VG-MOVE-001": ("new", "eight-way wire names; vertical-only encoder cannot pass a diagonal"),
    "VG-MOVE-002": ("new", "aim-hold: move must not clobber held aim; core still overwrites without adapter"),
    "VG-ACT-007": ("new", "attack-beat event bridge; swing sprites cannot mint a beat"),
    "VG-WORLD-008": ("new", "dressing-pass; tree visuals cannot mint solids"),
    "VG-ITEM-006": ("new", "loot-filter presentation facts; hide cannot mutate ownership"),
    "VG-BUILD-001": ("new", "character-sheet slice fixtures; tinted melee clones fail; core STAT/BUILD stays Kimi"),
    "VG-QA-001": ("new", "pack evidence_manifest.py; screenshot without provenance cannot certify; native/tests stays Kimi"),
    "VG-QA-002": ("new", "headless-contract AttackStarted to swing/audio; mock events cannot prove; native/tests stays Kimi"),
    "VG-ACT-005": ("new", "telegraph-spec catalog ticks+reach; durationMs/50 cannot invent a window"),
}

KIMI = {
    "CORE", "MOVE", "ACT", "STAT", "BUILD", "ITEM", "FORGE", "SAVE",
    "HOUSE", "WORLD", "ENEMY", "STORY", "END", "NET", "SEC", "TOOLS",
    "QA", "SHIP", "LIVE",
}

OWNER = "TASK-0145, 0177, 0178, 0197, 0203, 0205–0207 Owner Demo — do not duplicate"


def parse_goals(text: str) -> list[dict[str, str]]:
    blocks = re.split(r"\n### \[ \] ", text)[1:]
    rows = []
    for block in blocks:
        first = block.split("\n", 1)[0]
        vg = first.split()[0].strip()
        gate = ""
        m = re.search(r"\*\*Gate:\*\*\s*(\S+)", block)
        if m:
            gate = m.group(1)
        packets = ""
        m = re.search(r"\*\*Existing packets:\*\*\s*(.+)", block)
        if m:
            packets = m.group(1).strip()
        rows.append({"id": vg, "gate": gate, "packets": packets})
    return rows


def family(vg: str) -> str:
    return vg.split("-")[1]


def disposition(row: dict[str, str]) -> tuple[str, str]:
    vg = row["id"]
    if vg in CURSOR:
        return CURSOR[vg]
    fam = family(vg)
    packets = row["packets"]
    if "Owner Demo" in packets or vg in {"VG-GOV-007"}:
        return "extend", OWNER
    if packets.startswith("TASK-"):
        note = packets
        if "0108" in packets:
            note += " — extend, never re-spec TASK-0108"
        if fam in KIMI:
            note += " — Kimi lease"
        return "extend", note
    if fam in KIMI:
        return "new", "Kimi lease (`native/src/**` / tools); current-head search, no absence claim"
    if fam == "PERF" and vg == "VG-PERF-002":
        return "new", "Kimi/core tick budget; not a GPU claim"
    return "new", packets


def main() -> None:
    rows = parse_goals(GOALS.read_text(encoding="utf-8"))
    if len(rows) != 200:
        raise SystemExit(f"expected 200 goals, got {len(rows)}")
    lines = [
        "# VG planning ID registry (200)",
        "",
        "Draft companion to `CROSSWALK.md`. Planning IDs do **not** become",
        "TASK numbers. Generated from `pack/backlog/ATOMIC_GOALS.md`.",
        "Cursor scenario notes are evidence on this lease; Kimi rows are",
        "not implemented here.",
        "",
        "| VG | Gate | Disposition | Existing / lease |",
        "|---|---|---|---|",
    ]
    for row in rows:
        disp, note = disposition(row)
        note = note.replace("|", "/")
        lines.append(f"| {row['id']} | {row['gate']} | {disp} | {note} |")
    lines.append("")
    lines.append(f"Registry rows: {len(rows)}. No TASK identifiers were minted.")
    lines.append("")
    OUT.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {OUT} ({len(rows)} rows)")


if __name__ == "__main__":
    main()
