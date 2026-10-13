#!/usr/bin/env python3
"""Rename project symbols everywhere in the tree, or audit function prefixes.

``rename_symbols.py OLD NEW [OLD NEW ...]`` renames each symbol OLD to NEW
(functions, types, globals, macros, struct members, test names alike).
``rename_symbols.py --prefix Task_ Script_ [--dir src/main/script]`` renames
every symbol spelled ``Task_*`` (only those defined under ``--dir`` when it
is given) to ``Script_*``. ``--map FILE`` reads ``OLD NEW`` lines.
``--header OLD NEW`` renames the header ``include/pe1/OLD.h`` (or any tracked
header whose path ends in ``/OLD.h``): the file moves, its include guard
``PE1_OLD_H`` becomes ``PE1_NEW_H``, and only path mentions of the header
(``pe1/OLD.h``, a bare ``OLD.h``) change, so a header named like a common word
(``sys.h``) does not rename that word, nor ``psyq/libcd/sys.c``.

A rename is a whole-word substitution (``\\bOLD\\b``) over every tracked text
file: the splat symbol files of the executable and the overlays, the manual
undefined-symbol list, yaml manifests, C sources, headers, ``.inc`` bodies,
docs, ``docs/match_inventory.json`` and the tests. A tracked file whose stem
is a renamed symbol (``Task_ValueOps.c``, ``OLD_externs.h``) moves with it,
and its manifest and path mentions follow through the same substitution.

Historical records are left alone: the reorganization maps under
``tools/reorg/maps``, ``docs/reorganization_mapping.csv`` and
``docs/REORGANIZATION*.md`` describe moves that already happened.

The script refuses, before touching anything:

* a NEW that already occurs as a word anywhere in those files (or as a file
  stem) unless the same batch renames it away;
* an OLD that is a PsyQ name: defined by a ``PSYQ_ASM_*`` macro, or named as
  an SDK label in ``configs/USA/psyq_provenance.json``;
* an OLD that the build looks up by name outside the symbol files (a
  Makefile, ``tools/scripts`` or a linker script naming it).

``--audit`` checks every external function in ``src/main`` outside ``psyq/``:
its prefix must be one its directory lists in the subsystem table of
docs/CONVENTIONS.md, or one another live (not dissolved) subsystem lists
(the function serves that subsystem from a unit the binary places here;
``--verbose`` lists those), or the name must be in AUDIT_EXCEPTIONS. It exits
1 on any other function. It also checks the other names of the game code
(``--names`` runs only this part): types and ``g_`` globals declared in
``include/pe1`` (without ``psyq_*.h``), ``src/main`` (without ``psyq/``) and
``src/overlays``, the headers of ``include/pe1`` and the tests in
``tools/tests``, and the ``Old_*`` functions the function audit does not see
(static inline helpers, ``__asm__`` aliases). A name must not start with the prefix of a dissolved or
renamed module (``Task``, ``FieldEng``, ``FieldAnim``, ``Tbl``, ``Evt``, ...,
read from the subsystem table: the dissolved rows' prefixes and the "renamed
from `dir`, prefix from `Old_`" notes), nor a header or test with that
module's directory name (``task_*.h``, ``test_task_*.py``), unless
NAME_EXCEPTIONS lists it with a reason.
"""
from __future__ import annotations

import argparse
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
TEXT_SUFFIXES = {".c", ".h", ".inc", ".txt", ".yaml", ".yml", ".json", ".md",
                 ".py", ".csv", ".ld", ".mk", ".sh", ".toml", ""}
HISTORY = ("tools/reorg/maps/", "docs/reorganization_mapping.csv",
           "docs/REORGANIZATION")
SELF = "tools/reorg/rename_symbols.py"
BUILD_LOOKUP = ("Makefile", "tools/scripts/")
WORD = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")

# External functions in src/main that carry no subsystem prefix on purpose,
# with the reason. PsyQ units (src/main/psyq) are not audited: they keep SDK
# labels, and private SDK routines keep the project names they were given.
AUDIT_EXCEPTIONS = {
    "main": "the C entry point; boot owns it",
}

# Names that start with an old module name on purpose, keyed by the full name
# or by "<kind> <old module>" (kind: type, global, function, header, test).
# GTE and GPU are also the hardware, and libgpu's private routines keep their
# Gpu_ project names (they are defined under psyq/ and pass on their own).
NAME_EXCEPTIONS = {
    "type Gte": "GTE hardware types (pe1/gte.h), not the dissolved gte module",
    "header gte": "GTE hardware headers (gte.h, gte_types.h, ...)",
    "test gte": "tests of GTE register reads and LIBGTE ratan",
    "type Gpu": "GPU hardware packets, registers and libgpu state",
    "global Gpu": "libgpu DMA state words",
    "header gpu": "libgpu-side headers (psyq/libgpu is their only user but gpu_image.h)",
    "test gpu": "tests of psyq/libgpu routines",
    "test overlay": "overlay link tooling test",
    "g_StrFileDirBuffer": "STR movie file directory, not the Str_ text prefix",
    "g_StrFileDirLba": "STR movie file directory, not the Str_ text prefix",
}


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=True, text=True,
                          capture_output=True).stdout


def tracked_text_files() -> list[str]:
    files = []
    for path in git("ls-files").splitlines():
        p = ROOT / path
        if path == SELF or path.startswith(HISTORY):
            continue
        if p.is_symlink() or not p.is_file():
            continue
        if p.suffix not in TEXT_SUFFIXES and p.name != "Makefile":
            continue
        files.append(path)
    return files


def read(path: str) -> str | None:
    try:
        return (ROOT / path).read_text()
    except UnicodeDecodeError:
        return None


DEF_RX = re.compile(
    r"^(?!\s|#|/|\*|}|typedef\b|extern\b|return\b)[^;=()]*?\b([A-Za-z_]\w*)\s*\(([^;]*)$")
ASM_DEF_RX = re.compile(r"^(GAME_ASM_FUNCTION|PSYQ_ASM_FUNCTION|PSYQ_ASM_OBJECT)\(\s*(\w+)")


def function_definitions(files: list[str]) -> dict[str, list[tuple[str, str]]]:
    """name -> [(file, kind)] for every function defined in src/ C files."""
    defs: dict[str, list[tuple[str, str]]] = {}
    for path in files:
        if not path.startswith("src/") or not path.endswith((".c", ".inc")):
            continue
        text = read(path) or ""
        lines = text.splitlines()
        for i, line in enumerate(lines):
            m = ASM_DEF_RX.match(line)
            if m:
                defs.setdefault(m.group(2), []).append((path, m.group(1)))
                continue
            m = DEF_RX.match(line)
            if not m:
                continue
            # a definition: the parameter list closes and a body opens.
            j, tail = i, line
            while ")" not in tail.split("(", 1)[-1] and j + 1 < len(lines):
                j += 1
                tail += lines[j]
            rest = tail[tail.rfind(")") + 1:].strip()
            if rest.startswith(";") or (not rest.startswith("{") and
                                        not (j + 1 < len(lines) and lines[j + 1].startswith("{"))):
                continue
            kind = "static" if re.match(r"(static|inline|__inline__)\b", line) else "c"
            if line.rstrip().endswith("\\") or m.group(1).endswith("_FUNC"):
                kind = "macro"
            defs.setdefault(m.group(1), []).append((path, kind))
    return defs


def psyq_names() -> set[str]:
    names: set[str] = set()
    prov = ROOT / "configs/USA/psyq_provenance.json"
    if prov.exists():
        def walk(node):
            if isinstance(node, dict):
                for key, value in node.items():
                    if key in ("symbol", "symbols", "labels", "label", "sdk_symbol",
                               "public", "publics", "name") and value is not None:
                        for item in (value if isinstance(value, list) else [value]):
                            if isinstance(item, str):
                                names.add(item)
                            elif isinstance(item, dict):
                                walk(item)
                    else:
                        walk(value)
            elif isinstance(node, list):
                for item in node:
                    walk(item)
        walk(json.loads(prov.read_text()))
    return names


def collect_prefix(files: list[str], old: str, new: str, dirs: list[str],
                   defs) -> list[tuple[str, str]]:
    rx = re.compile(r"\b" + re.escape(old) + r"[A-Za-z0-9_]+\b")
    names: set[str] = set()
    for path in files:
        text = read(path)
        if text:
            names.update(rx.findall(text))
    for path in git("ls-files").splitlines():
        stem = pathlib.PurePath(path).name.split(".")[0]
        if stem.startswith(old) and not path.startswith(HISTORY):
            names.add(stem)
    pairs = []
    for name in sorted(names):
        if dirs:
            owners = {f for f, _ in defs.get(name, [])}
            stems = {p for p in git("ls-files").splitlines()
                     if pathlib.PurePath(p).name.split(".")[0] == name}
            owners |= stems
            if not owners or not all(any(o.startswith(d.rstrip("/") + "/") for d in dirs)
                                     for o in owners):
                continue
        pairs.append((name, new + name[len(old):]))
    return pairs


def rename(pairs: list[tuple[str, str]], dry_run: bool) -> int:
    files = tracked_text_files()
    texts = {p: read(p) for p in files}
    texts = {p: t for p, t in texts.items() if t is not None}
    tracked = git("ls-files").splitlines()
    olds = {o for o, _ in pairs}
    if len(olds) != len(pairs) or len({n for _, n in pairs}) != len(pairs):
        sys.exit("duplicate OLD or NEW in the rename list")
    words: set[str] = set()
    for text in texts.values():
        words.update(WORD.findall(text))
    stems = {pathlib.PurePath(p).name.split(".")[0] for p in tracked}
    defs = function_definitions(files)
    sdk = psyq_names()
    errors = []
    for old, new in pairs:
        if not WORD.fullmatch(old) or not WORD.fullmatch(new):
            errors.append(f"{old} -> {new}: not an identifier")
        if (new in words or new in stems) and new not in olds:
            errors.append(f"{old} -> {new}: {new} already exists")
        if old in sdk or any(k.startswith("PSYQ") for _, k in defs.get(old, [])):
            errors.append(f"{old}: PsyQ name, not renamed")
        for path in files:
            if (path.startswith(BUILD_LOOKUP) or path.endswith((".ld", ".mk"))) and re.search(r"\b" + old + r"\b", texts[path]):
                errors.append(f"{old}: named by build tooling in {path}")
    if errors:
        print("refused:\n  " + "\n  ".join(errors))
        return 1

    table = dict(pairs)
    rx = re.compile(r"\b(" + "|".join(sorted(map(re.escape, olds), key=len, reverse=True)) + r")\b")
    changed: dict[str, int] = {}
    for path, text in texts.items():
        new_text, n = rx.subn(lambda m: table[m.group(1)], text)
        if n:
            changed[path] = n
            if not dry_run:
                (ROOT / path).write_text(new_text)
    moves = []
    for path in tracked:
        pp = pathlib.PurePath(path)
        stem, _, ext = pp.name.partition(".")
        for old, new in pairs:
            if stem == old or stem.startswith(old + "_externs"):
                moves.append((path, str(pp.with_name(new + stem[len(old):] + ("." + ext if ext else "")))))
                break
    for src, dst in moves:
        if (ROOT / dst).exists():
            sys.exit(f"cannot move {src}: {dst} exists")
        if not dry_run:
            git("mv", src, dst)

    print(f"{'would rename' if dry_run else 'renamed'} {len(pairs)} symbols:")
    for old, new in pairs:
        where = sorted({f for f, _ in defs.get(old, [])})
        print(f"  {old} -> {new}" + (f"  [{', '.join(where)}]" if where else ""))
    print(f"{sum(changed.values())} substitutions in {len(changed)} files:")
    for path in sorted(changed):
        print(f"  {changed[path]:5d}  {path}")
    if moves:
        print(f"{len(moves)} files moved:")
        for src, dst in moves:
            print(f"  {src} -> {dst}")
    return 0


def old_module_names() -> tuple[set[str], set[str]]:
    """(CamelCase prefixes, snake directory names) of dissolved or renamed modules."""
    text = (ROOT / "docs/CONVENTIONS.md").read_text()
    camel: set[str] = set()
    snake: set[str] = set()
    for line in text.splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) != 4 or not re.fullmatch(r"`[a-z0-9_]+`", cells[0]):
            continue
        directory, note = cells[0].strip("`"), cells[3]
        if note.startswith("dissolved") and directory != "main":
            snake.add(directory)
            camel.update(p.rstrip("_") for p in re.findall(r"`([A-Za-z0-9]+_)`", cells[1]))
        m = re.search(r"renamed from `([a-z0-9_]+)`", note)
        if m:
            snake.add(m.group(1))
        m = re.search(r"prefix(?:es)? from ((?:`[A-Za-z0-9]+_`(?:,| and| or)?\s*)+)", note)
        if m:
            camel.update(p.rstrip("_") for p in re.findall(r"`([A-Za-z0-9]+_)`", m.group(1)))
    if "engine" in snake:
        snake.add("field_engine")
    if "FieldAnim" in camel:
        snake.add("field_anim")
    return camel, snake


TYPE_RX = re.compile(r"\}\s*(\w+)\s*;|\btypedef\b[^;{}]*?\b(\w+)\s*(?:\[[^\]]*\]\s*)*;|"
                     r"\(\s*\*\s*(\w+)\s*\)\s*\(|\b(?:struct|union|enum)\s+(\w+)\s*\{")


def audit_names(verbose: bool) -> int:
    """Flag types, globals, functions, headers and tests named after an old module.

    Functions are caught here when the function audit cannot see them: static
    inline helpers in headers and ``__asm__`` aliases of an address."""
    camel, snake = old_module_names()
    sdk = psyq_names()
    defs = function_definitions(tracked_text_files())

    def old_camel(name: str) -> str | None:
        for token in camel:
            if re.match(re.escape(token) + r"(?:[A-Z0-9_]|$)", name):
                return token
        return None

    def old_snake(stem: str) -> str | None:
        for token in sorted(snake, key=len, reverse=True):
            if stem == token or stem.startswith(token + "_"):
                return token
        return None

    found: dict[tuple[str, str], set[str]] = {}
    for path in tracked_text_files():
        parts = pathlib.PurePath(path).parts
        game = (parts[:2] == ("src", "overlays") or
                (parts[:2] == ("src", "main") and parts[2:3] != ("psyq",)) or
                (parts[:2] == ("include", "pe1") and not parts[-1].startswith("psyq_")))
        if game and path.endswith((".c", ".h", ".inc")):
            text = read(path) or ""
            for groups in TYPE_RX.findall(text):
                name = next(g for g in groups if g)
                if old_camel(name) and name not in sdk:
                    found.setdefault(("type", name), set()).add(path)
            for name in re.findall(r"\bg_(\w+)", text):
                if old_camel(name):
                    found.setdefault(("global", "g_" + name), set()).add(path)
            for name in re.findall(r"\b([A-Z][A-Za-z0-9]*_\w+)\s*\(", text):
                token = old_camel(name)
                if (token and name[len(token)] == "_" and name not in sdk and
                        not any(f.startswith("src/main/psyq/") for f, _ in defs.get(name, []))):
                    found.setdefault(("function", name), set()).add(path)
        if parts[:2] == ("include", "pe1") and path.endswith(".h") and not parts[-1].startswith("psyq_"):
            stem = pathlib.PurePath(path).stem
            if old_snake(stem):
                found.setdefault(("header", stem), set()).add(path)
        if parts[:2] == ("tools", "tests") and parts[-1].startswith("test_") and path.endswith(".py"):
            stem = pathlib.PurePath(path).stem[len("test_"):]
            if old_snake(stem):
                found.setdefault(("test", "test_" + stem), set()).add(path)
    bad = 0
    for (kind, name), paths in sorted(found.items()):
        bare = name[2:] if kind == "global" else name
        token = old_camel(bare) if kind in ("type", "global", "function") else old_snake(
            bare[len("test_"):] if kind == "test" else bare)
        where = ", ".join(sorted(paths)[:2]) + (" ..." if len(paths) > 2 else "")
        reason = NAME_EXCEPTIONS.get(name) or NAME_EXCEPTIONS.get(f"{kind} {token}")
        if reason:
            if verbose:
                print(f"  ok {kind} {name}  -- {reason}")
            continue
        bad += 1
        print(f"  !! {kind} {name}  (old module `{token}`; {where})")
    print(f"{bad} types, globals, functions, headers or tests named after an old module")
    return 1 if bad else 0


def header_pairs(pairs: list[tuple[str, str]]) -> tuple[list[tuple[str, str]], list[tuple[str, str]]]:
    """(guard renames, header moves) for --header OLD NEW pairs."""
    tracked = git("ls-files").splitlines()
    guards, moves = [], []
    for old, new in pairs:
        hits = [p for p in tracked if p.endswith("/" + old + ".h")]
        if len(hits) != 1:
            sys.exit(f"--header {old}: {len(hits)} tracked headers named {old}.h")
        dst = str(pathlib.PurePath(hits[0]).with_name(new + ".h"))
        if (ROOT / dst).exists():
            sys.exit(f"--header {old}: {dst} exists")
        moves.append((hits[0], dst))
        guards.append((f"PE1_{old.upper()}_H", f"PE1_{new.upper()}_H"))
    return guards, moves


def rename_headers(pairs: list[tuple[str, str]], dry_run: bool) -> int:
    guards, moves = header_pairs(pairs)
    texts = {p: read(p) for p in tracked_text_files()}
    rxs = []
    for (old, new), (src, _) in zip(pairs, moves):
        rel = src.split("include/", 1)[1] if src.startswith("include/") else old + ".h"
        rxs.append((re.compile(r"(?<![\w./-])(" + re.escape(rel) + r"|" + re.escape(old + ".h") + r")\b"),
                    lambda m, rel=rel, old=old, new=new: m.group(1).replace(old + ".h", new + ".h")))
    for guard_old, guard_new in guards:
        rxs.append((re.compile(r"\b" + guard_old + r"\b"), lambda m, g=guard_new: g))
    changed = {}
    for path, text in texts.items():
        if text is None:
            continue
        new_text, total = text, 0
        for rx, fn in rxs:
            new_text, n = rx.subn(fn, new_text)
            total += n
        if total:
            changed[path] = total
            if not dry_run:
                (ROOT / path).write_text(new_text)
    if not dry_run:
        for src, dst in moves:
            git("mv", src, dst)
    print(f"{'would rename' if dry_run else 'renamed'} {len(moves)} headers:")
    for src, dst in moves:
        print(f"  {src} -> {dst}")
    print(f"{sum(changed.values())} substitutions in {len(changed)} files:")
    for path in sorted(changed):
        print(f"  {changed[path]:5d}  {path}")
    return 0


def conventions_table() -> dict[str, tuple[list[str], bool]]:
    """directory -> (prefixes, dissolved) from the subsystem table."""
    text = (ROOT / "docs/CONVENTIONS.md").read_text()
    table: dict[str, tuple[list[str], bool]] = {}
    for line in text.splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) != 4 or not re.fullmatch(r"`[a-z0-9_]+`", cells[0]):
            continue
        table[cells[0].strip("`")] = (re.findall(r"`([A-Za-z0-9]+_)`", cells[1]),
                                      cells[3].startswith("dissolved"))
    return table


def audit(verbose: bool) -> int:
    """Check external function prefixes in src/main against CONVENTIONS.md.

    A function may carry its own directory's prefix, or the prefix of another
    live subsystem whose responsibility it serves while the binary places it
    in this unit (listed with --verbose). A prefix of a dissolved subsystem,
    or one that no subsystem owns, needs an AUDIT_EXCEPTIONS entry.
    """
    files = tracked_text_files()
    defs = function_definitions(files)
    table = conventions_table()
    live = {p for prefixes, dissolved in table.values() if not dissolved for p in prefixes}
    bad = 0
    cross: dict[str, int] = {}
    for name, where in sorted(defs.items()):
        for path, kind in where:
            parts = pathlib.PurePath(path).parts
            if parts[:2] != ("src", "main") or len(parts) < 4 or parts[2] == "psyq":
                continue
            if kind in ("static", "macro") or re.fullmatch(r"func_[0-9A-F]{8}", name):
                continue
            d = parts[2]
            own = table.get(d, ([], False))[0]
            m = re.match(r"[A-Za-z0-9]+_", name)
            prefix = m.group(0) if m else ""
            if prefix in own:
                continue
            if name in AUDIT_EXCEPTIONS:
                if verbose:
                    print(f"  ok {d}: {name}  -- {AUDIT_EXCEPTIONS[name]}")
                continue
            if prefix in live:
                cross[d] = cross.get(d, 0) + 1
                if verbose:
                    print(f"  cross {d}: {name}  ({path})")
                continue
            bad += 1
            print(f"  !! {d}: {name}  ({path})")
    print("functions serving another live subsystem, per directory: " +
          (", ".join(f"{d} {n}" for d, n in sorted(cross.items())) or "none"))
    print(f"{bad} prefix mismatches without a documented exception")
    return 1 if bad or audit_names(verbose) else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("pairs", nargs="*", metavar="OLD NEW")
    parser.add_argument("--prefix", nargs=2, metavar=("OLD_", "NEW_"))
    parser.add_argument("--dir", action="append", default=[],
                        help="with --prefix: only symbols defined under this directory")
    parser.add_argument("--exclude", action="append", default=[],
                        help="with --prefix: leave this symbol alone")
    parser.add_argument("--map", type=pathlib.Path, help="file of OLD NEW lines")
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--header", nargs=2, action="append", default=[],
                        metavar=("OLD", "NEW"), help="rename include/pe1/OLD.h to NEW.h")
    parser.add_argument("--audit", action="store_true")
    parser.add_argument("--names", action="store_true",
                        help="only the type/global/header/test part of --audit")
    parser.add_argument("--verbose", "-v", action="store_true")
    args = parser.parse_args()
    if args.names:
        return audit_names(args.verbose)
    if args.audit:
        return audit(args.verbose)
    if args.header:
        if args.pairs or args.map or args.prefix:
            sys.exit("--header renames headers only; run symbol renames separately")
        return rename_headers([tuple(p) for p in args.header], args.dry_run)
    if len(args.pairs) % 2:
        sys.exit("pairs must be OLD NEW")
    pairs = list(zip(args.pairs[::2], args.pairs[1::2]))
    if args.map:
        for line in args.map.read_text().splitlines():
            line = line.split("#", 1)[0].split()
            if line:
                pairs.append((line[0], line[1]))
    if args.prefix:
        files = tracked_text_files()
        pairs += [p for p in collect_prefix(files, *args.prefix, args.dir,
                                            function_definitions(files))
                  if p[0] not in args.exclude and p[0] not in dict(pairs)]
    if not pairs:
        print("nothing to rename")
        return 0
    return rename(pairs, args.dry_run)


if __name__ == "__main__":
    sys.exit(main())
