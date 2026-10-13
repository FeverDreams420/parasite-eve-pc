#!/usr/bin/env python3
"""Compare a (merged) source against the objects of the units it replaces.

    marker_probe.py SOURCE.c --ref OBJ [--ref OBJ ...] [-v] [-d FUNC]
    marker_probe.py --gp OBJ [OBJ ...]

The first form compiles SOURCE.c with ``tools/scripts/cc.sh`` into a scratch
object and prints the Levenshtein distance over instruction words for every
function of the reference objects (normally the retail-matching
``build/USA/src/main/<unit>.c.o`` files, copied away before the merge).
Relocations count as part of the word; addends of section symbols
(``.rodata+0x40``, local ``jal`` targets) are ignored because they move when
units are concatenated, so a link check (``make check``) stays the final
word. ``-d FUNC`` prints a diff of one function.

The second form lists, per object, the words reached through ``$gp``
(``R_MIPS_GPREL16``) and the ones addressed absolutely (``HI16``/``LO16``),
and flags words that one unit reaches through ``$gp`` and another
absolutely: those decide which tentative (COMMON) definitions a merged unit
may carry.
"""
from __future__ import annotations

import argparse
import difflib
import os
import pathlib
import re
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
OBJDUMP = "mipsel-none-elf-objdump"
SECTION_SYMBOLS = {".text", ".rodata", ".data", ".sdata", ".bss", ".sbss"}


def disasm(obj: str) -> dict[str, list[str]]:
    out = subprocess.run([OBJDUMP, "-dr", "--no-show-raw-insn", "-j", ".text", obj],
                         capture_output=True, text=True, check=True).stdout
    funcs: dict[str, list[str]] = {}
    current = None
    for line in out.splitlines():
        match = re.match(r"^[0-9a-f]+ <(.+)>:", line)
        if match:
            current = match.group(1)
            funcs[current] = []
            continue
        if current is None:
            continue
        match = re.match(r"^\s+[0-9a-f]+:\s+(R_\S+)\s+(\S+)", line)
        if match and funcs[current]:
            kind, target = match.groups()
            base = re.split(r"[+-]0x", target)[0]
            if base in SECTION_SYMBOLS:
                target = base
            word = funcs[current][-1]
            word = re.sub(r"(?<![\w$])-?(0x[0-9a-f]+|\d+)(?=\(|$)", "R", word)
            funcs[current][-1] = f"{word} @{kind}:{target}"
            continue
        match = re.match(r"^\s+[0-9a-f]+:\s+(.*)", line)
        if match:
            word = re.sub(r"\s+", " ", match.group(1).strip())
            word = re.sub(r"\b[0-9a-f]+ <([^>+]*)(\+0x[0-9a-f]+)?>",
                          lambda m: "<" + (m.group(2) or "+0") + ">", word)
            funcs[current].append(word)
    return funcs


def lev(a: list[str], b: list[str]) -> int:
    prev = list(range(len(b) + 1))
    for i, x in enumerate(a, 1):
        cur = [i] + [0] * len(b)
        for j, y in enumerate(b, 1):
            cur[j] = min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + (x != y))
        prev = cur
    return prev[-1]


def compile_source(source: str, out: str) -> None:
    result = subprocess.run(["bash", "tools/scripts/cc.sh", source, out], cwd=ROOT,
                            capture_output=True, text=True, errors="replace")
    if result.returncode or not os.path.exists(out):
        sys.stderr.write(result.stdout + result.stderr)
        sys.exit(f"compile failed: {source}")


def gp_report(objs: list[str]) -> int:
    usage: dict[str, dict[str, set[str]]] = {}
    for obj in objs:
        out = subprocess.run([OBJDUMP, "-r", "-j", ".text", obj], capture_output=True,
                             text=True, check=True).stdout
        gp, absolute = set(), set()
        for line in out.splitlines():
            parts = line.split()
            if len(parts) != 3 or not parts[1].startswith("R_MIPS_"):
                continue
            sym = re.split(r"[+-]0x", parts[2])[0]
            if sym in SECTION_SYMBOLS:
                continue
            if parts[1] == "R_MIPS_GPREL16":
                gp.add(sym)
            elif parts[1] in ("R_MIPS_HI16", "R_MIPS_LO16"):
                absolute.add(sym)
        usage[obj] = {"gp": gp, "abs": absolute}
        print(f"{obj}\n  gp:  {' '.join(sorted(gp)) or '-'}")
    all_gp = set().union(*(u["gp"] for u in usage.values()))
    clashes = 0
    for obj, u in usage.items():
        both = sorted(u["abs"] & all_gp)
        if both:
            clashes += len(both)
            print(f"CLASH {obj}: absolute {' '.join(both)}")
    print(f"$gp words: {' '.join(sorted(all_gp)) or '-'}")
    return 1 if clashes else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("source", nargs="?")
    parser.add_argument("--ref", action="append", default=[])
    parser.add_argument("--gp", nargs="+")
    parser.add_argument("-v", action="store_true")
    parser.add_argument("-d", metavar="FUNC")
    args = parser.parse_args()
    if args.gp:
        return gp_report(args.gp)
    if not args.source or not args.ref:
        parser.error("SOURCE and --ref are required")

    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, "probe.o")
        compile_source(args.source, out)
        new = disasm(out)
    ref: dict[str, list[str]] = {}
    for obj in args.ref:
        ref.update(disasm(obj))
    total = 0
    if list(new) != [f for f in new if f in ref] or set(ref) - set(new):
        missing = [f for f in ref if f not in new]
        extra = [f for f in new if f not in ref]
        if missing or extra:
            print(f"function sets differ: missing {missing} extra {extra}")
        order = [f for f in new if f in ref]
        if order != [f for f in ref if f in new]:
            print("function order differs")
    for name, words in ref.items():
        if name not in new:
            total += len(words)
            continue
        distance = lev(words, new[name])
        total += distance
        if distance or args.v:
            print(f"{name}: lev {distance}")
        if args.d == name:
            sys.stdout.writelines(difflib.unified_diff(
                [w + "\n" for w in words], [w + "\n" for w in new[name]],
                "ref", "new"))
    print(f"{args.source}: {len(ref)} functions, total lev {total}")
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
