#!/usr/bin/env python3
"""List the distinct per-file compilation settings of the game code.

    flag_audit.py [--scope main|overlays|all] [--markdown] [--json]

Every C file under ``src/main`` (without ``src/main/psyq``) and
``src/overlays`` is resolved by ``tools/scripts/cc.sh`` itself
(``PE_CC_PRINT_FLAGS=1``), so the audit reports what the build really runs,
not what the marker comments seem to say. Files are grouped by their
effective setting:

- the cc1 (GCC 2.7.2, or 2.8.1 for ``GCC_VERSION: 2.8.1``) and its options
  beyond the defaults ``-w -O2 -G0 -funsigned-char -mips1 -mcpu=3000``;
- the assembler back end (MASPSX + GNU as, plain GNU as, gas 2.7/2.8.1);
- the assembler ``-G`` value, MASPSX's options, its ASPSX version and
  whether MASPSX keeps its forced-G0 mode (``MASPSX_FORCE_G0``). cc.sh
  expands divisions and emulates ASPSX 2.70 by default, so only the
  exceptions (``--no-expand-div``, another ASPSX version) are shown.

Marker text that cc.sh does not act on is reported as dead.
"""
from __future__ import annotations

import argparse
import json
import os
import pathlib
import re
import subprocess
import sys
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor

ROOT = pathlib.Path(__file__).resolve().parents[2]
CC = ROOT / "tools" / "scripts" / "cc.sh"
CC1_DEFAULT = ["-w", "-O2", "-G0", "-funsigned-char", "-mips1", "-mcpu=3000"]
MARKER_RE = re.compile(
    r"/\*\s*(CC1_FLAGS|MASPSX_FLAGS|MASPSX_FORCE_G0|ASPSX_VERSION|GCC_VERSION|"
    r"GAS_VERSION|ASSEMBLER):\s*(.*?)\s*\*/")


def sources(scope: str) -> list[pathlib.Path]:
    roots = []
    if scope in ("main", "all"):
        roots.append(ROOT / "src" / "main")
    if scope in ("overlays", "all"):
        roots.append(ROOT / "src" / "overlays")
    files = []
    for root in roots:
        for path in sorted(root.rglob("*.c")):
            if "psyq" in path.relative_to(ROOT / "src").parts[1:2]:
                continue
            files.append(path)
    return files


def resolve(path: pathlib.Path) -> dict[str, str]:
    env = dict(os.environ, PE_CC_PRINT_FLAGS="1")
    out = subprocess.run(["bash", str(CC), str(path), "-"], cwd=ROOT, env=env,
                         capture_output=True, text=True, check=True).stdout
    info = {}
    for line in out.splitlines():
        key, _, value = line.partition(": ")
        info[key] = value.strip()
    return info


def markers(path: pathlib.Path) -> list[tuple[int, str, str]]:
    found = []
    for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
        for match in MARKER_RE.finditer(line):
            found.append((number, match.group(1), match.group(2)))
    return found


def setting(info: dict[str, str]) -> dict[str, object]:
    cc1_flags = info["cc1_flags"].split()
    extra = [flag for flag in cc1_flags if flag not in CC1_DEFAULT
             and not re.fullmatch(r"-G\d+", flag)]
    cc1_g = next(flag for flag in cc1_flags if re.fullmatch(r"-G\d+", flag))
    opt = next(flag for flag in cc1_flags if re.fullmatch(r"-O\d", flag))
    if opt != "-O2":
        extra.insert(0, opt)
    gcc = "2.8.1" if "281" in info["cc1"] else "2.7.2"
    result = {
        "gcc": gcc,
        "cc1_g": cc1_g,
        "cc1_extra": extra,
        "backend": info["backend"],
        "as_g": info["as_g"],
    }
    if info["backend"] == "maspsx":
        flags = info.get("maspsx_flags", "").split()
        result["aspsx"] = info["aspsx_version"]
        result["force_g0"] = info["as_g"] != "-G0" and "--dont-force-G0" not in flags
        result["maspsx"] = [flag for flag in flags
                            if flag not in ("--dont-force-G0", "--expand-div")]
        if "--expand-div" not in flags:
            result["maspsx"].append("--no-expand-div")
    return result


def key_of(s: dict[str, object]) -> str:
    parts = []
    if s["gcc"] != "2.7.2":
        parts.append(f"gcc {s['gcc']}")
    parts.append("cc1 " + " ".join([s["cc1_g"], *s["cc1_extra"]]))
    if s["backend"] == "maspsx":
        tail = [s["as_g"], *s["maspsx"]]
        if s["force_g0"]:
            tail.append("FORCE_G0")
        if s["aspsx"] != "2.70":
            tail.append(f"aspsx {s['aspsx']}")
        parts.append("maspsx " + " ".join(tail))
    else:
        parts.append(f"{s['backend']} {s['as_g']}")
    return " ; ".join(parts)


def dead_markers(path: pathlib.Path, s: dict[str, object],
                 found: list[tuple[int, str, str]]) -> list[str]:
    """Marker text cc.sh reads but that changes nothing, or never reads."""
    dead = []
    known_cc1 = {"-O1", "-O3", "-mdebuga", "-g3", "-fno-schedule-insns",
                 "-fno-schedule-insns2", "-fno-delayed-branch",
                 "-fno-strength-reduce", "-fno-force-mem",
                 "-mno-split-addresses", "-fno-expensive-optimizations",
                 "-fcall-used-$1", "-fno-gcse", "-fno-cse-follow-jumps",
                 "-fno-cse-skip-blocks", "-fno-rerun-cse-after-loop",
                 "-fno-thread-jumps"}
    known_cc1 |= {f"-ffixed-{n}" for n in (10, 11, 15, 20, 21, 22, 23, 24, 25)}
    for line, kind, value in found:
        if kind == "CC1_FLAGS":
            for flag in value.split():
                if re.fullmatch(r"-G\d+", flag):
                    if flag == "-G0":
                        dead.append(f"line {line}: CC1_FLAGS {flag} is the default")
                elif flag not in known_cc1:
                    dead.append(f"line {line}: CC1_FLAGS {flag} is not read by cc.sh")
        elif kind == "MASPSX_FLAGS":
            for flag in value.split():
                if re.fullmatch(r"-G\d+", flag):
                    if flag == "-G0":
                        dead.append(f"line {line}: MASPSX_FLAGS {flag} is the default")
                elif flag == "--expand-div":
                    dead.append(f"line {line}: MASPSX_FLAGS {flag} is the default")
                elif flag not in ("--no-expand-div", "--dont-expand-li",
                                  "--use-comm-section"):
                    dead.append(f"line {line}: MASPSX_FLAGS {flag} is not read by cc.sh")
            if s["backend"] != "maspsx":
                dead.append(f"line {line}: MASPSX_FLAGS on a {s['backend']} unit")
        elif kind == "MASPSX_FORCE_G0":
            if s["backend"] != "maspsx" or s["as_g"] == "-G0":
                dead.append(f"line {line}: MASPSX_FORCE_G0 with assembler -G0 does nothing")
        elif kind == "ASPSX_VERSION" and value == "2.70":
            dead.append(f"line {line}: ASPSX_VERSION 2.70 is the default")
    kinds = [kind for _, kind, _ in found]
    for kind in set(kinds):
        if kinds.count(kind) > 1:
            dead.append(f"{kind} written {kinds.count(kind)} times")
    return dead


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--scope", choices=("main", "overlays", "all"), default="all")
    parser.add_argument("--markdown", action="store_true",
                        help="print the tables of docs/COMPILATION_FLAGS.md")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    files = sources(args.scope)
    with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        infos = list(pool.map(resolve, files))

    rows = []
    for path, info in zip(files, infos):
        s = setting(info)
        found = markers(path)
        rows.append({
            "file": str(path.relative_to(ROOT)),
            "set": key_of(s),
            "setting": s,
            "markers": [f"{kind}: {value}" for _, kind, value in found],
            "dead": dead_markers(path, s, found),
        })

    if args.json:
        json.dump(rows, sys.stdout, indent=1)
        print()
        return 0

    groups: dict[str, list[dict]] = defaultdict(list)
    for row in rows:
        groups[row["set"]].append(row)
    order = sorted(groups, key=lambda k: (-len(groups[k]), k))
    default = key_of(setting({"cc1": "cc1_stock272psx.sh",
                              "cc1_flags": " ".join(CC1_DEFAULT),
                              "backend": "maspsx", "as_g": "-G0",
                              "aspsx_version": "2.70",
                              "maspsx_flags": "--expand-div"}))

    def area(file: str) -> str:
        return "main" if file.startswith("src/main/") else "overlays"

    if args.markdown:
        print("| # | effective setting | main | overlays |")
        print("|---|---|---|---|")
        for number, key in enumerate(order, 1):
            members = groups[key]
            label = f"`{key}`" + (" (default)" if key == default else "")
            main_n = sum(area(r["file"]) == "main" for r in members)
            print(f"| S{number} | {label} | {main_n} | {len(members) - main_n} |")
        print()
        print(f"{len(order)} distinct settings over {len(rows)} files.")
        print()
        for number, key in enumerate(order, 1):
            if key == default:
                continue
            print(f"#### S{number}: `{key}`")
            print()
            for row in groups[key]:
                print(f"- `{row['file']}`")
            print()
    else:
        for number, key in enumerate(order, 1):
            print(f"S{number} [{len(groups[key])}] {key}"
                  + (" (default)" if key == default else ""))
            if key != default:
                for row in groups[key]:
                    print(f"    {row['file']}")
        print(f"{len(order)} distinct settings over {len(rows)} files")

    dead = [(row["file"], d) for row in rows for d in row["dead"]]
    if dead:
        print()
        print("Dead or repeated marker text:" if not args.markdown
              else "#### Dead or repeated marker text\n")
        for file, d in dead:
            print(f"- `{file}`: {d}" if args.markdown else f"    {file}: {d}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
