import tempfile
import unittest
from pathlib import Path

from tools.scripts import shift_full_test as sf
from tools.scripts import shift_test as st
from tools.scripts.shift_audit import BUILD, LINKERS


def words(*values: int) -> bytes:
    return b"".join(v.to_bytes(4, "little") for v in values)


PAD = 0x10


class CompareWordsTests(unittest.TestCase):
    def test_relocated_fields_moved_by_the_pad_are_explained(self):
        base = words(0x80190000, 0x0C064000, 0x3C028019, 0x24420010, 0x12345678)
        shifted = words(0x80190010, 0x0C064004, 0x3C028019, 0x24420020, 0x12345678)
        relocs = {0: sf.Reloc(st.R_MIPS_32, "D_80190000", 0x80190000, False),
                  4: sf.Reloc(st.R_MIPS_26, "func_80190000", 0x80190000, False),
                  8: sf.Reloc(st.R_MIPS_HI16, "D_80190010", 0x80190010, False),
                  12: sf.Reloc(st.R_MIPS_LO16, "D_80190010", 0x80190010, False)}
        result = sf.compare_words(base, shifted, relocs, PAD)
        self.assertEqual(result.counts["shifted_address_field"], 3)
        self.assertEqual(result.counts["unchanged_half"], 1)
        self.assertEqual(result.counts["unchanged"], 1)
        self.assertFalse(any(result.counts[k] for k in sf.FAILING))

    def test_an_unrelocated_difference_fails(self):
        result = sf.compare_words(words(1, 2), words(1, 3), {}, PAD)
        self.assertEqual(result.counts["unexplained_difference"], 1)
        self.assertEqual(result.findings["unexplained_difference"][0][0], 4)

    def test_a_relocated_field_moved_by_another_amount_fails(self):
        relocs = {0: sf.Reloc(st.R_MIPS_32, "D_80190000", 0x80190000, False)}
        result = sf.compare_words(words(0x80190000), words(0x80190020), relocs, PAD)
        self.assertEqual(result.counts["unexplained_relocated"], 1)

    def test_fixed_targets_and_pins_are_told_apart(self):
        relocs = {0: sf.Reloc(st.R_MIPS_32, "GPU_REG", 0x1F801810, True),
                  4: sf.Reloc(st.R_MIPS_32, "jtbl_80010000", 0x80010000, True)}
        result = sf.compare_words(words(0x1F801810, 0x80010000), words(0x1F801810, 0x80010000),
                                  relocs, PAD)
        self.assertEqual(result.counts["hardware_reference"], 1)
        self.assertEqual(result.counts[sf.DID_NOT_MOVE], 1)
        self.assertFalse(any(result.counts[k] for k in sf.FAILING))

    def test_a_shorter_image_is_reported(self):
        result = sf.compare_words(words(1, 2), words(1), {}, PAD)
        self.assertEqual(result.counts["size_changed"], 1)
        self.assertEqual(result.counts["missing_in_shifted"], 1)


class HeaderTests(unittest.TestCase):
    def header(self, size: int, sp: int = 0x801FFFF0, pc: int = 0x80072534) -> bytes:
        data = bytearray(sf.HEADER_SIZE)
        data[sf.ENTRY_OFFSET:sf.ENTRY_OFFSET + 4] = pc.to_bytes(4, "little")
        data[sf.TEXT_SIZE_OFFSET:sf.TEXT_SIZE_OFFSET + 4] = size.to_bytes(4, "little")
        data[0x30:0x34] = sp.to_bytes(4, "little")
        return bytes(data)

    def test_only_the_entry_point_and_text_size_move_by_the_pad(self):
        result = sf.Comparison()
        sf.compare_header(self.header(0x1EE000), self.header(0x1EE000 + PAD, pc=0x80072534 + PAD),
                          PAD, result)
        self.assertEqual(result.counts["header_text_size"], 1)
        self.assertEqual(result.counts["header_entry_point"], 1)
        self.assertEqual(result.counts["unexplained_difference"], 0)
        result = sf.Comparison()
        sf.compare_header(self.header(0x1EE000), self.header(0x1EE000 + PAD, 0x80200000, 0x80072534 + PAD),
                          PAD, result)
        self.assertEqual(result.counts["unexplained_difference"], 1)

    def test_an_entry_point_that_did_not_move_fails(self):
        result = sf.Comparison()
        sf.compare_header(self.header(0x1EE000), self.header(0x1EE000 + PAD), PAD, result)
        self.assertEqual(result.findings["unexplained_difference"],
                         [(sf.ENTRY_OFFSET, "header initial PC did not move with the startup")])


@unittest.skipUnless((BUILD / "main.map").exists() and (LINKERS / "main.ld").exists()
                     and (BUILD / "overlays" / "mdec_diag" / "mdec_diag.bin").exists()
                     and (LINKERS / "overlays" / "mdec_diag.ld").exists(),
                     "needs `make build` and the mdec_diag overlay built")
class BuiltShiftTests(unittest.TestCase):
    def test_main_and_an_overlay_shift_with_every_difference_explained(self):
        with tempfile.TemporaryDirectory(prefix="shift_full_") as tmp:
            report = sf.measure(0x10, Path(tmp), ["mdec_diag", "room_m005_theater", "room_m022_theater"],
                                jobs=3)
        self.assertTrue(sf.passed(report), sf.render(report, 5))
        self.assertEqual(report["block"]["shifted_base"] - report["block"]["base"], 0x10)
        self.assertEqual(report["main"]["counts"]["header_text_size"], 1)
        self.assertEqual(report["main"]["counts"]["header_entry_point"], 1)
        self.assertGreater(report["overlays"]["counts"]["shifted_address_field"], 0)
        # room_m022's actor class table is a databin blob: its handler
        # addresses stay numbers (RoomLib_HandlerDNop0 at 0x8018F1CC first).
        literal = report["overlays"]["literal_code_words"]
        self.assertEqual(sorted(literal), ["room_m022_theater"])
        self.assertEqual(literal["room_m022_theater"][0][:2], (0x801924E8, 0x8018F1CC))


if __name__ == "__main__":
    unittest.main()
