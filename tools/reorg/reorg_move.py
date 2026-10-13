#!/usr/bin/env python3
"""Move source files with ``git mv`` and rewrite every reference to them.

Input is one or more mappings ``old -> new`` (repository-relative paths):

* ``reorg_move.py MAP.csv`` reads ``old,new`` rows (a header row and extra
  columns are ignored, ``#`` lines are comments);
* ``--dir OLD NEW`` maps every tracked file below directory OLD to the same
  relative path below NEW;
* ``--file OLD NEW`` maps one file.

A ``.c`` file drags along the unmapped headers and ``.inc`` files of its own
directory that only it includes (``X_externs.h``, template includes), so the
mapping can be written in terms of translation units.

References rewritten (old paths are never left behind):

* splat manifests (``configs/USA/main.yaml``, ``configs/USA/overlays/*.yaml``):
  every subsegment whose name resolves to a moved source, kept relative to the
  manifest's ``src_path`` (``../room_lib/X`` stays a relative form);
* quoted ``#include "..."`` lines that resolve relative to the including file,
  when either side moves;
* path-qualified mentions in any tracked text file (docs, comments, tests,
  JSON such as ``original_asm_evidence.json`` and ``match_inventory.json``):
  ``<dir>/<stem>`` tokens of the moved file, optionally prefixed by
  ``src/main/``, ``main/``, ``build/USA/src/main/`` and so on, with or without
  the extension;
* with ``--dir``: directory-level mentions ``src/main/OLD/`` as well.

Files that describe the reorganisation itself (``docs/REORGANIZATION*.md``,
``docs/reorganization_mapping.csv``, ``tools/reorg/``) are left alone because
they quote old paths on purpose.

``--dry-run`` prints the plan without touching the tree. Use
``reorg_verify.py`` with the same arguments afterwards.
"""
from __future__ import annotations

import argparse
import csv
import os
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
CONFIG_GLOBS = ("configs/USA/main.yaml", "configs/USA/overlays/*.yaml")
SOURCE_ROOTS = ("src/main", "src/overlays")
EXCLUDED = (
    re.compile(r"^docs/REORGANIZATION[^/]*\.md$"),
    re.compile(r"^docs/reorganization_mapping\.csv$"),
    re.compile(r"^tools/reorg/"),
)
TEXT_SUFFIXES = {
    ".c", ".h", ".inc", ".s", ".py", ".sh", ".md", ".json", ".yaml", ".yml",
    ".txt", ".csv", ".toml", ".ld", ".xml", "",
}
SUBSEG = re.compile(
    r"^(?P<head>\s*-\s*\[\s*0x[0-9A-Fa-f]+\s*,\s*"
    r"(?P<kind>c|\.rodata|\.data|\.sdata|\.bss|\.sbss)\s*,\s*)"
    r"(?P<name>[^\],\s]+)(?P<tail>.*)$"
)
# `src/main/gpu`, `src/main/gpu/`, `src/main/gpu/*.c`, not `src/main/gpu/X.c`.
DIR_MENTION = r"(?<![\w.-]){}(?=/(?![\w])|(?![\w/.-])|\.(?![\w]))"
INCLUDE = re.compile(r'^(?P<head>\s*#\s*include\s+")(?P<path>[^"]+)(?P<tail>".*)$')


def git(*args: str, capture: bool = True) -> str:
    result = subprocess.run(["git", *args], cwd=ROOT, check=True,
                            text=True, capture_output=capture)
    return result.stdout if capture else ""


def tracked_files() -> list[str]:
    return [line for line in git("ls-files").splitlines() if line]


def is_excluded(path: str) -> bool:
    return any(rx.search(path) for rx in EXCLUDED)


def norm(path: str) -> str:
    return os.path.normpath(path).replace(os.sep, "/")


def source_root(path: str) -> str | None:
    for root in SOURCE_ROOTS:
        if path.startswith(root + "/"):
            return root
    return None


# --------------------------------------------------------------------------
# Mapping


def read_mapping(args) -> dict[str, str]:
    moves: dict[str, str] = {}
    tracked = set(tracked_files())

    def add(old: str, new: str) -> None:
        old, new = norm(old), norm(new)
        if old == new:
            return
        if old in moves and moves[old] != new:
            sys.exit(f"conflicting targets for {old}: {moves[old]} / {new}")
        moves[old] = new

    for csv_path in args.mapping:
        with open(csv_path, newline="") as handle:
            for row in csv.reader(handle):
                if not row or row[0].startswith("#") or row[0] == "old":
                    continue
                if len(row) < 2:
                    sys.exit(f"{csv_path}: bad row {row}")
                add(row[0].strip(), row[1].strip())
    for old, new in args.file or ():
        add(old, new)
    for old_dir, new_dir in args.dir or ():
        old_dir, new_dir = norm(old_dir), norm(new_dir)
        members = [p for p in tracked if p.startswith(old_dir + "/")]
        if not members:
            sys.exit(f"--dir {old_dir}: no tracked files")
        for path in members:
            add(path, new_dir + path[len(old_dir):])

    missing = [old for old in moves if old not in tracked]
    if missing:
        sys.exit("not tracked: " + ", ".join(sorted(missing)))
    return moves


def add_private_includes(moves: dict[str, str]) -> None:
    """Move sibling headers/.inc files used only by moved .c files."""
    tracked = tracked_files()
    users: dict[str, set[str]] = {}
    for path in tracked:
        if source_root(path) is None or not path.endswith((".c", ".h", ".inc")):
            continue
        for line in (ROOT / path).read_text(errors="replace").splitlines():
            match = INCLUDE.match(line)
            if not match:
                continue
            target = norm(os.path.join(os.path.dirname(path), match["path"]))
            if (ROOT / target).is_file():
                users.setdefault(target, set()).add(path)
    changed = True
    while changed:
        changed = False
        for target, includers in sorted(users.items()):
            if target in moves or target.endswith(".c"):
                continue
            if all(user in moves for user in includers):
                new_dirs = {os.path.dirname(moves[u]) for u in includers}
                same_dir = {os.path.dirname(u) for u in includers} == {
                    os.path.dirname(target)}
                if len(new_dirs) == 1 and same_dir:
                    new = norm(os.path.join(new_dirs.pop(),
                                            os.path.basename(target)))
                    if new == target:
                        continue  # the includers only change their names
                    moves[target] = new
                    changed = True


# --------------------------------------------------------------------------
# Rewriting


def unit_forms(path: str) -> tuple[str, str] | None:
    """(root, path relative to the source root, .c stripped)."""
    root = source_root(path)
    if root is None:
        return None
    rel = path[len(root) + 1:]
    if rel.endswith(".c"):
        rel = rel[:-2]
    return root, rel


def rewrite_manifest(text: str, manifest: str, moves: dict[str, str]) -> str:
    match = re.search(r"^\s*src_path:\s*(\S+)", text, re.M)
    if not match:
        return text
    src_path = match.group(1)
    out = []
    for line in text.splitlines(keepends=True):
        sub = SUBSEG.match(line.rstrip("\n"))
        if sub:
            source = norm(os.path.join(src_path, sub["name"] + ".c"))
            if source in moves:
                new = moves[source]
                name = norm(os.path.relpath(new, src_path))
                name = name[:-2] if name.endswith(".c") else name
                line = sub["head"] + name + sub["tail"] + (
                    "\n" if line.endswith("\n") else "")
        out.append(line)
    return "".join(out)


def rewrite_includes(text: str, path: str, new_path: str,
                     moves: dict[str, str]) -> str:
    out = []
    for line in text.splitlines(keepends=True):
        match = INCLUDE.match(line.rstrip("\n"))
        if match:
            target = norm(os.path.join(os.path.dirname(path), match["path"]))
            if (ROOT / target).is_file() and (target in moves or path in moves):
                new_target = moves.get(target, target)
                rel = norm(os.path.relpath(new_target, os.path.dirname(new_path)))
                if rel != match["path"]:
                    line = match["head"] + rel + match["tail"] + (
                        "\n" if line.endswith("\n") else "")
        out.append(line)
    return "".join(out)


def token_rules(moves: dict[str, str]):
    """Regex rules for path-qualified mentions of moved files."""
    rules = []
    for old, new in sorted(moves.items(), key=lambda kv: -len(kv[0])):
        old_forms, new_forms = unit_forms(old), unit_forms(new)
        if old_forms is None or new_forms is None:
            continue
        (old_root, old_rel), (new_root, new_rel) = old_forms, new_forms
        if "/" not in old_rel:
            continue  # bare names are too ambiguous for text rewriting
        ext = ".c" if old.endswith(".c") else ""
        pattern = re.compile(
            r"(?<![\w.-])(?P<prefix>(?:[\w.+-]+/)*)" + re.escape(old_rel)
            + r"(?P<ext>" + re.escape(ext) + r"(?:\.o(?:\.d)?)?|\.s|\.o)?"
            + (r"(?![\w])" if not ext else r"(?![\w])"))
        rules.append((pattern, old_root, new_root, new_rel, ext))
    return rules


def prefix_ok(prefix: str, root: str) -> bool:
    """The prefix before `<dir>/<stem>` must name the same source root."""
    if prefix == "":
        return True
    base = root.split("/")[-1] + "/"          # main/ or overlays/
    return prefix.endswith(base) or (root == "src/overlays" and
                                     prefix.endswith("../"))


def rewrite_tokens(text: str, rules) -> str:
    for pattern, old_root, new_root, new_rel, ext in rules:
        def repl(match, old_root=old_root, new_root=new_root,
                 new_rel=new_rel):
            prefix = match["prefix"]
            if not prefix_ok(prefix, old_root):
                return match.group(0)
            if old_root != new_root and prefix.endswith(
                    old_root.split("/")[-1] + "/"):
                prefix = prefix[: -len(old_root.split("/")[-1]) - 1] + \
                    new_root.split("/")[-1] + "/"
            return prefix + new_rel + (match["ext"] or "")
        text = pattern.sub(repl, text)
    return text


def rewrite_dirs(text: str, dirs) -> str:
    for old_dir, new_dir in dirs:
        # Only bare directory mentions (`src/main/gpu/`, `src/main/gpu/*.c`);
        # `dir/Name` tokens are file mentions, handled (or reported) per file,
        # so test fixtures with invented file names stay self-consistent.
        text = re.sub(DIR_MENTION.format(re.escape(old_dir)), new_dir, text)
    return text


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("mapping", nargs="*", help="CSV files with old,new")
    parser.add_argument("--dir", nargs=2, action="append", metavar=("OLD", "NEW"))
    parser.add_argument("--file", nargs=2, action="append", metavar=("OLD", "NEW"))
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--write-map", metavar="CSV",
                        help="record the expanded mapping (for reorg_verify.py)")
    parser.add_argument("--allow-dirty", action="store_true")
    args = parser.parse_args()
    if not args.dry_run and not args.allow_dirty and git(
            "status", "--porcelain", "--untracked-files=no").strip():
        sys.exit("working tree has uncommitted changes (use --allow-dirty)")

    moves = read_mapping(args)
    if not moves:
        print("nothing to move")
        return 0
    add_private_includes(moves)
    clash = [new for new in moves.values() if (ROOT / new).exists()
             and new not in moves]
    if clash:
        sys.exit("target exists: " + ", ".join(sorted(clash)))
    if len(set(moves.values())) != len(moves):
        sys.exit("two files map to one target")

    rules = token_rules(moves)
    dirs = [(norm(o), norm(n)) for o, n in (args.dir or ())]
    edits: dict[str, str] = {}
    for path in tracked_files():
        if is_excluded(path) or pathlib.PurePath(path).suffix not in TEXT_SUFFIXES:
            continue
        file = ROOT / path
        if not file.is_file() or file.is_symlink():
            continue
        try:
            text = file.read_text()
        except UnicodeDecodeError:
            continue
        new_text = text
        if path.startswith("configs/") and path.endswith(".yaml"):
            new_text = rewrite_manifest(new_text, path, moves)
        if source_root(path) and path.endswith((".c", ".h", ".inc")):
            new_text = rewrite_includes(new_text, path, moves.get(path, path), moves)
        new_text = rewrite_tokens(new_text, rules)
        new_text = rewrite_dirs(new_text, dirs)
        if new_text != text:
            edits[path] = new_text

    for old, new in sorted(moves.items()):
        print(f"move  {old} -> {new}")
    for path in sorted(edits):
        print(f"edit  {path}")
    print(f"{len(moves)} moves, {len(edits)} edited files")
    if args.write_map:
        with open(args.write_map, "w", newline="") as handle:
            writer = csv.writer(handle, lineterminator="\n")
            writer.writerow(["old", "new"])
            for old, new in sorted(moves.items()):
                writer.writerow([old, new])
    if args.dry_run:
        return 0

    for path, text in edits.items():
        (ROOT / path).write_text(text)
    for old, new in sorted(moves.items()):
        (ROOT / new).parent.mkdir(parents=True, exist_ok=True)
        git("mv", old, new)
    git("add", "-u")
    return 0


if __name__ == "__main__":
    sys.exit(main())
