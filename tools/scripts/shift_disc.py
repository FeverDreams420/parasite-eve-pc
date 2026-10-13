#!/usr/bin/env python3
"""Build a bootable disc image of a shifted build, for an emulator test.

Needs `make build` and `make overlay-build-all`. Everything is written to the
`--out` directory (a scratch directory: the shifted executable, PE.IMG, disc
image and cue are never committed); the repository and the retail disc are
only read.

1. Main is relinked from the built objects with PAD bytes inserted at the
   start of `.main` (shift_test.pad_script, as `make shift-full-test` does)
   or, with `--insert-before`, before one input statement of the linker
   script, and every overlay is relinked against the shifted main
   (shift_full_test.overlay_link). PAD 0 relinks the retail layout.
   A pad below 0x80070E04 moves the primitive-queue block in `.text` while
   the six `lui`/`ori` literals of Draw_InitGpuHwRegs and Draw_FlushPrimQueue
   keep addressing it at the retail address (docs/SHIFTABILITY_PLAN.md 9):
   their stores then land in whatever code moved there. Inserting the pad
   after the block (`--insert-before 'build/USA/src/main/draw/frame.c.o(.text*)'`)
   moves everything else.
2. PE.IMG: the retail image with every overlay slice replaced in place by its
   relinked binary (repack_pe_img.repack; slices keep their offset and size,
   so no sector table changes), and the block pointers in the data outside
   the slices (linkers/USA/pe_img_pointers.txt) moved with the block.
3. The executable: the PS-X EXE file on the disc cannot grow (its directory
   record and the files after it fix its sectors). A shift adds PAD bytes to
   the end of the image, which is the end of the overlay window block: run
   time buffers that the file only holds as zeros, above the highest address
   any object refers to. When the shifted file is larger than the retail
   one, the excess must be zero bytes at the end; it is cut and the header's
   text size lowered by the same amount, so the BIOS loads exactly as many
   bytes as from the retail file. The cut bytes are not loaded (they lie in
   the startup's stack area, which nothing initialises from the file).
4. The disc: a copy of the retail raw image (2352-byte sectors). The ISO 9660
   root directory gives the sectors of the executable and of PE.IMG; their
   Mode 2 Form 1 sectors are rewritten with the new user data and a freshly
   computed EDC and ECC (P and Q parity). A sector whose user data does not
   change is left alone unless `--rewrite-all` is given.

Self-checks: the EDC/ECC code recomputes `--verify-sectors` untouched sectors
of the retail image and must give back their bytes; with `--pad 0
--rewrite-all` the patched disc must be byte-identical to the retail one,
which proves the patcher end to end.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import shutil
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.scripts import overlay_windows  # noqa: E402
from tools.scripts import repack_pe_img  # noqa: E402
from tools.scripts import shift_full_test as sft  # noqa: E402
from tools.scripts import shift_test as st  # noqa: E402
from tools.scripts.shift_audit import BUILD, LINKERS, OVERLAY_CONFIGS  # noqa: E402

RAW = 2352
USER = 2048
USER_OFFSET = 24  # sync 12, header 4, subheader 8
HEADER_SIZE = 0x800
TEXT_SIZE_OFFSET = 0x1C
SUBMODE_FORM2 = 0x20

DEFAULT_CUE = ROOT / "disc" / "Parasite Eve (Disc 1).cue"


# --- CD-ROM error detection and correction ----------------------------------

def _tables():
    edc = []
    for i in range(256):
        value = i
        for _ in range(8):
            value = (value >> 1) ^ (0xD8018001 if value & 1 else 0)
        edc.append(value)
    ecc_f = [0] * 256
    ecc_b = [0] * 256
    for i in range(256):
        j = ((i << 1) ^ (0x11D if i & 0x80 else 0)) & 0xFF
        ecc_f[i] = j
        ecc_b[i ^ j] = i
    return edc, ecc_f, ecc_b


EDC_LUT, ECC_F, ECC_B = _tables()


def _ecc_plan(major_count: int, minor_count: int, major_mult: int, minor_inc: int) -> list[list[int]]:
    size = major_count * minor_count
    plan = []
    for major in range(major_count):
        index = (major >> 1) * major_mult + (major & 1)
        row = []
        for _ in range(minor_count):
            row.append(index)
            index += minor_inc
            if index >= size:
                index -= size
        plan.append(row)
    return plan


# (offset of the parity in the sector, index plan over sector[12:])
ECC_P = (0x81C, _ecc_plan(86, 24, 2, 86))
ECC_Q = (0x8C8, _ecc_plan(52, 43, 86, 88))


def edc(data: bytes) -> int:
    value = 0
    lut = EDC_LUT
    for byte in data:
        value = (value >> 8) ^ lut[(value ^ byte) & 0xFF]
    return value


def _ecc_block(src: bytearray, plan: list[list[int]]) -> bytes:
    f, b = ECC_F, ECC_B
    count = len(plan)
    out = bytearray(2 * count)
    for major, row in enumerate(plan):
        ecc_a = ecc_b = 0
        for index in row:
            temp = src[index]
            ecc_a = f[ecc_a ^ temp]
            ecc_b ^= temp
        ecc_a = b[f[ecc_a] ^ ecc_b]
        out[major] = ecc_a
        out[major + count] = ecc_a ^ ecc_b
    return bytes(out)


def encode_form1(sector: bytes, user: bytes) -> bytes:
    """SECTOR (raw, Mode 2 Form 1) with USER as its data and new EDC/ECC.
    The sync, address header and subheader are kept."""
    if len(sector) != RAW or len(user) != USER:
        raise ValueError("bad sector or user data size")
    out = bytearray(sector)
    out[USER_OFFSET:USER_OFFSET + USER] = user
    out[0x818:0x81C] = struct.pack("<I", edc(bytes(out[16:0x818])))
    # Mode 2: the address header takes no part in the parity.
    src = bytearray(out[12:])
    src[0:4] = b"\0\0\0\0"
    p = _ecc_block(src, ECC_P[1])
    src[ECC_P[0] - 12:ECC_P[0] - 12 + len(p)] = p
    out[ECC_P[0]:ECC_P[0] + len(p)] = p
    q = _ecc_block(src, ECC_Q[1])
    out[ECC_Q[0]:ECC_Q[0] + len(q)] = q
    return bytes(out)


def is_form1(sector: bytes) -> bool:
    return sector[15] == 2 and not sector[18] & SUBMODE_FORM2 and sector[18] == sector[22]


# --- ISO 9660 ----------------------------------------------------------------

def read_sector(fh, lba: int) -> bytes:
    fh.seek(lba * RAW)
    return fh.read(RAW)


def root_files(fh) -> dict[str, tuple[int, int, int]]:
    """Name -> (extent LBA, size, record offset in the disc image) of the root
    directory's files."""
    pvd = read_sector(fh, 16)[USER_OFFSET:USER_OFFSET + USER]
    if pvd[0:6] != b"\x01CD001":
        raise SystemExit("no ISO 9660 primary volume descriptor at sector 16")
    root = pvd[156:156 + 34]
    lba, size = struct.unpack_from("<I", root, 2)[0], struct.unpack_from("<I", root, 10)[0]
    files = {}
    for k in range((size + USER - 1) // USER):
        block = read_sector(fh, lba + k)[USER_OFFSET:USER_OFFSET + USER]
        at = 0
        while at < USER and block[at]:
            length = block[at]
            name_len = block[at + 32]
            name = block[at + 33:at + 33 + name_len].decode("ascii", "replace").split(";")[0]
            files[name.upper()] = (struct.unpack_from("<I", block, at + 2)[0],
                                   struct.unpack_from("<I", block, at + 10)[0],
                                   (lba + k) * RAW + USER_OFFSET + at)
            at += length
    return files


def read_file(fh, lba: int, size: int) -> bytes:
    out = bytearray()
    for k in range((size + USER - 1) // USER):
        out += read_sector(fh, lba + k)[USER_OFFSET:USER_OFFSET + USER]
    return bytes(out[:size])


def write_file(fh, lba: int, old: bytes, new: bytes, force: set[int] | None = None) -> int:
    """Rewrite the sectors of a file in place (those whose data changes, and
    the FORCE sector indices); returns the sectors written."""
    if len(old) != len(new):
        raise SystemExit("a file on the disc cannot change size")
    written = 0
    for k in range((len(new) + USER - 1) // USER):
        a = old[k * USER:(k + 1) * USER]
        b = new[k * USER:(k + 1) * USER]
        if a == b and k not in (force or ()):
            continue
        raw = read_sector(fh, lba + k)
        if not is_form1(raw):
            raise SystemExit(f"sector {lba + k} is not Mode 2 Form 1")
        user = bytearray(raw[USER_OFFSET:USER_OFFSET + USER])
        user[:len(b)] = b  # a short last sector keeps its tail bytes
        fh.seek((lba + k) * RAW)
        fh.write(encode_form1(raw, bytes(user)))
        written += 1
    return written


def verify_codec(fh, first: int, count: int) -> None:
    """Recompute EDC/ECC of untouched Form 1 sectors: must equal the disc."""
    checked = 0
    for lba in range(first, first + count):
        raw = read_sector(fh, lba)
        if len(raw) < RAW or not is_form1(raw):
            continue
        if encode_form1(raw, raw[USER_OFFSET:USER_OFFSET + USER]) != raw:
            raise SystemExit(f"EDC/ECC self-check failed on sector {lba}")
        checked += 1
    if not checked:
        raise SystemExit("EDC/ECC self-check found no Form 1 sector")
    print(f"EDC/ECC self-check: {checked} retail sectors from {first} recomputed identically")


# --- build -------------------------------------------------------------------

def insert_pad(text: str, pad: int, before: str) -> str:
    """Insert PAD bytes in .main before the input statement BEFORE (e.g.
    `build/USA/src/main/draw/frame.c.o(.text*)`), instead of at its start."""
    lines = [i for i, line in enumerate(text.splitlines(keepends=True)) if line.strip() == before + ";"]
    if len(lines) != 1:
        raise SystemExit(f"expected one `{before};` statement in main.ld, found {len(lines)}")
    split = text.splitlines(keepends=True)
    indent = split[lines[0]][:len(split[lines[0]]) - len(split[lines[0]].lstrip())]
    split.insert(lines[0], f"{indent}. += 0x{pad:X}; /* shift_disc padding */\n")
    return "".join(split)


def link_main(pad: int, scratch: Path, before: str | None = None) -> tuple[Path, bytes]:
    script = LINKERS / "main.ld"
    extra = [LINKERS / "undefined_syms_manual.txt", LINKERS / "undefined_addr_aliases.main.txt"]
    for need in [script, *extra, BUILD / "main.exe"]:
        if not need.exists():
            raise SystemExit(f"missing {need}: run `make build` first")
    if before:
        text, pinned = st.pad_script(script.read_text(), 0, rebase=False)
        text = insert_pad(script.read_text(), pad, before)
    else:
        text, pinned = st.pad_script(script.read_text(), pad, rebase=False)
    if pinned:
        raise SystemExit(f"fixed placements in main.ld: {', '.join(pinned)}")
    ld = scratch / "main.shifted.ld"
    ld.write_text(text)
    elf = scratch / "main.shifted.elf"
    res = st.link(ld, elf, extra)
    if res.returncode:
        raise SystemExit("shifted main link failed:\n" + res.stderr[-3000:])
    return elf, sft.objcopy(elf, scratch / "main.shifted.exe")


def link_overlays(main_elf: Path, scratch: Path, jobs: int) -> dict[str, bytes]:
    names = sft.overlay_names()

    def one(name: str) -> tuple[str, bytes]:
        elf, err = sft.overlay_link(name, main_elf, scratch / "overlays" / name)
        if err:
            raise SystemExit(f"{name}: {err}")
        return name, sft.objcopy(elf, elf.with_suffix(".bin"))

    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        return dict(pool.map(one, names))


def fit_executable(exe: bytes, size: int) -> tuple[bytes, int]:
    """Cut a zero tail so the file fits SIZE bytes; returns (file, bytes cut)."""
    excess = len(exe) - size
    if excess <= 0:
        return exe + bytes(-excess), 0
    if any(exe[-excess:]):
        raise SystemExit(f"the shifted executable is 0x{excess:X} bytes larger than the disc file "
                         "and its tail is not zero")
    out = bytearray(exe[:-excess])
    text = struct.unpack_from("<I", out, TEXT_SIZE_OFFSET)[0]
    if text - excess != size - HEADER_SIZE:
        raise SystemExit(f"header text size 0x{text:X} does not describe the file")
    struct.pack_into("<I", out, TEXT_SIZE_OFFSET, text - excess)
    return bytes(out), excess


def slice_sectors(slots) -> set[int]:
    return {k for slot in slots for k in range(slot.offset // USER, -(-slot.end // USER))}


def sha1(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--pad", type=lambda s: int(s, 0), default=0x10,
                        help="bytes inserted before main's first unit (multiple of 4, 0 = retail layout)")
    parser.add_argument("--out", type=Path, required=True, help="scratch directory for every output")
    parser.add_argument("--cue", type=Path, default=DEFAULT_CUE, help="the retail disc's cue sheet")
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--rewrite-all", action="store_true",
                        help="re-encode every sector of the executable and of every overlay "
                             "slice of PE.IMG, changed or not")
    parser.add_argument("--insert-before", metavar="INPUT",
                        help="insert the pad before this input statement of main.ld (e.g. "
                             "'build/USA/src/main/draw/frame.c.o(.text*)') instead of at the start of .main")
    parser.add_argument("--verify-sectors", type=int, default=64,
                        help="untouched retail sectors to recompute as an EDC/ECC self-check")
    args = parser.parse_args(argv)
    if args.pad % 4 or args.pad < 0:
        parser.error("--pad must be a non-negative multiple of 4")
    out = args.out.resolve()
    if ROOT in out.parents or out == ROOT:
        parser.error("--out must be outside the repository (a scratch directory)")
    out.mkdir(parents=True, exist_ok=True)

    cue_text = args.cue.read_text()
    bin_name = cue_text.split('"')[1]
    retail_bin = args.cue.parent / bin_name

    with open(retail_bin, "rb") as fh:
        files = root_files(fh)
        exe_lba, exe_size, _ = files["SLUS_006.62"]
        img_lba, img_size, _ = files["PE.IMG"]
        retail_exe = read_file(fh, exe_lba, exe_size)
        retail_img = read_file(fh, img_lba, img_size)
        verify_codec(fh, img_lba + 1000, args.verify_sectors)

    where = f"before {args.insert_before}" if args.insert_before else "at the start of .main"
    print(f"pad 0x{args.pad:X} {where}: linking main and {len(sft.overlay_names())} overlays into {out}")
    main_elf, exe = link_main(args.pad, out, args.insert_before)
    built = link_overlays(main_elf, out, args.jobs)
    warnings: list[str] = []
    slots = repack_pe_img.read_slots(OVERLAY_CONFIGS)
    image, _ = repack_pe_img.repack(retail_img, slots, built, warnings)
    for warning in warnings:
        print(f"PE.IMG: {warning}")
    windows = overlay_windows.load()
    move = repack_pe_img.block_move(main_elf, windows.retail_base)
    tables = repack_pe_img.read_pointer_tables(repack_pe_img.POINTERS)
    image = repack_pe_img.relocate_pointers(image, tables, slots, (windows.retail_base, windows.retail_end), move)
    print(f"PE.IMG: {sum(t.count for t in tables)} block pointers outside the slices moved by 0x{move:X}")
    (out / "PE.IMG").write_bytes(image)
    fitted, cut = fit_executable(exe, exe_size)
    (out / "SLUS_006.62").write_bytes(fitted)
    print(f"executable: built 0x{len(exe):X} bytes, disc file 0x{exe_size:X}"
          + (f"; cut 0x{cut:X} zero bytes from the end and lowered the header text size" if cut else ""))
    changed = sum(retail_img[k:k + USER] != image[k:k + USER] for k in range(0, len(image), USER))
    print(f"PE.IMG: {changed} sectors differ from the retail image, sha1 {sha1(image)}")

    disc = out / bin_name
    shutil.copyfile(retail_bin, disc)
    with open(disc, "r+b") as fh:
        force_exe = set(range(-(-exe_size // USER))) if args.rewrite_all else None
        force_img = slice_sectors(repack_pe_img.read_slots(OVERLAY_CONFIGS)) if args.rewrite_all else None
        n_exe = write_file(fh, exe_lba, retail_exe, fitted, force_exe)
        n_img = write_file(fh, img_lba, retail_img, image, force_img)
    (out / args.cue.name).write_text(cue_text)
    print(f"disc: rewrote {n_exe} executable sectors and {n_img} PE.IMG sectors -> {out / args.cue.name}")

    same = disc.stat().st_size == retail_bin.stat().st_size and _same_file(disc, retail_bin)
    print(f"disc image identical to the retail one: {same}")
    if args.pad == 0 and not same:
        print("FAIL: the retail layout must give back the retail disc", file=sys.stderr)
        return 1
    print(f"main ELF for symbol probes: {main_elf}")
    return 0


def _same_file(a: Path, b: Path) -> bool:
    with open(a, "rb") as fa, open(b, "rb") as fb:
        while True:
            x, y = fa.read(1 << 22), fb.read(1 << 22)
            if x != y:
                return False
            if not x:
                return True


if __name__ == "__main__":
    sys.exit(main())
