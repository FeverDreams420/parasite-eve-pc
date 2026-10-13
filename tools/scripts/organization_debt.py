#!/usr/bin/env python3
"""Reject growth in opaque or unconventional source filenames.

Address and placeholder filenames are valid migration state, but new work must
not make that state larger. Counts are scoped so mass-generated overlays cannot
hide regressions in the resident executable.

Game units in ``src/main`` (outside ``psyq/``) are named like the original
developers would name C files (docs/CONVENTIONS.md): lower snake case, short,
named by the unit's role inside its subsystem directory, without the
directory's or the functions' prefix (``battle/encounter.c``, not
``battle/Battle_EncounterFlow.c`` or ``battle/battle_encounter.c``). Names
that break this count as ``unconventional_files``. The shared room library
(``src/overlays/room_lib``) follows the same rule and is counted in the
overlays scope; the other overlay directories keep their names for now.
"""
from __future__ import annotations

import argparse
import json
import pathlib
import re


ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
BASELINE = ROOT / "tools" / "organization_debt_baseline.json"

ADDRESS_NAME = re.compile(r"func_[0-9A-Fa-f]{6,}$")
PLACEHOLDER_NAME = re.compile(
    r"(?:misc|main|task|entity|seq|spu|akao|menu|engine|gpu|psyq)\d*$",
    re.IGNORECASE,
)
SNAKE_NAME = re.compile(r"[a-z][a-z0-9]*(?:_[a-z0-9]+)*")


def defines_function(path: pathlib.Path, name: str) -> bool:
    """The unit defines a function called `name` (boot/main.c defines main)."""
    pattern = r"^[A-Za-z_][\w \t\*]*\b%s\s*\([^;]*$" % re.escape(name)
    return re.search(pattern, path.read_text(errors="ignore"), re.M) is not None


def unconventional(rel: pathlib.PurePath) -> bool:
    """A src/main game unit whose file name is not a short snake-case role."""
    if rel.parts[1] == "psyq":
        return False  # SDK object names (docs/CONVENTIONS.md, PsyQ file names)
    if len(rel.parts) != 3:
        return True  # a game unit sits in exactly one subsystem directory
    directory, stem = rel.parts[1], rel.stem
    return not SNAKE_NAME.fullmatch(stem) or stem.startswith(directory + "_")


def unconventional_shared(rel: pathlib.PurePath) -> bool:
    """A room_lib unit whose file name is not a short snake-case role."""
    stem = rel.stem
    return not SNAKE_NAME.fullmatch(stem) or stem.startswith(rel.parts[1] + "_")


def collect(source_root: pathlib.Path = SRC) -> dict[str, dict[str, int]]:
    counts = {
        "main": {"address_files": 0, "placeholder_files": 0,
                 "unconventional_files": 0},
        "overlays": {"address_files": 0, "placeholder_files": 0,
                     "unconventional_files": 0},
    }
    for path in source_root.rglob("*.c"):
        rel = path.relative_to(source_root)
        scope = "overlays" if rel.parts and rel.parts[0] == "overlays" else "main"
        if scope == "main" and unconventional(rel):
            counts[scope]["unconventional_files"] += 1
        if rel.parts[:2] == ("overlays", "room_lib") and unconventional_shared(rel):
            counts[scope]["unconventional_files"] += 1
        if ADDRESS_NAME.fullmatch(path.stem):
            counts[scope]["address_files"] += 1
        elif (PLACEHOLDER_NAME.fullmatch(path.stem) and "psyq" not in rel.parts
              and not defines_function(path, path.stem)):
            # PsyQ units carry SDK object names (LIBSPU SPU is spu.c), and a
            # unit named after the function it defines (boot/main.c) is not a
            # placeholder.
            counts[scope]["placeholder_files"] += 1
    return counts


def check(counts, baseline_path: pathlib.Path = BASELINE) -> int:
    baseline = json.loads(baseline_path.read_text())
    increases = []
    decreases = []
    for scope, metrics in counts.items():
        for metric, value in metrics.items():
            before = baseline.get(scope, {}).get(metric, 0)
            if value > before:
                increases.append((scope, metric, before, value))
            elif value < before:
                decreases.append((scope, metric, before, value))
    if increases:
        print("Organization debt increased:")
        for scope, metric, before, after in increases:
            print(f"  {scope}.{metric}: {before} -> {after}")
        return 1
    if decreases:
        print("Organization debt improved, but the ratchet baseline is stale:")
        for scope, metric, before, after in decreases:
            print(f"  {scope}.{metric}: {before} -> {after}")
        print("Run `make organization-baseline` and commit the lowered baseline.")
        return 1
    print("OK: opaque and unconventional source filename counts match the ratchet baseline.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--update-baseline", action="store_true")
    args = parser.parse_args()
    counts = collect()
    if args.update_baseline:
        BASELINE.write_text(json.dumps(counts, indent=2, sort_keys=True) + "\n")
        print(f"wrote {BASELINE}")
        return 0
    if args.json or not args.check:
        print(json.dumps(counts, indent=2, sort_keys=True))
    return check(counts) if args.check else 0


if __name__ == "__main__":
    raise SystemExit(main())
