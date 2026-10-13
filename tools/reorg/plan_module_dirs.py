#!/usr/bin/env python3
"""Emit the remaining cross-directory moves of step 1c as an ``old,new`` CSV.

Reads ``docs/reorganization_mapping.csv`` (old file -> new module), follows
the moves already recorded in ``tools/reorg/maps/*.csv`` to find each file's
current path, and prints a move into the module's directory for every file
that is not there yet. The file name is kept; only the directory changes.

    plan_module_dirs.py [--from-dir DIR] > maps/step1c_xxx.csv
    reorg_move.py maps/step1c_xxx.csv
"""
from __future__ import annotations

import argparse
import csv
import os
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--from-dir", help="only files currently in this directory")
    args = parser.parse_args()

    moved: dict[str, str] = {}
    for path in sorted((ROOT / "tools/reorg/maps").glob("*.csv")):
        with open(path, newline="") as handle:
            for row in csv.DictReader(handle):
                moved[row["old"]] = row["new"]

    def current(path: str) -> str:
        seen = set()
        while path in moved and path not in seen:
            seen.add(path)
            path = moved[path]
        return path

    writer = csv.writer(sys.stdout, lineterminator="\n")
    writer.writerow(["old", "new"])
    with open(ROOT / "docs/reorganization_mapping.csv", newline="") as handle:
        for row in csv.DictReader(handle):
            old = current(row["old"])
            if not (ROOT / old).is_file():
                print(f"missing: {old}", file=sys.stderr)
                continue
            target_dir = os.path.dirname(row["new_module"])
            if os.path.dirname(old) == target_dir:
                continue
            if args.from_dir and os.path.dirname(old) != args.from_dir.rstrip("/"):
                continue
            writer.writerow([old, f"{target_dir}/{os.path.basename(old)}"])
    return 0


if __name__ == "__main__":
    sys.exit(main())
