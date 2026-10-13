#!/usr/bin/env python3
"""Check (or --fix) the #include directives of every .c and .h in src/ and include/.

Rules (docs/CONVENTIONS.md, "Includes"):

- every header include sits in one block at the top of the file, after the
  marker block, the file comment and (in a header) the include guard, and
  before any declaration or code;
- an include that has to follow code or a #define stays where it is only with
  a one-line comment directly above it that starts with `Late include:` and
  says why (`/* Late include: ... */`);
- `.inc` files are function templates expanded in place after the #defines
  that configure them; they may appear anywhere and more than once;
- no header is included twice in one file, whatever path spells it;
- headers are spelled in quotes by their canonical path: `common.h` and
  `pe1/...` from include/, `m2c_macros.h` from tools/m2c, local headers
  relative to the including file;
- the block is ordered `common.h`, the other include-path headers (`pe1/...`,
  the PsyQ `pe1/psyq_*.h` among them, then `include_asm.h` and
  `m2c_macros.h`) alphabetically, then a blank line and the local headers
  alphabetically;
- every header has an include guard (`#ifndef X` / `#define X` as its first
  directives, `#endif` as its last).

Include order can change which declarations and macros the compiler sees
first, so `--fix` only rewrites; the build decides whether the result is
byte-identical.
"""
from __future__ import annotations

import argparse
import os
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
INCLUDE_DIRS = [ROOT / "include", ROOT / "tools/m2c", ROOT / "sdk/psyq-4.0/PSX/INCLUDE"]
INCLUDE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"](.*)$')
DIRECTIVE = re.compile(r"^\s*#\s*(\w*)")
LATE = re.compile(r"^\s*/\*\s*Late include:.*\*/\s*$")
GUARD_IFNDEF = re.compile(r"^\s*#\s*ifndef\s+(\w+)\s*$")
GUARD_DEFINE = re.compile(r"^\s*#\s*define\s+(\w+)\s*$")


def sources() -> list[pathlib.Path]:
    files: list[pathlib.Path] = []
    for base in (ROOT / "src", ROOT / "include"):
        files += base.rglob("*.c")
        files += base.rglob("*.h")
    return sorted(files)


class Line:
    __slots__ = ("raw", "kind", "directive", "include", "ctx")

    def __init__(self, raw: str):
        self.raw = raw
        self.kind = "blank"      # blank | comment | pp | code
        self.directive = ""
        self.include = None      # (quote, name, trailing text)
        self.ctx: tuple = ()


def scan(text: str) -> list[Line]:
    """Classify each line; track the #if branch context of directives."""
    lines = [Line(raw) for raw in text.split("\n")]
    in_comment = False
    continued = False
    stack: list[int] = []
    branch = 0
    for line in lines:
        raw = line.raw
        if continued:
            line.kind = "pp"
            continued = raw.rstrip().endswith("\\")
            line.ctx = tuple(stack)
            continue
        visible = []
        i = 0
        while i < len(raw):
            if in_comment:
                end = raw.find("*/", i)
                if end < 0:
                    i = len(raw)
                else:
                    in_comment = False
                    i = end + 2
            elif raw.startswith("/*", i):
                in_comment = True
                i += 2
            elif raw.startswith("//", i):
                break
            else:
                visible.append(raw[i])
                i += 1
        stripped = "".join(visible).strip()
        line.ctx = tuple(stack)
        if not stripped:
            line.kind = "comment" if raw.strip() else "blank"
            continue
        if stripped.startswith("#"):
            line.kind = "pp"
            continued = raw.rstrip().endswith("\\")
            match = DIRECTIVE.match(stripped)
            word = match.group(1) if match else ""
            line.directive = word
            if word in ("if", "ifdef", "ifndef"):
                branch += 1
                stack.append(branch)
            elif word in ("elif", "else"):
                branch += 1
                if stack:
                    stack[-1] = branch
            elif word == "endif" and stack:
                stack.pop()
            included = INCLUDE.match(raw)
            if included:
                line.include = included.groups()
            continue
        line.kind = "code"
    return lines


def guard_depth(path: pathlib.Path, lines: list[Line]) -> int | None:
    """1 when a header's first directives are its include guard, else None."""
    directives = [l for l in lines if l.kind == "pp"]
    if len(directives) < 3:
        return None
    first = GUARD_IFNDEF.match(directives[0].raw)
    second = GUARD_DEFINE.match(directives[1].raw)
    if not (first and second and first.group(1) == second.group(1)):
        return None
    if directives[-1].directive != "endif":
        return None
    if any(l.kind == "code" for l in lines[:lines.index(directives[0])]):
        return None
    return 1


def resolve(path: pathlib.Path, quote: str, name: str) -> pathlib.Path | None:
    candidates = []
    if quote == '"':
        candidates.append(path.parent / name)
    candidates += [base / name for base in INCLUDE_DIRS]
    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()
    return None


def canonical(path: pathlib.Path, quote: str, name: str):
    """(canonical spelling, group, resolved path) of one include."""
    target = resolve(path, quote, name)
    if target is None:
        return name, 2, None
    for base in INCLUDE_DIRS[:2]:
        base = base.resolve()
        if target.is_relative_to(base):
            spelled = target.relative_to(base).as_posix()
            if spelled == "common.h":
                return spelled, 0, target
            if spelled.startswith("pe1/"):
                return spelled, 1, target
            return spelled, 2, target
    spelled = os.path.relpath(target, path.resolve().parent)
    return pathlib.Path(spelled).as_posix(), 3, target


def sort_key(item):
    spelled, group = item
    return (group, spelled.lower(), spelled)


class Include:
    def __init__(self, index, line, path):
        self.index = index
        self.line = line
        quote, name, self.trailing = line.include
        self.template = name.endswith(".inc")
        self.spelled, self.group, self.target = canonical(path, quote, name)
        self.quote, self.name = quote, name


def analyse(path: pathlib.Path, text: str):
    lines = scan(text)
    depth = guard_depth(path, lines) if path.suffix == ".h" else 0
    base = depth or 0
    first_code = next((i for i, l in enumerate(lines) if l.kind == "code"), len(lines))
    includes = [Include(i, l, path) for i, l in enumerate(lines) if l.include]
    for inc in includes:
        inc.late = inc.index > first_code
        prev = inc.index - 1
        while prev >= 0 and lines[prev].kind == "blank":
            prev -= 1
        inc.marked = prev >= 0 and bool(LATE.match(lines[prev].raw))
        inc.conditional = len(inc.line.ctx) > base
    return lines, depth, first_code, includes


def errors_for(path: pathlib.Path, text: str) -> list[str]:
    lines, depth, first_code, includes = analyse(path, text)
    errors = []
    if path.suffix == ".h" and depth is None:
        errors.append("header without an include guard")
    seen: dict = {}
    for inc in includes:
        number = inc.index + 1
        if inc.template:
            continue
        if inc.quote != '"' or inc.name != inc.spelled:
            errors.append(f"line {number}: spell it #include \"{inc.spelled}\"")
        if inc.late and not inc.marked:
            errors.append(f"line {number}: #include after code"
                          " (move it to the top block or say why: /* Late include: ... */)")
        key = inc.target or inc.spelled
        for other in seen.get(key, []):
            a, b = other.line.ctx, inc.line.ctx
            if a[:len(b)] == b or b[:len(a)] == a:
                errors.append(f"line {number}: {inc.spelled} already included on line"
                              f" {other.index + 1}")
                break
        seen.setdefault(key, []).append(inc)
    block = [inc for inc in includes
             if not inc.template and not inc.late and not inc.conditional]
    if block:
        want = sorted(dict.fromkeys((inc.spelled, inc.group) for inc in block), key=sort_key)
        have = [(inc.spelled, inc.group) for inc in block]
        if have != want:
            errors.append("top includes are not in order (common.h, include-path headers,"
                          " local headers; alphabetical)")
        start, end = block[0].index, block[-1].index
        for i in range(start, end + 1):
            if lines[i].kind not in ("blank",) and not lines[i].include:
                errors.append(f"line {i + 1}: top include block is interrupted")
                break
    return errors


def fix(path: pathlib.Path, text: str) -> tuple[str, list[str]]:
    """Hoist, deduplicate, respell and sort the header includes."""
    lines, depth, first_code, includes = analyse(path, text)
    notes = []
    movable = [inc for inc in includes
               if not inc.template and not inc.conditional and not (inc.late and inc.marked)]
    if not movable:
        return text, notes
    top = [inc for inc in movable if not inc.late]
    if top:
        insert = top[0].index
    else:
        # After the leading comments, markers and guard.
        insert = 0
        guard_seen = 0
        for i, line in enumerate(lines):
            if line.kind in ("blank", "comment"):
                insert = i + 1
                continue
            if depth and line.kind == "pp" and guard_seen < 2 and line.directive in ("ifndef", "define"):
                guard_seen += 1
                insert = i + 1
                continue
            break
        while insert > 0 and lines[insert - 1].kind == "blank":
            insert -= 1
    moving = {inc.index for inc in movable}
    for inc in movable:
        if inc.late:
            crossed = [l for l in lines[insert:inc.index]
                       if l.kind == "pp" and l.directive in ("define", "undef") and not l.include]
            if crossed:
                notes.append(f"line {inc.index + 1}: {inc.spelled} moves above"
                             f" {len(crossed)} #define/#undef lines")
            prev = inc.index - 1
            while prev >= 0 and lines[prev].kind == "blank":
                prev -= 1
            if prev >= 0 and lines[prev].kind == "comment" and prev not in moving:
                notes.append(f"line {inc.index + 1}: comment above the moved include stays behind")
    entries = []
    for inc in movable:
        entries.append((inc.spelled, inc.group, inc.trailing.rstrip()))
    chosen: dict = {}
    for spelled, group, trailing in entries:
        if spelled not in chosen or (trailing and not chosen[spelled][1]):
            chosen[spelled] = (group, trailing)
    ordered = sorted(((s, g) for s, (g, _) in chosen.items()), key=sort_key)
    block_lines = []
    previous_group = None
    for spelled, group in ordered:
        if previous_group is not None and (group == 3) != (previous_group == 3):
            block_lines.append("")
        block_lines.append(f'#include "{spelled}"{chosen[spelled][1]}')
        previous_group = group
    out: list[str | None] = [l.raw for l in lines]
    # Drop the old top block (includes and the blank lines between them).
    if top:
        last_top = max(inc.index for inc in top)
        for i in range(insert, last_top + 1):
            if lines[i].kind == "blank" or i in moving:
                out[i] = None
    for inc in movable:
        out[inc.index] = None
    result: list[str] = []
    dropped = False
    for i, raw in enumerate(out):
        if i == insert:
            if result and result[-1] != "":
                result.append("")
            result += block_lines
            nxt = next((r for r in out[i:] if r is not None), None)
            if nxt is not None and nxt != "":
                result.append("")
            dropped = True
        if raw is None:
            dropped = True
            continue
        # Collapse only the blank lines a removed include leaves behind.
        if raw == "" and dropped and result and result[-1] == "":
            continue
        if raw != "":
            dropped = False
        result.append(raw)
    if insert >= len(out):
        result += block_lines
    new = "\n".join(result)
    return new, notes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--fix", action="store_true")
    parser.add_argument("files", nargs="*", type=pathlib.Path)
    args = parser.parse_args()
    files = []
    for given in args.files:
        given = given.resolve()
        if given.is_dir():
            files += sorted(p for p in given.rglob("*") if p.suffix in (".c", ".h"))
        else:
            files.append(given)
    files = files or sources()
    failed = 0
    for path in files:
        text = path.read_text()
        rel = path.relative_to(ROOT)
        if args.fix:
            new, notes = fix(path, text)
            if new != text:
                path.write_text(new)
                text = new
            for note in notes:
                print(f"{rel}: note: {note}")
        for error in errors_for(path, text):
            failed += 1
            print(f"{rel}: {error}")
    if failed:
        return 1
    print(f"OK: the includes of {len(files)} sources are tidy.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
