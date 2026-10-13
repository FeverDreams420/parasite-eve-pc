#!/usr/bin/env python3
"""Give an overlay function and its address-named file a role name.

``rename_overlay_function.py OVERLAY OLD NEW [OLD NEW ...]`` renames the C
symbol OLD (``func_XXXXXXXX``) of overlay OVERLAY to NEW:

* every whole-word use in the overlay's own sources, in ``include/pe1`` and
  in ``src/overlays/room_lib`` templates is rewritten;
* ``NEW = 0xXXXXXXXX; // type:func`` is added to the overlay's symbol file,
  so splat labels the address (and data words pointing at it) the same way;
* when the overlay has a unit file named OLD.c, it moves to NEW.c together
  with its ``OLD_externs*.h`` headers, and the manifest entry follows;
* path-qualified mentions in docs and tests follow the file.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
CFG = ROOT / "configs/USA/overlays"


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=True, text=True,
                          capture_output=True).stdout


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("overlay")
    parser.add_argument("pairs", nargs="+", metavar="OLD NEW")
    args = parser.parse_args()
    if len(args.pairs) % 2:
        sys.exit("pairs must be OLD NEW")
    cfg = CFG / f"{args.overlay}.yaml"
    sym = CFG / f"sym.{args.overlay}.txt"
    cfg_text = cfg.read_text()
    src_path = re.search(r"^\s*src_path:\s*(\S+)", cfg_text, re.M).group(1)
    ov_dir = src_path.split("/")[-1]
    for old, new in zip(args.pairs[::2], args.pairs[1::2]):
        tracked = git("ls-files").splitlines()
        sources = [p for p in tracked if p.endswith((".c", ".h", ".inc")) and (
            p.startswith(src_path + "/") or p.startswith("include/pe1/") or
            p.startswith("src/overlays/room_lib/")) and (ROOT / p).is_file()]
        m = re.fullmatch(r"func_([0-9A-F]{8})", old)
        if not m:
            sys.exit(f"{old} is not an address name")
        if any(re.search(r"\b" + new + r"\b(?!\.)", (ROOT / p).read_text()) for p in sources):
            sys.exit(f"{new} already used")
        rx = re.compile(r"\b" + old + r"\b")
        for path in sources:
            file = ROOT / path
            text = file.read_text()
            if path.startswith(src_path + "/") or rx.search(text):
                new_text = rx.sub(new, text)
                new_text = new_text.replace(f"{old}_externs", f"{new}_externs")
                if new_text != text:
                    file.write_text(new_text)
        text = sym.read_text() if sym.exists() else ""
        if not text.endswith("\n") and text:
            text += "\n"
        sym.write_text(text + f"{new} = 0x{m.group(1)}; // type:func\n")
        unit = ROOT / src_path / f"{old}.c"
        if unit.exists():
            git("mv", f"{src_path}/{old}.c", f"{src_path}/{new}.c")
            for header in sorted((ROOT / src_path).glob(f"{old}_externs*.h")):
                git("mv", str(header.relative_to(ROOT)),
                    str(header.relative_to(ROOT)).replace(f"{old}_externs", f"{new}_externs"))
            cfg_text = re.sub(r"(,\s*c,\s*)" + old + r"(\s*\])", r"\g<1>" + new + r"\2", cfg_text)
            path_rx = re.compile(r"(?<=" + re.escape(ov_dir) + r"/)" + old + r"(?=\.c\b|\b)")
            for path in tracked:
                if not path.startswith(("docs/", "tools/tests/")) or \
                        path.startswith("docs/REORGANIZATION") or \
                        not path.endswith((".md", ".py", ".json")):
                    continue
                file = ROOT / path
                text = file.read_text()
                new_text = path_rx.sub(new, text)
                if new_text != text:
                    file.write_text(new_text)
        cfg.write_text(cfg_text)
        print(f"{args.overlay}: {old} -> {new}")
    git("add", "-A", src_path, "include", "src/overlays/room_lib", "configs", "docs", "tools/tests")
    return 0


if __name__ == "__main__":
    sys.exit(main())
