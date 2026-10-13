#!/usr/bin/env python3
"""Check (or --fix) the per-file compilation markers of the game code.

tools/scripts/cc.sh greps a whole `.c` file for its markers, so a marker can
hide anywhere and be spelled many ways. In `src/main` (without
`src/main/psyq`) and `src/overlays` every unit must instead declare its
settings in one block at the very top of the file:

    /* GCC_VERSION: 2.8.1 */
    /* CC1_FLAGS: -G8 -g3 -fno-schedule-insns */
    /* ASSEMBLER: GNU */
    /* MASPSX_FLAGS: -G8 --use-comm-section --no-expand-div */
    /* MASPSX_FORCE_G0: 1 */
    /* ASPSX_VERSION: 2.56 */

- one marker per line, in this order, each at most once, followed by a
  blank line; no marker text anywhere else in the file;
- `-G<n>` first in its marker, then the options in the order listed in
  CC1_OPTIONS / MASPSX_OPTIONS below, no repeats;
- only options cc.sh acts on, and no defaults written out (`-G0`,
  `--expand-div`, ASPSX 2.70); MASPSX_FORCE_G0 only with an assembler -G,
  MASPSX markers only on MASPSX units.

cc.sh expands divisions (`--expand-div`) and emulates ASPSX 2.70 unless a
unit says otherwise (`--no-expand-div`, `ASPSX_VERSION: 2.56`).

`--fix` rewrites files into that form (it only moves and respells markers;
the build decides whether the result is byte-identical). docs/CONVENTIONS.md
describes the markers, docs/COMPILATION_FLAGS.md what each one does.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
KINDS = ["GCC_VERSION", "CC1_FLAGS", "ASSEMBLER", "GAS_VERSION", "MASPSX_FLAGS",
         "MASPSX_FORCE_G0", "ASPSX_VERSION"]
# Options cc.sh passes on, in the canonical order.
CC1_OPTIONS = ["-O1", "-O3", "-g3", "-mdebuga", "-fno-schedule-insns",
               "-fno-schedule-insns2", "-fno-delayed-branch", "-fno-strength-reduce",
               "-fno-force-mem", "-fno-expensive-optimizations", "-fno-gcse",
               "-fno-cse-follow-jumps", "-fno-cse-skip-blocks",
               "-fno-rerun-cse-after-loop", "-fno-thread-jumps",
               "-mno-split-addresses", "-fcall-used-$1"] + [
    f"-ffixed-{n}" for n in (10, 11, 15, 20, 21, 22, 23, 24, 25)]
MASPSX_OPTIONS = ["--use-comm-section", "--no-expand-div", "--dont-expand-li"]
# Written defaults: cc.sh applies them to every unit, so the marker says nothing.
DEFAULTS = {"-G0", "--expand-div"}
DEFAULT_ASPSX = "2.70"
ANY_MARKER = re.compile(r"\b(" + "|".join(KINDS) + r")\s*:")
LINE = re.compile(r"^/\* (" + "|".join(KINDS) + r"): (\S.*?) \*/$")
LOOSE = re.compile(r"^\s*/\*\s*(" + "|".join(KINDS) + r")\s*:\s*(.*?)\s*\*/\s*$")
G = re.compile(r"-G(\d+)")


def game_sources() -> list[pathlib.Path]:
    files = sorted((ROOT / "src/overlays").rglob("*.c"))
    files += [p for p in sorted((ROOT / "src/main").rglob("*.c"))
              if p.relative_to(ROOT / "src/main").parts[0] != "psyq"]
    return files


def canonical(markers: dict[str, list[str]]) -> tuple[list[str], list[str]]:
    """(canonical marker lines, errors) for the collected marker values."""
    errors, lines = [], []
    values = {k: " ".join(v).split() for k, v in markers.items()}
    backend = "maspsx"
    if "ASSEMBLER" in values or "GAS_VERSION" in values:
        backend = "gnu"
    as_g = None
    for kind in KINDS:
        if kind not in values:
            continue
        tokens = values[kind]
        if kind in ("CC1_FLAGS", "MASPSX_FLAGS"):
            known = CC1_OPTIONS if kind == "CC1_FLAGS" else MASPSX_OPTIONS
            gs = [t for t in tokens if G.fullmatch(t)]
            rest = [t for t in tokens if not G.fullmatch(t)]
            if len(set(gs)) > 1:
                errors.append(f"{kind} has several -G values: {gs}")
            unknown = [t for t in rest if t not in known and t not in DEFAULTS]
            if unknown:
                errors.append(f"{kind} options cc.sh does not read: {unknown}")
            out = [g for g in dict.fromkeys(gs) if g not in DEFAULTS]
            out += [o for o in known if o in rest]
            if kind == "MASPSX_FLAGS":
                if backend != "maspsx":
                    errors.append("MASPSX_FLAGS on a GNU as unit")
                as_g = next((g for g in out if G.fullmatch(g)), None)
            if out:
                lines.append(f"/* {kind}: {' '.join(out)} */")
        elif kind == "MASPSX_FORCE_G0":
            if tokens != ["1"]:
                errors.append(f"MASPSX_FORCE_G0 must be 1, not {tokens}")
            if as_g is None:
                errors.append("MASPSX_FORCE_G0 without an assembler -G does nothing")
            else:
                lines.append("/* MASPSX_FORCE_G0: 1 */")
        elif kind == "ASPSX_VERSION":
            if backend != "maspsx":
                errors.append("ASPSX_VERSION on a GNU as unit")
            if tokens != [DEFAULT_ASPSX]:
                lines.append(f"/* ASPSX_VERSION: {' '.join(tokens)} */")
        else:
            lines.append(f"/* {kind}: {' '.join(tokens)} */")
    return lines, errors


def check(path: pathlib.Path, fix: bool) -> list[str]:
    text = path.read_text()
    lines = text.split("\n")
    markers: dict[str, list[str]] = {}
    marker_rows = []
    errors = []
    for number, line in enumerate(lines):
        if not ANY_MARKER.search(line):
            continue
        match = LOOSE.match(line)
        if not match:
            errors.append(f"line {number + 1}: marker text outside a marker line")
            continue
        markers.setdefault(match.group(1), []).append(match.group(2))
        marker_rows.append(number)
    want, problems = canonical(markers)
    if any(len(v) > 1 for v in markers.values()):
        problems.append("a marker is written more than once")
    rest = [l for i, l in enumerate(lines) if i not in set(marker_rows)]
    while (want or marker_rows) and rest and rest[0] == "":
        rest.pop(0)
    expected = "\n".join(want + ([""] if want else []) + rest)
    if expected != text:
        if fix and not errors:
            path.write_text(expected)
        else:
            problems.append("markers are not one canonical block at the top"
                            " (tools/scripts/check_flag_markers.py --fix)")
    if fix:
        problems = [p for p in problems if "canonical block" not in p
                    and "more than once" not in p and "does nothing" not in p
                    and "-G values" not in p]
    return errors + problems


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--fix", action="store_true")
    parser.add_argument("files", nargs="*", type=pathlib.Path)
    args = parser.parse_args()
    files = [f.resolve() for f in args.files] or game_sources()
    failed = 0
    for path in files:
        for error in check(path, args.fix):
            failed += 1
            print(f"{path.relative_to(ROOT)}: {error}")
    if failed:
        return 1
    print(f"OK: flag markers of {len(files)} game units are canonical.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
