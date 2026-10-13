import contextlib
import io
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.scripts import repack_pe_img as rp
from tools.scripts.shift_audit import tool

SECTOR = rp.SECTOR

CONFIG = """\
name: {name}.bin
options:
  basename: {name}
  base_path: ../../..
  build_path: build/USA/overlays/{name}
segments:
  - [0x0]
# extract locally with: dd if=disc/extracted/PE.IMG of=original/USA/overlays/{name}.bin bs={bs} skip={skip} count={count}
"""


def room_payload(size: int, slot: int, fill: int) -> bytes:
    """A room section: word 0 is the payload size, then content, then zeros."""
    body = struct.pack("<II", size, 8) + bytes([fill]) * (size - 8)
    return body + bytes(slot - size)


class SyntheticImage:
    """A 12-sector image with three overlay slices:

    room_a   sectors 2..4 (a whole room section, 2 sectors)
    room_b   sectors 4..7, and room_b_copy, its sector 1 at byte granularity
    tail     bytes 0x5000+0x10..0x5000+0x110 (byte slice inside sector 10)
    """

    def __init__(self, root: Path):
        self.root = root
        self.configs = root / "configs" / "USA" / "overlays"
        self.configs.mkdir(parents=True)
        image = bytearray(b"\xEE" * (12 * SECTOR))
        self.parts = {
            "room_a": (SECTOR, 2, 2, room_payload(0x900, 2 * SECTOR, 0xA1)),
            "room_b": (SECTOR, 4, 3, room_payload(0x1234, 3 * SECTOR, 0xB2)),
            "room_b_copy": (1, 5 * SECTOR, SECTOR, None),
            "tail": (1, 10 * SECTOR + 0x10, 0x100, bytes(range(256))),
        }
        for name, (bs, skip, count, data) in self.parts.items():
            if data is None:  # a byte-level copy of room_b's second sector
                data = self.parts["room_b"][3][SECTOR:2 * SECTOR]
            image[bs * skip:bs * (skip + count)] = data
            (self.configs / f"{name}.yaml").write_text(
                CONFIG.format(name=name, bs=bs, skip=skip, count=count))
            self.build(name, data)
        self.image = bytes(image)

    def build(self, name: str, data: bytes) -> None:
        path = self.root / "build" / "USA" / "overlays" / name / f"{name}.bin"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)

    def built(self, slots):
        return {s.name: s.built.read_bytes() for s in slots if s.built.exists()}

    def repack(self):
        slots = rp.read_slots(self.configs)
        return rp.repack(self.image, slots, self.built(slots))

    def ranges(self):
        """Sector tables as main would hold them for this image."""
        ranges = rp.boundary_ranges("asset", [0, 1, 2])
        packed = lambda scratch, texture, room: scratch | texture << 8 | room << 20
        ranges += rp.room_ranges([(2, packed(0, 0, 2)), (4, packed(0, 0, 3)),
                                  (7, packed(1, 2, 0)), (10, packed(0, 0, 2))])
        return ranges


class SlotTests(unittest.TestCase):
    def test_reads_byte_and_sector_slices_and_the_build_path(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            slots = {s.name: s for s in rp.read_slots(img.configs)}
            root = Path(tmp).resolve()
        self.assertEqual((slots["room_b"].offset, slots["room_b"].size), (4 * SECTOR, 3 * SECTOR))
        self.assertEqual((slots["tail"].offset, slots["tail"].size), (10 * SECTOR + 0x10, 0x100))
        self.assertEqual(slots["tail"].built, root / "build/USA/overlays/tail/tail.bin")

    def test_a_config_without_a_slice_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "x.yaml"
            path.write_text("options: {basename: x, build_path: b}\n")
            with self.assertRaises(rp.RepackError):
                rp.config_slot(path)


class RepackTests(unittest.TestCase):
    def test_unchanged_builds_give_back_the_original_image(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            out, results = img.repack()
        self.assertEqual(out, img.image)
        self.assertEqual(len(results), 4)
        self.assertTrue(all(r.slack == 0 for r in results))

    def test_a_changed_overlay_is_written_in_place(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            img.build("tail", b"\x55" * 0x100)
            out, _ = img.repack()
        at = 10 * SECTOR + 0x10
        self.assertEqual(out[at:at + 0x100], b"\x55" * 0x100)
        self.assertEqual(out[:at], img.image[:at])
        self.assertEqual(out[at + 0x100:], img.image[at + 0x100:])

    def test_a_smaller_overlay_is_zero_padded_and_reports_slack(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            img.build("room_a", b"\x11" * 0x100)
            out, results = img.repack()
        self.assertEqual(out[2 * SECTOR:4 * SECTOR], b"\x11" * 0x100 + bytes(2 * SECTOR - 0x100))
        self.assertEqual(len(out), len(img.image))
        slack = {r.slot.name: r.slack for r in results}
        self.assertEqual(slack["room_a"], 2 * SECTOR - 0x100)

    def test_every_oversized_overlay_fails_with_its_excess(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            img.build("room_a", bytes(2 * SECTOR + 4))
            img.build("tail", bytes(0x101))
            with self.assertRaises(rp.RepackError) as caught:
                img.repack()
        problems = "\n".join(caught.exception.problems)
        self.assertIn("room_a: built 0x1004 bytes, slice holds 0x1000 (0x4 too large)", problems)
        self.assertIn("tail: built 0x101 bytes, slice holds 0x100 (0x1 too large)", problems)

    def test_a_view_inside_a_slice_follows_the_slice_it_copies(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            data = bytearray(img.parts["room_b"][3])
            data[SECTOR + 7] ^= 0xFF
            img.build("room_b", bytes(data))
            slots = rp.read_slots(img.configs)
            warnings = []
            out, _ = rp.repack(img.image, slots, img.built(slots), warnings)
        self.assertEqual(out[4 * SECTOR:7 * SECTOR], bytes(data))
        self.assertEqual(warnings, ["room_b_copy is a view of room_b and differs from its rebuilt "
                                    "bytes at 0x2807; room_b's bytes are written"])

    def test_partly_overlapping_slices_must_agree(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            (img.configs / "straddle.yaml").write_text(
                CONFIG.format(name="straddle", bs=SECTOR, skip=3, count=2))
            img.build("straddle", img.image[3 * SECTOR:5 * SECTOR])
            self.assertEqual(img.repack()[0], img.image)
            data = bytearray(img.parts["room_a"][3])
            data[SECTOR + 7] ^= 0xFF
            img.build("room_a", bytes(data))
            with self.assertRaises(rp.RepackError) as caught:
                img.repack()
        self.assertEqual(caught.exception.problems,
                         ["room_a and straddle overlap at 0x1800..0x2000 and differ at 0x1807"])

    def test_a_missing_build_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            (img.root / "build/USA/overlays/tail/tail.bin").unlink()
            with self.assertRaises(rp.RepackError) as caught:
                img.repack()
        self.assertIn("tail: missing built overlay", caught.exception.problems[0])

    def test_command_line_writes_and_checks_the_sha1(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            source = Path(tmp) / "PE.IMG"
            source.write_bytes(img.image)
            out = Path(tmp) / "out" / "PE.IMG"
            pointers = Path(tmp) / "pointers.txt"
            pointers.write_text("# offset count stride what\n")
            sha = rp.sha1(img.image)
            argv = ["--pe-img", str(source), "--configs", str(img.configs), "--out", str(out),
                    "--pointers", str(pointers), "--input-sha1", sha, "--expect-sha1", sha]
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(rp.main(argv), 0)
            self.assertEqual(out.read_bytes(), img.image)
            img.build("tail", b"\x55" * 0x100)
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(rp.main(argv), 1)


class PointerTests(unittest.TestCase):
    BLOCK = (0x8010BCF8, 0x801FE000)

    def image_with(self, words: dict[int, int]) -> bytes:
        data = bytearray(0x200)
        for at, value in words.items():
            struct.pack_into("<I", data, at, value)
        return bytes(data)

    def test_listed_words_follow_the_block(self):
        image = self.image_with({0x40: 0x8018F2D4, 0x54: 0x8018F2C4, 0x48: 0x12345678})
        table = rp.PointerTable(0x40, 2, 0x14, "video names")
        out = rp.relocate_pointers(image, [table], [], self.BLOCK, 0x10)
        self.assertEqual(struct.unpack_from("<3I", out, 0x40)[0], 0x8018F2E4)
        self.assertEqual(struct.unpack_from("<I", out, 0x54)[0], 0x8018F2D4)
        self.assertEqual(struct.unpack_from("<I", out, 0x48)[0], 0x12345678)
        self.assertEqual(rp.relocate_pointers(image, [table], [], self.BLOCK, 0), image)

    def test_a_word_outside_the_block_or_inside_a_slice_fails(self):
        image = self.image_with({0x40: 0x80010000, 0x80: 0x8018F2D4})
        slot = rp.Slot("room", 0x80, 0x10, Path("room.bin"))
        with self.assertRaises(rp.RepackError) as caught:
            rp.relocate_pointers(image, [rp.PointerTable(0x40, 1, 4, ""), rp.PointerTable(0x80, 1, 4, "")],
                                 [slot], self.BLOCK, 0x10)
        self.assertEqual(caught.exception.problems, [
            "pointer at 0x40 holds 0x80010000, outside the window block 0x8010BCF8..0x801FE000",
            "pointer at 0x80 lies in the rebuilt slice of room"])

    def test_the_manifest_lists_offset_count_stride(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "p.txt"
            path.write_text("# comment\n0x22AE10  47  0x14  video table\n")
            self.assertEqual(rp.read_pointer_tables(path), [rp.PointerTable(0x22AE10, 47, 0x14, "video table")])

    def test_the_retail_manifest_addresses_the_block(self):
        tables = rp.read_pointer_tables(rp.POINTERS)
        self.assertEqual(sum(t.count for t in tables), 47 + 715)


class TableTests(unittest.TestCase):
    def test_room_rows_unpack_into_three_consecutive_ranges(self):
        packed = 33 | 74 << 8 | 91 << 20
        self.assertEqual([(r.part, r.start, r.end) for r in rp.room_ranges([(100, packed)])],
                         [("scratch", 100, 133), ("texture", 133, 207), ("room", 207, 298)])

    def test_boundary_tables_stop_at_their_padding(self):
        self.assertEqual(rp.monotonic_prefix([0, 71, 72, 72, 87, 0, 0]), [0, 71, 72, 72, 87])

    def test_coverage_finds_gaps_overlaps_and_a_short_end(self):
        ranges = rp.boundary_ranges("asset", [0, 2, 5]) + rp.boundary_ranges("stream", [6, 8])
        self.assertEqual(rp.coverage_problems(ranges, 8), [
            "sector tables: gap at sector 5 before stream[0] 6..8"])
        ranges = rp.boundary_ranges("asset", [0, 4]) + rp.boundary_ranges("stream", [3, 8])
        self.assertIn("overlap", rp.coverage_problems(ranges, 8)[0])
        self.assertEqual(rp.coverage_problems(rp.boundary_ranges("asset", [0, 4]), 8),
                         ["sector tables end at sector 4, the image has 8"])

    def test_slices_are_classified_against_the_ranges(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            _, results = img.repack()
            built = img.built([r.slot for r in results])
            lines, problems = rp.table_report(results, built, img.ranges(), 12)
        kinds = {r.slot.name: (r.kind, [x.label for x in r.ranges]) for r in results}
        self.assertEqual(problems, [])
        self.assertEqual(kinds["room_a"], ("exact", ["room[map 1].room 2..4"]))
        self.assertEqual(kinds["room_b_copy"], ("inside", ["room[map 2].room 4..7"]))
        self.assertEqual(kinds["tail"], ("inside", ["room[map 4].room 10..12"]))
        self.assertIn("sector tables: 7 ranges cover sectors 0..12", lines)

    def test_a_room_header_must_fit_the_table_and_cover_the_content(self):
        with tempfile.TemporaryDirectory() as tmp:
            img = SyntheticImage(Path(tmp))
            # A payload that grew under a stale header, and a header that
            # claims more than the room table's two sectors.
            grown = bytearray(img.parts["room_a"][3])
            grown[0x900:0x980] = b"\x77" * 0x80
            img.build("room_a", bytes(grown))
            claim = bytearray(img.parts["room_b"][3])
            struct.pack_into("<I", claim, 0, 3 * SECTOR + 1)
            img.build("room_b", bytes(claim))
            _, results = img.repack()
            built = img.built([r.slot for r in results])
            _, problems = rp.table_report(results, built, img.ranges(), 12)
        self.assertEqual(problems, [
            "room_a: content runs to 0x980, past the header's payload size 0x900",
            "room_b: header gives 0x1801 bytes, room[map 2].room 4..7 holds 3 sectors",
        ])


AS = tool("mipsel-none-elf-as")
LD = tool("mipsel-none-elf-ld")


@unittest.skipUnless(shutil.which(AS) or Path(AS).exists(), "needs mipsel binutils")
class ElfTableTests(unittest.TestCase):
    def test_reads_the_three_tables_by_symbol(self):
        source = """
        .data
        .globl D_800930D8, g_SceneSectorDirectory, D_80093378
        D_800930D8: .half 0, 1, 2, 0
        g_SceneSectorDirectory: .word 2
                    .half 0, 0, 0
        .align 2
        D_80093378: .word 2, 2 << 20, 4, 3 << 20, 7, 1 | 2 << 8, 10, 2 << 20, 12, 0
        """
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            (tmp / "t.s").write_text(source)
            subprocess.run([AS, "-EL", "-o", str(tmp / "t.o"), str(tmp / "t.s")], check=True)
            subprocess.run([LD, "-EL", "-Ttext=0x80010000", "-Tdata=0x80020000", "-e", "0",
                            "-o", str(tmp / "t.elf"), str(tmp / "t.o")], check=True)
            ranges = rp.read_main_tables(tmp / "t.elf", 12)
        self.assertEqual(rp.coverage_problems(ranges, 12), [])
        self.assertEqual([r.label for r in ranges if r.end > r.start], [
            "asset[0] 0..1", "asset[1] 1..2",
            "room[map 1].room 2..4", "room[map 2].room 4..7",
            "room[map 3].scratch 7..8", "room[map 3].texture 8..10",
            "room[map 4].room 10..12",
        ])


if __name__ == "__main__":
    unittest.main()
