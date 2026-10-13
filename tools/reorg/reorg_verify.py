#!/usr/bin/env python3
"""Static checks after ``reorg_move.py``: nothing refers to the old paths.

Usage: ``reorg_verify.py MAP.csv [MAP.csv ...] [--old-dir DIR ...]``

Checks, for every ``old,new`` row:

* the old path is gone from the index and the new path is tracked;
* no tracked text file (outside the reorganisation docs and ``tools/reorg/``)
  still names the old path in a path-qualified form (``dir/stem``,
  ``src/main/dir/stem.c``, ``main/dir/stem`` ...);
* ``--old-dir``: the directory has no tracked files and no mention as
  ``src/main/DIR/`` remains.

Then, tree-wide:

* every quoted ``#include`` in ``src/`` resolves (relative, ``include/`` or
  ``tools/m2c/``);
* the manifest/source contract holds (``tools/scripts/check_c_subseg_sources.py``).

Byte identity is not checked here; run the make gates for that.
"""
from __future__ import annotations

import argparse
import csv
import os
import pathlib
import re
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import reorg_move as rm  # noqa: E402

ROOT = rm.ROOT
INCLUDE_DIRS = ("include", "tools/m2c")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("mapping", nargs="+")
    parser.add_argument("--old-dir", action="append", default=[])
    args = parser.parse_args()

    moves: dict[str, str] = {}
    for path in args.mapping:
        with open(path, newline="") as handle:
            for row in csv.reader(handle):
                if row and row[0] != "old" and not row[0].startswith("#"):
                    moves[rm.norm(row[0])] = rm.norm(row[1])

    errors: list[str] = []
    tracked = rm.tracked_files()
    tracked_set = set(tracked)
    for old, new in moves.items():
        if old in tracked_set:
            errors.append(f"still tracked: {old}")
        if new not in tracked_set:
            errors.append(f"not tracked: {new}")
    for old_dir in args.old_dir:
        old_dir = rm.norm(old_dir)
        left = [p for p in tracked if p.startswith(old_dir + "/")]
        if left:
            errors.append(f"{old_dir} still has {len(left)} tracked files")

    rules = rm.token_rules(moves)
    for path in tracked:
        if rm.is_excluded(path) or pathlib.PurePath(path).suffix not in rm.TEXT_SUFFIXES:
            continue
        file = ROOT / path
        if not file.is_file() or file.is_symlink():
            continue
        try:
            text = file.read_text()
        except UnicodeDecodeError:
            continue
        for pattern, old_root, *_ in rules:
            for match in pattern.finditer(text):
                if rm.prefix_ok(match["prefix"], old_root):
                    line = text.count("\n", 0, match.start()) + 1
                    errors.append(f"{path}:{line}: stale {match.group(0)}")
        for old_dir in args.old_dir:
            for match in re.finditer(rm.DIR_MENTION.format(
                    re.escape(rm.norm(old_dir))), text):
                line = text.count("\n", 0, match.start()) + 1
                errors.append(f"{path}:{line}: stale directory {old_dir}/")
        if path.startswith("src/") and path.endswith((".c", ".h", ".inc")):
            for number, line in enumerate(text.splitlines(), 1):
                match = rm.INCLUDE.match(line)
                if not match:
                    continue
                inc = match["path"]
                candidates = [os.path.join(os.path.dirname(path), inc)] + [
                    os.path.join(d, inc) for d in INCLUDE_DIRS]
                if not any((ROOT / c).is_file() for c in candidates):
                    errors.append(f"{path}:{number}: unresolved include {inc}")

    result = subprocess.run(
        [sys.executable, "tools/scripts/check_c_subseg_sources.py"],
        cwd=ROOT, text=True, capture_output=True)
    if result.returncode:
        errors.append("check_c_subseg_sources.py failed:\n" + result.stdout
                      + result.stderr)

    for error in errors:
        print(error)
    print(f"{len(moves)} moves checked, {len(errors)} problems")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
