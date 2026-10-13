import pathlib
import re
import tempfile
import unittest

from tools.scripts import overlay_windows as ow

TABLE = ow.WindowTable((("OVERLAY_WINDOW_A", 0x0), ("OVERLAY_WINDOW_B", 0x100),
                        ("OVERLAY_WINDOW_C", 0x1000)), 0x80100000, 0x80102000)


class TableTests(unittest.TestCase):
    def test_locate_uses_the_window_an_address_falls_in(self):
        self.assertEqual(TABLE.locate(0x80100000), ("OVERLAY_WINDOW_A", 0))
        self.assertEqual(TABLE.locate(0x801000FC), ("OVERLAY_WINDOW_A", 0xFC))
        self.assertEqual(TABLE.locate(0x80100100), ("OVERLAY_WINDOW_B", 0))
        self.assertEqual(TABLE.locate(0x80101FFF), ("OVERLAY_WINDOW_C", 0xFFF))
        for outside in (0x800FFFFC, 0x80102000):
            with self.assertRaises(ow.WindowError):
                TABLE.locate(outside)

    def test_expression_round_trips_at_any_base(self):
        self.assertEqual(TABLE.expression(0x80100180), "OVERLAY_WINDOW_B + 0x80")
        self.assertEqual(TABLE.evaluate("OVERLAY_WINDOW_B + 0x80"), 0x80100180)
        self.assertEqual(TABLE.evaluate("(OVERLAY_WINDOW_B + 0x80)", base=0x80100010), 0x80100190)
        self.assertIsNone(TABLE.evaluate("0x80100180"))
        self.assertIsNone(TABLE.evaluate("OVERLAY_WINDOW_Z + 0x4"))

    def test_table_shape_is_checked(self):
        with self.assertRaises(ow.WindowError):
            ow.WindowTable((("OVERLAY_WINDOW_A", 0x10),), 0x80100000, 0x80102000)
        with self.assertRaises(ow.WindowError):
            ow.WindowTable((("OVERLAY_WINDOW_A", 0x0), ("OVERLAY_WINDOW_B", 0x0)), 0x80100000, 0x80102000)

    def test_rebase_ld_rewrites_section_addresses(self):
        script = ("SECTIONS\n{\n"
                  "    .ovl_a 0x80100104 : AT(ovl_a_ROM_START) SUBALIGN(4)\n"
                  "    {\n        FILL(0x00000000);\n    }\n"
                  "    .ovl_b 0x80101000 : AT(ovl_b_ROM_START) SUBALIGN(4)\n}\n")
        out = ow.rebase_ld(script, TABLE)
        self.assertIn(".ovl_a (OVERLAY_WINDOW_B + 0x4) : AT(ovl_a_ROM_START)", out)
        self.assertIn(".ovl_b (OVERLAY_WINDOW_C + 0x0) : AT(ovl_b_ROM_START)", out)
        self.assertIn("FILL(0x00000000);", out)
        self.assertEqual(ow.rebase_ld(out, TABLE), out)
        with self.assertRaises(ow.WindowError):
            ow.rebase_ld("    .ovl_c 0x80200000 : AT(x)\n", TABLE)


class RepositoryTableTests(unittest.TestCase):
    def test_the_table_matches_the_retail_layout(self):
        table = ow.load()
        self.assertEqual((table.retail_base, table.retail_end), (0x8010BCF8, 0x801FE000))
        self.assertEqual(ow.check(table), [])
        retail = {name: table.retail_base + off for name, off in table.windows}
        self.assertEqual(retail["OVERLAY_WINDOW_ROOM"], 0x8018EFE8)
        # the comments name each window's retail address
        text = ow.TABLE.read_text()
        for name, addr in retail.items():
            self.assertRegex(text, rf"{name} = .*/\* 0x{addr:08X}")

    def test_check_reports_segments_outside_the_block(self):
        with tempfile.TemporaryDirectory() as tmp:
            cfg = pathlib.Path(tmp) / "x.yaml"
            cfg.write_text("segments:\n  - name: x\n    type: code\n    start: 0x0\n"
                           "    vram: 0x80100000\n  - name: y\n    type: code\n    start: 0x10\n"
                           "    vram: 0x80300000\n  - [0x20]\n")
            problems = ow.check(TABLE, pathlib.Path(tmp), pathlib.Path(tmp) / "none.txt")
        self.assertTrue(any("segment y at 0x80300000 is outside" in p for p in problems), problems)
        self.assertTrue(any(re.match(r"OVERLAY_WINDOW_B .* neither", p) for p in problems), problems)


if __name__ == "__main__":
    unittest.main()
