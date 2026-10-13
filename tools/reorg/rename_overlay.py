#!/usr/bin/env python3
"""Rename a configured overlay across config, sources, symbols and docs.

``rename_overlay.py OLD NEW [OLD NEW ...]`` renames each overlay the way
docs/ROOMS.md ("What a rename touches") describes:

* ``configs/USA/overlays/OLD.yaml`` and ``sym.OLD.txt`` move to the new name,
  and inside the config every option path, the binary ``name:``, the segment
  ``name:`` and the ``dir:`` keys follow; ``target_path`` and the
  ``dd ... of=`` extraction comment keep the retail asset name;
* ``src/overlays/OLD/`` moves to ``src/overlays/NEW/``;
* path-qualified mentions in every tracked text file follow:
  ``overlays/OLD/...``, ``../OLD/...``, ``overlays/OLD`` at the end of a path,
  ``sym.OLD.txt``, ``OLD.yaml``, ``OLD.ld``, ``OLD.map``, ``OLD.elf`` and the
  splat ``*.OLD.txt`` linker inputs. A bare ``OLD.bin`` is the retail asset
  and is left alone.

``--prefix`` also renames segment and data subsegment names that start with
``OLD_`` (for overlays whose old name was misleading, such as ``sys_reset``).
Bare mentions of the old name in prose are printed, not rewritten.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
CFG = "configs/USA/overlays"
EXCLUDED = (
    re.compile(r"^docs/REORGANIZATION[^/]*\.md$"),
    re.compile(r"^docs/reorganization_mapping\.csv$"),
    re.compile(r"^tools/reorg/"),
)
TEXT_SUFFIXES = {
    ".c", ".h", ".inc", ".s", ".py", ".sh", ".md", ".json", ".yaml", ".yml",
    ".txt", ".csv", ".toml", ".ld", ".xml", ".mk", "",
}


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=True, text=True,
                          capture_output=True).stdout


def rewrite_config(text: str, old: str, new: str, prefix: bool) -> str:
    out = []
    q = re.escape(old)
    for line in text.splitlines(keepends=True):
        stripped = line.lstrip()
        if stripped.startswith("target_path:") or stripped.startswith("# extract"):
            out.append(line)
            continue
        line = re.sub(r"^name: " + q + r"\.bin", "name: " + new + ".bin", line)
        line = re.sub(r"(\s(?:basename|name|dir):\s*)" + q + r"(?=\s*$)",
                      lambda m: m.group(1) + new, line)
        line = re.sub(r"(?<=/)" + q + r"(?=[/.]|\s*$)", new, line)
        line = re.sub(r"(?<=\.)" + q + r"(?=\.txt)", new, line)
        if prefix:
            line = re.sub(r"(?<![\w+-])" + q + r"_(?=\w)", new + "_", line)
        out.append(line)
    return "".join(out)


def path_rules(old: str, new: str):
    q = re.escape(old)
    return [
        (re.compile(r"(?<=overlays/)" + q + r"(?=/|[^\w+.-]|$|\.(?:yaml|ld|map|elf|log)\b)",
                    re.M), new),
        (re.compile(r"(?<=\.\./)" + q + r"(?=/)"), new),
        (re.compile(r"(?<![\w+-])sym\." + q + r"\.txt"), f"sym.{new}.txt"),
        (re.compile(r"(?<=_auto\.)" + q + r"(?=\.txt)"), new),
        (re.compile(r"(?<=overlay-build\.)" + q + r"(?=\.log)"), new),
        (re.compile(r"(?<![\w+/-])" + q + r"\.(yaml|ld|map|elf)\b"), new + r".\1"),
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("pairs", nargs="+", metavar="OLD NEW")
    parser.add_argument("--prefix", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()
    if len(args.pairs) % 2:
        sys.exit("pairs must be OLD NEW")
    pairs = list(zip(args.pairs[::2], args.pairs[1::2]))
    tracked = [p for p in git("ls-files").splitlines() if p]
    for old, new in pairs:
        if not (ROOT / CFG / f"{old}.yaml").is_file():
            sys.exit(f"no config {old}.yaml")
        if (ROOT / CFG / f"{new}.yaml").exists() or (ROOT / "src/overlays" / new).exists():
            sys.exit(f"target {new} exists")

    edits: dict[str, str] = {}
    for path in tracked:
        if any(rx.search(path) for rx in EXCLUDED):
            continue
        if pathlib.PurePath(path).suffix not in TEXT_SUFFIXES:
            continue
        file = ROOT / path
        if not file.is_file() or file.is_symlink():
            continue
        try:
            text = file.read_text()
        except UnicodeDecodeError:
            continue
        new_text = text
        for old, new in pairs:
            if path == f"{CFG}/{old}.yaml":
                new_text = rewrite_config(new_text, old, new, args.prefix)
                continue
            for rx, repl in path_rules(old, new):
                new_text = rx.sub(repl, new_text)
        if new_text != text:
            edits[path] = new_text

    for path in sorted(edits):
        print(f"edit  {path}")
    leftovers = []
    for path in tracked:
        if any(rx.search(path) for rx in EXCLUDED):
            continue
        if pathlib.PurePath(path).suffix not in TEXT_SUFFIXES:
            continue
        text = edits.get(path)
        if text is None:
            try:
                text = (ROOT / path).read_text()
            except (UnicodeDecodeError, OSError):
                continue
        for old, _ in pairs:
            for n, line in enumerate(text.splitlines(), 1):
                if re.search(r"(?<![\w+-])" + re.escape(old) + r"(?![\w+])", line) \
                        and "target_path" not in line and "of=original" not in line:
                    leftovers.append(f"{path}:{n}: {line.strip()[:100]}")
    for line in leftovers:
        print(f"bare  {line}")
    if args.dry_run:
        return 0
    for path, text in edits.items():
        (ROOT / path).write_text(text)
    for old, new in pairs:
        git("mv", f"{CFG}/{old}.yaml", f"{CFG}/{new}.yaml")
        if (ROOT / CFG / f"sym.{old}.txt").is_file():
            git("mv", f"{CFG}/sym.{old}.txt", f"{CFG}/sym.{new}.txt")
        if any(p.startswith(f"src/overlays/{old}/") for p in tracked):
            git("mv", f"src/overlays/{old}", f"src/overlays/{new}")
    git("add", "-u")
    return 0


if __name__ == "__main__":
    sys.exit(main())
