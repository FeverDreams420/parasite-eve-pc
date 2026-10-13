#!/usr/bin/env python3
"""Write a PE.IMG whose overlay slices are the freshly built overlays.

Every overlay config names the slice of the retail PE.IMG it was extracted
from (its `extract locally with: dd ...` comment). This tool copies the
original image and overwrites each slice in place with the built binary
(`<build_path>/<basename>.bin`). Nothing moves: a slice keeps its offset and
its size, so no sector number anywhere changes.

- A built overlay larger than its slice fails the repack (all of them are
  listed, with the excess). A smaller one is padded with zero bytes up to the
  end of its slice.
- Slices may overlap; the overlapping bytes of both builds must agree. The
  one exception is a slice that lies inside a larger one, a view of its
  bytes (`fx_common_sector17` is a data copy of one sector of `fx_common`):
  the disc holds one copy, so the containing overlay's bytes are written and
  a view that differs (a relinked `fx_common` at a shifted base, while the
  view stays literal data) is reported as a warning.
- The report gives the slack of every slice (slice size minus built size).
- Data outside the slices that holds addresses inside the overlay window
  block (`linkers/USA/pe_img_pointers.txt`: the menu video table and the
  credits line table) follows the block: with `--main-elf`, every listed
  word is moved by the distance between the ELF's block base and the retail
  one (zero for the retail layout). Each must hold a retail block address.

With `--main-elf` it also reads the sector tables of the built executable
that describe PE.IMG, checks that they still cover the image end to end, and
says, for every overlay slice, which table range holds it and whether that
range records the overlay's own size (see docs/SHIFTABILITY_PLAN.md 3.7).
An in-place repack never changes a table; the check guards the invariants a
size change would break: a room section's sector count in the room table
must hold the payload size its own header gives, and a room overlay's
content must end within that size.

The retail build must reproduce the original image byte for byte
(`make pe-img-check`).
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import struct
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

import yaml

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools.scripts import overlay_windows  # noqa: E402


SECTOR = 2048  # CD-ROM mode 2 form 1 user data; PE.IMG is addressed in sectors

DD_RE = re.compile(
    r"dd\s+if=\S+\s+of=\S+\s+bs=(?P<bs>\d+)\s+skip=(?P<skip>\d+)\s+count=(?P<count>\d+)")

# Labels of the built executable that hold PE.IMG's sector layout, by role.
# The tables are data blobs of the main executable; the first name the ELF
# defines is used, so a later descriptive label replaces the splat label
# without touching this tool.
TABLE_SYMBOLS = {
    # u16 sector boundaries of the boot, font, menu and effect assets.
    "asset": ("g_PeImageSectorTable", "D_800930D8"),
    # u32 base sector, then u16 sector offsets of the stream/sound ranges.
    "stream": ("g_SceneSectorDirectory",),
    # 8-byte rows per map: u32 start sector, then scratch:8, texture:12,
    # room:12 sector counts (SceneRoomSectorRange).
    "room": ("g_RoomSectorTable", "D_80093378"),
}
ROOM_PARTS = (("scratch", 0, 8), ("texture", 8, 12), ("room", 20, 12))
POINTERS = Path(__file__).resolve().parents[2] / "linkers" / "USA" / "pe_img_pointers.txt"
BLOCK_BASE_SYMBOL = "overlay_windows_VRAM"


class RepackError(Exception):
    def __init__(self, problems: list[str]):
        super().__init__("\n".join(problems))
        self.problems = problems


@dataclass(frozen=True)
class Slot:
    """One overlay's slice of PE.IMG, in bytes."""

    name: str
    offset: int
    size: int
    built: Path

    @property
    def end(self) -> int:
        return self.offset + self.size


@dataclass
class SlotResult:
    slot: Slot
    built_size: int
    ranges: list["SectorRange"] = field(default_factory=list)
    kind: str = ""

    @property
    def slack(self) -> int:
        return self.slot.size - self.built_size


@dataclass(frozen=True)
class SectorRange:
    """One range a main table gives, in sectors relative to PE.IMG."""

    table: str
    index: int
    part: str
    start: int
    end: int

    @property
    def label(self) -> str:
        if self.table == "room":  # rows are indexed by map number - 1
            return f"room[map {self.index + 1}].{self.part} {self.start}..{self.end}"
        return f"{self.table}[{self.index}] {self.start}..{self.end}"


# --- slots -----------------------------------------------------------------

def config_slot(config: Path) -> Slot:
    text = config.read_text()
    match = DD_RE.search(text)
    if match is None:
        raise RepackError([f"{config}: no 'extract locally with: dd ...' slice comment"])
    options = yaml.safe_load(text)["options"]
    root = (config.parent / options.get("base_path", ".")).resolve()
    block = int(match.group("bs"))
    return Slot(
        name=config.stem,
        offset=block * int(match.group("skip")),
        size=block * int(match.group("count")),
        built=root / options["build_path"] / (options["basename"] + ".bin"),
    )


def read_slots(config_dir: Path) -> list[Slot]:
    slots = [config_slot(path) for path in sorted(config_dir.glob("*.yaml"))]
    if not slots:
        raise RepackError([f"{config_dir}: no overlay configs"])
    return sorted(slots, key=lambda slot: (slot.offset, slot.name))


# --- repack ----------------------------------------------------------------

def repack(image: bytes, slots: list[Slot], built: dict[str, bytes],
           warnings: list[str] | None = None) -> tuple[bytes, list[SlotResult]]:
    """The image with every slot replaced by its built bytes, zero padded.
    WARNINGS collects the views that differ from the slice they lie in."""
    warnings = [] if warnings is None else warnings
    problems: list[str] = []
    results: list[SlotResult] = []
    padded: dict[str, bytes] = {}
    for slot in slots:
        if slot.end > len(image):
            problems.append(f"{slot.name}: slice 0x{slot.offset:X}+0x{slot.size:X} "
                            f"runs past the image end 0x{len(image):X}")
            continue
        data = built.get(slot.name)
        if data is None:
            problems.append(f"{slot.name}: missing built overlay {slot.built}")
            continue
        results.append(SlotResult(slot, len(data)))
        if len(data) > slot.size:
            problems.append(f"{slot.name}: built 0x{len(data):X} bytes, slice holds 0x{slot.size:X} "
                            f"(0x{len(data) - slot.size:X} too large)")
            continue
        padded[slot.name] = data + bytes(slot.size - len(data))

    by_offset = [slot for slot in slots if slot.name in padded]
    views = set()
    for i, first in enumerate(by_offset):
        for second in by_offset[i + 1:]:
            if second.offset >= first.end:
                break
            lo, hi = second.offset, min(first.end, second.end)
            a = padded[first.name][lo - first.offset:hi - first.offset]
            b = padded[second.name][lo - second.offset:hi - second.offset]
            if a == b:
                continue
            if second.end <= first.end and first.size > second.size:
                # A slice inside a larger one is a byte view of it
                # (fx_common_sector17 of fx_common): the disc holds one copy,
                # the containing overlay's. A view built as literal data
                # cannot follow a relink of the overlay that owns the bytes.
                views.add(second.name)
                warnings.append(f"{second.name} is a view of {first.name} and differs from its "
                                f"rebuilt bytes at 0x{lo + _first_difference(a, b):X}; "
                                f"{first.name}'s bytes are written")
                continue
            problems.append(f"{first.name} and {second.name} overlap at 0x{lo:X}..0x{hi:X} "
                            f"and differ at 0x{lo + _first_difference(a, b):X}")
    if problems:
        raise RepackError(problems)

    out = bytearray(image)
    for slot in by_offset:
        if slot.name not in views:
            out[slot.offset:slot.end] = padded[slot.name]
    return bytes(out), results


def _first_difference(a: bytes, b: bytes) -> int:
    return next(k for k in range(len(a)) if a[k] != b[k])


# --- block pointers in data outside the slices ---------------------------------

@dataclass(frozen=True)
class PointerTable:
    """COUNT words, STRIDE bytes apart from OFFSET, each a block address."""

    offset: int
    count: int
    stride: int
    what: str

    def offsets(self) -> range:
        return range(self.offset, self.offset + self.count * self.stride, self.stride)


def read_pointer_tables(path: Path) -> list[PointerTable]:
    tables = []
    for number, line in enumerate(path.read_text().splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        fields = line.split(None, 3)
        if len(fields) < 3:
            raise RepackError([f"{path}:{number}: expected `offset count stride [what]`"])
        tables.append(PointerTable(int(fields[0], 0), int(fields[1], 0), int(fields[2], 0),
                                   fields[3] if len(fields) > 3 else ""))
    return tables


def relocate_pointers(image: bytes, tables: list[PointerTable], slots: list[Slot],
                      block: tuple[int, int], move: int) -> bytes:
    """IMAGE with every listed word moved by MOVE; BLOCK is the retail
    [start, end) of the window block, which every listed word must address."""
    problems = []
    out = bytearray(image)
    for table in tables:
        for at in table.offsets():
            if at % 4 or at + 4 > len(image):
                problems.append(f"pointer at 0x{at:X} is not an aligned word of the image")
                continue
            inside = [slot.name for slot in slots if slot.offset < at + 4 and at < slot.end]
            if inside:
                problems.append(f"pointer at 0x{at:X} lies in the rebuilt slice of {inside[0]}")
                continue
            value = struct.unpack_from("<I", image, at)[0]
            if not block[0] <= value < block[1]:
                problems.append(f"pointer at 0x{at:X} holds 0x{value:08X}, "
                                f"outside the window block 0x{block[0]:08X}..0x{block[1]:08X}")
                continue
            struct.pack_into("<I", out, at, value + move)
    if problems:
        raise RepackError(problems)
    return bytes(out)


def block_move(elf_path: Path, retail_base: int) -> int:
    """How far the ELF's overlay window block lies from the retail one."""
    from elftools.elf.elffile import ELFFile

    with open(elf_path, "rb") as fh:
        found = ELFFile(fh).get_section_by_name(".symtab").get_symbol_by_name(BLOCK_BASE_SYMBOL)
    if not found:
        raise RepackError([f"{elf_path}: {BLOCK_BASE_SYMBOL} is not defined"])
    return found[0]["st_value"] - retail_base


# --- main's sector tables ----------------------------------------------------

def boundary_ranges(table: str, values: list[int]) -> list[SectorRange]:
    """Ranges between consecutive entries of a monotonic boundary table."""
    return [SectorRange(table, i, "", values[i], values[i + 1]) for i in range(len(values) - 1)]


def monotonic_prefix(values: list[int]) -> list[int]:
    """The entries up to the first one that goes back (the table's padding)."""
    out: list[int] = []
    for value in values:
        if out and value < out[-1]:
            break
        out.append(value)
    return out


def room_ranges(rows: list[tuple[int, int]]) -> list[SectorRange]:
    ranges = []
    for index, (start, packed) in enumerate(rows):
        at = start
        for part, shift, bits in ROOM_PARTS:
            count = (packed >> shift) & ((1 << bits) - 1)
            ranges.append(SectorRange("room", index, part, at, at + count))
            at += count
    return ranges


def coverage_problems(ranges: list[SectorRange], image_sectors: int) -> list[str]:
    """The tables must describe the whole image once, with no gap or overlap."""
    problems = []
    at = 0
    for item in sorted((r for r in ranges if r.end > r.start), key=lambda r: r.start):
        if item.start != at:
            kind = "gap" if item.start > at else "overlap"
            problems.append(f"sector tables: {kind} at sector {min(at, item.start)} "
                            f"before {item.label}")
        at = max(at, item.end)
    if at != image_sectors:
        problems.append(f"sector tables end at sector {at}, the image has {image_sectors}")
    return problems


def classify(slot: Slot, ranges: list[SectorRange]) -> tuple[str, list[SectorRange]]:
    hits = [r for r in ranges
            if r.end > r.start and r.start * SECTOR < slot.end and slot.offset < r.end * SECTOR]
    if len(hits) != 1:
        return ("spans" if hits else "untabled"), hits
    item = hits[0]
    start, end = item.start * SECTOR, item.end * SECTOR
    if (slot.offset, slot.end) == (start, end):
        return "exact", hits
    if slot.offset == start:
        return "prefix", hits
    return "inside", hits


def header_problems(result: SlotResult, data: bytes) -> list[str]:
    """A room section opens with its payload size (word 0), which the room
    table's sector count must hold. When the overlay is the whole section,
    its content must also end within that size: a payload that grew under a
    stale header would be cut short by the loader's read."""
    if not result.ranges or result.kind not in ("exact", "prefix"):
        return []
    item = result.ranges[0]
    if item.table != "room" or item.part != "room" or len(data) < 4:
        return []
    size = struct.unpack_from("<I", data, 0)[0]
    problems = []
    if size > (item.end - item.start) * SECTOR:
        problems.append(f"{result.slot.name}: header gives 0x{size:X} bytes, "
                        f"{item.label} holds {item.end - item.start} sectors")
    used = len(data.rstrip(b"\0"))
    if result.kind == "exact" and used > size:
        problems.append(f"{result.slot.name}: content runs to 0x{used:X}, "
                        f"past the header's payload size 0x{size:X}")
    return problems


def read_main_tables(elf_path: Path, image_sectors: int) -> list[SectorRange]:
    from elftools.elf.elffile import ELFFile

    with open(elf_path, "rb") as fh:
        elf = ELFFile(fh)
        symtab = elf.get_section_by_name(".symtab")
        loaded = [(s["sh_addr"], s.data()) for s in elf.iter_sections()
                  if s["sh_type"] == "SHT_PROGBITS" and s["sh_addr"]]

        def address(role: str) -> int:
            for name in TABLE_SYMBOLS[role]:
                found = symtab.get_symbol_by_name(name)
                if found:
                    return found[0]["st_value"]
            raise RepackError([f"{elf_path}: none of {', '.join(TABLE_SYMBOLS[role])} is defined"])

        def read(addr: int, size: int) -> bytes:
            for base, data in loaded:
                if base <= addr and addr + size <= base + len(data):
                    return data[addr - base:addr + size - base]
            raise RepackError([f"{elf_path}: 0x{addr:08X}+0x{size:X} is not in a loaded section"])

        asset, stream, room = address("asset"), address("stream"), address("room")
        if not asset < stream < room:
            raise RepackError([f"{elf_path}: sector tables out of order "
                               f"(asset 0x{asset:08X}, stream 0x{stream:08X}, room 0x{room:08X})"])
        values = list(struct.unpack(f"<{(stream - asset) // 2}H", read(asset, stream - asset)))
        ranges = boundary_ranges("asset", monotonic_prefix(values))
        base = struct.unpack("<I", read(stream, 4))[0]
        count = (room - stream - 4) // 2
        offsets = monotonic_prefix(list(struct.unpack(f"<{count}H", read(stream + 4, count * 2))))
        ranges += boundary_ranges("stream", [base + value for value in offsets])
        rows = []
        while True:
            start, packed = struct.unpack("<2I", read(room + 8 * len(rows), 8))
            if start >= image_sectors:
                break
            rows.append((start, packed))
        ranges += room_ranges(rows)
    return ranges


def table_report(results: list[SlotResult], built: dict[str, bytes],
                 ranges: list[SectorRange], image_sectors: int) -> tuple[list[str], list[str]]:
    problems = coverage_problems(ranges, image_sectors)
    lines = []
    kinds: dict[str, list[str]] = {}
    for result in results:
        result.kind, result.ranges = classify(result.slot, ranges)
        kinds.setdefault(result.kind, []).append(result.slot.name)
        problems += header_problems(result, built[result.slot.name])
    lines.append(f"sector tables: {len([r for r in ranges if r.end > r.start])} ranges cover "
                 f"sectors 0..{image_sectors}")
    meaning = {
        "exact": "the range is the slice; its sector count is the overlay's size",
        "prefix": "the slice opens a larger range; no table records the overlay's size",
        "inside": "the slice lies inside a range; no table records the overlay's size",
        "spans": "the slice covers parts of several ranges",
        "untabled": "no table range holds the slice",
    }
    for kind in ("exact", "prefix", "inside", "spans", "untabled"):
        names = kinds.get(kind, [])
        if names:
            lines.append(f"  {kind:8} {len(names):3}  {meaning[kind]}")
    return lines, problems


# --- command line ------------------------------------------------------------

def sha1(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def write_atomic(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=path.parent, prefix=path.name + ".")
    try:
        with os.fdopen(fd, "wb") as fh:
            fh.write(data)
        os.replace(tmp, path)
    except BaseException:
        os.unlink(tmp)
        raise


def run(args: argparse.Namespace) -> int:
    image = args.pe_img.read_bytes()
    if args.input_sha1 and sha1(image) != args.input_sha1:
        raise RepackError([f"{args.pe_img}: SHA-1 {sha1(image)}, expected the retail {args.input_sha1}"])
    slots = read_slots(args.configs)
    built = {slot.name: slot.built.read_bytes() for slot in slots if slot.built.exists()}
    warnings: list[str] = []
    out, results = repack(image, slots, built, warnings)
    for warning in warnings:
        print(f"warning: {warning}")

    windows = overlay_windows.load()
    block = (windows.retail_base, windows.retail_end)
    move = block_move(args.main_elf, windows.retail_base) if args.main_elf else 0
    out = relocate_pointers(out, read_pointer_tables(args.pointers), slots, block, move)

    if args.main_elf:
        lines, problems = table_report(results, built, read_main_tables(args.main_elf, len(image) // SECTOR),
                                       len(image) // SECTOR)
        if problems:
            raise RepackError(problems)
    else:
        lines = []

    if args.verbose:
        print(f"{'overlay':32} {'offset':>10} {'slice':>8} {'built':>8} {'slack':>7}  table range")
        for result in sorted(results, key=lambda r: r.slot.offset):
            where = ", ".join(r.label for r in result.ranges)
            print(f"{result.slot.name:32} 0x{result.slot.offset:08X} 0x{result.slot.size:06X} "
                  f"0x{result.built_size:06X} 0x{result.slack:05X}  {result.kind} {where}".rstrip())
    slack = [r for r in results if r.slack]
    print(f"{len(results)} overlays repacked in place; "
          + (f"{len(slack)} with slack, 0x{sum(r.slack for r in slack):X} bytes in all"
             if slack else "every slice is full (no slack)"))
    for result in slack if not args.verbose else []:
        print(f"  {result.slot.name}: 0x{result.slack:X} bytes free of 0x{result.slot.size:X}")
    for line in lines:
        print(line)

    if args.out:
        write_atomic(args.out, out)
        print(f"wrote {args.out} (sha1 {sha1(out)})")
    if args.expect_sha1 and sha1(out) != args.expect_sha1:
        raise RepackError([f"repacked image SHA-1 {sha1(out)} differs from {args.expect_sha1}"])
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--pe-img", type=Path, default=Path("disc/extracted/PE.IMG"),
                        help="the original image")
    parser.add_argument("--configs", type=Path, default=Path("configs/USA/overlays"),
                        help="overlay config directory")
    parser.add_argument("--out", type=Path, help="where to write the repacked image")
    parser.add_argument("--main-elf", type=Path,
                        help="built executable; checks its PE.IMG sector tables and places the block pointers")
    parser.add_argument("--pointers", type=Path, default=POINTERS,
                        help="block pointers in data outside the slices")
    parser.add_argument("--input-sha1", help="required SHA-1 of the original image")
    parser.add_argument("--expect-sha1", help="required SHA-1 of the repacked image")
    parser.add_argument("-v", "--verbose", action="store_true", help="one line per overlay")
    args = parser.parse_args(argv)
    try:
        return run(args)
    except RepackError as error:
        for problem in error.problems:
            print(f"error: {problem}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
