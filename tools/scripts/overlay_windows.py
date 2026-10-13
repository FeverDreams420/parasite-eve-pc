#!/usr/bin/env python3
"""The overlay window block: one table, read by the build tools.

`linkers/USA/overlay_windows.txt` names every overlay window as an offset
from `overlay_windows_VRAM`, the start of the floating window block (the
`overlay_windows` segment of configs/USA/main.yaml). The overlay configs and
catalogues describe the retail binaries, so they hold retail addresses; this
module turns a retail address inside the block into `WINDOW + offset`, the
form every overlay links with, and back.

    overlay_windows.py rebase-ld LINKER_SCRIPT   rewrite an overlay's splat
        linker script so that each section VMA in the block is `(WINDOW +
        offset)` (run by `make overlay-split`; idempotent)
    overlay_windows.py check                     every overlay segment lies in
        the block and every window starts an overlay or a main label

The window symbols themselves are defined by main's link (main.ld INCLUDEs
the table) and handed to the overlays from the built main.elf
(overlay_link_symbols.py), so the block moves when main grows.
"""
from __future__ import annotations

import argparse
import re
import sys
from bisect import bisect_right
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TABLE = ROOT / "linkers" / "USA" / "overlay_windows.txt"
MAIN_CONFIG = ROOT / "configs" / "USA" / "main.yaml"
OVERLAY_CONFIGS = ROOT / "configs" / "USA" / "overlays"
BASE_SYMBOL = "overlay_windows_VRAM"

ENTRY_RE = re.compile(
    r"^\s*(OVERLAY_WINDOW_\w+)\s*=\s*" + BASE_SYMBOL + r"\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\s*;", re.M)
SEGMENT_RE = re.compile(
    r"^  - name: (\S+)\n((?:    .*\n)*)", re.M)
EXPRESSION_RE = re.compile(r"^\(?\s*(OVERLAY_WINDOW_\w+)\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\s*\)?$")
SECTION_VMA_RE = re.compile(r"^(    \.[\w.]+ )(0x[0-9A-Fa-f]+)( : )", re.M)


class WindowError(Exception):
    pass


@dataclass(frozen=True)
class WindowTable:
    """Windows as (symbol, offset from the block base), sorted by offset."""
    windows: tuple[tuple[str, int], ...]
    retail_base: int
    retail_end: int

    def __post_init__(self):
        offsets = [off for _, off in self.windows]
        if not self.windows or offsets[0] != 0:
            raise WindowError("the first window must start the block (offset 0)")
        if offsets != sorted(set(offsets)):
            raise WindowError("window offsets must be distinct and in increasing order")

    @property
    def symbols(self) -> list[str]:
        return [name for name, _ in self.windows]

    def offset(self, name: str) -> int:
        return dict(self.windows)[name]

    def contains(self, retail_addr: int) -> bool:
        return self.retail_base <= retail_addr < self.retail_end

    def locate(self, retail_addr: int) -> tuple[str, int]:
        """(window symbol, offset in that window) of a retail block address."""
        if not self.contains(retail_addr):
            raise WindowError(f"0x{retail_addr:08X} is outside the overlay window block")
        rel = retail_addr - self.retail_base
        i = bisect_right([off for _, off in self.windows], rel) - 1
        name, off = self.windows[i]
        return name, rel - off

    def expression(self, retail_addr: int) -> str:
        name, off = self.locate(retail_addr)
        return f"{name} + 0x{off:X}"

    def evaluate(self, expr: str, base: int | None = None) -> int | None:
        """The address of `WINDOW + offset` with the block at BASE (retail by
        default); None when EXPR is not of that form."""
        m = EXPRESSION_RE.match(expr.strip())
        if not m or m.group(1) not in dict(self.windows):
            return None
        start = self.retail_base if base is None else base
        return start + self.offset(m.group(1)) + int(m.group(2), 0)


def segment_fields(config_text: str, segment: str) -> dict[str, str]:
    for m in SEGMENT_RE.finditer(config_text):
        if m.group(1) == segment:
            return dict(re.findall(r"^    (\w+): *(\S+)", m.group(2), re.M))
    raise WindowError(f"no `{segment}` segment")


def block_bounds(main_config: Path = MAIN_CONFIG) -> tuple[int, int]:
    """Retail [start, end) of the window block from main's splat config."""
    text = main_config.read_text()
    fields = segment_fields(text, "overlay_windows")
    start, vram = int(fields["start"], 0), int(fields["vram"], 0)
    ends = re.findall(r"^  - \[(0x[0-9A-Fa-f]+)\]\s*$", text, re.M)
    if not ends:
        raise WindowError(f"{main_config}: no end marker after the overlay_windows segment")
    return vram, vram + int(ends[-1], 16) - start


def load(table: Path = TABLE, main_config: Path = MAIN_CONFIG) -> WindowTable:
    text = re.sub(r"/\*.*?\*/", "", table.read_text(), flags=re.S)
    windows = tuple((name, int(off, 0)) for name, off in ENTRY_RE.findall(text))
    base, end = block_bounds(main_config)
    return WindowTable(windows, base, end)


def rebase_ld(text: str, table: WindowTable) -> str:
    """Section VMAs inside the block become `(WINDOW + offset)`."""
    def move(m):
        addr = int(m.group(2), 16)
        if not table.contains(addr):
            raise WindowError(f"section {m.group(1).strip()} at 0x{addr:08X} lies outside the "
                              "overlay window block (linkers/USA/overlay_windows.txt)")
        return f"{m.group(1)}({table.expression(addr)}){m.group(3)}"
    return SECTION_VMA_RE.sub(move, text)


def overlay_segments(config: Path) -> list[tuple[str, int]]:
    text = config.read_text()
    return [(m.group(1), int(dict(re.findall(r"^    (\w+): *(\S+)", m.group(2), re.M))["vram"], 0))
            for m in SEGMENT_RE.finditer(text)
            if re.search(r"^    vram: ", m.group(2), re.M)]


def main_labels(sym_file: Path) -> set[int]:
    text = sym_file.read_text() if sym_file.exists() else ""
    return {int(v, 16) for v in re.findall(r"^\s*\w+\s*=\s*(0x[0-9A-Fa-f]+)\s*;", text, re.M)}


def check(table: WindowTable, configs: Path = OVERLAY_CONFIGS,
          sym_file: Path = ROOT / "configs" / "USA" / "sym.main.txt") -> list[str]:
    """Problems: overlay segments outside the block, windows that are neither
    an overlay's load address nor a buffer main names."""
    problems = []
    starts = main_labels(sym_file)
    for cfg in sorted(configs.glob("*.yaml")):
        segments = overlay_segments(cfg)
        for name, vram in segments:
            if not table.contains(vram):
                problems.append(f"{cfg.name}: segment {name} at 0x{vram:08X} is outside the block")
        if segments:
            starts.add(segments[0][1])
    for name, off in table.windows:
        addr = table.retail_base + off
        if addr not in starts:
            problems.append(f"{name} (0x{addr:08X}) is neither an overlay's load address "
                            "nor a buffer named in configs/USA/sym.main.txt")
    return problems


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    rebase = sub.add_parser("rebase-ld")
    rebase.add_argument("script", type=Path)
    sub.add_parser("check")
    args = parser.parse_args(argv)
    try:
        table = load()
        if args.command == "rebase-ld":
            text = args.script.read_text()
            new = rebase_ld(text, table)
            if new != text:
                args.script.write_text(new)
            return 0
        problems = check(table)
    except WindowError as exc:
        print(f"overlay_windows: {exc}", file=sys.stderr)
        return 1
    for problem in problems:
        print(f"overlay_windows: {problem}", file=sys.stderr)
    if not problems:
        print(f"OK: {len(table.windows)} windows, block 0x{table.retail_base:08X}..0x{table.retail_end:08X}")
    return int(bool(problems))


if __name__ == "__main__":
    sys.exit(main())
