import pathlib
import tempfile
import unittest

from tools.scripts import overlay_link_symbols


class LinkedObjectsTests(unittest.TestCase):
    def test_reads_only_the_objects_the_script_links(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            build = root / "build" / "USA" / "overlays" / "room_x"
            script = root / "room_x.ld"
            script.write_text(
                "SECTIONS {\n"
                f"    {build}/src/overlays/room_lib/Lib.c.o(.rodata);\n"
                f"    {build}/src/overlays/room_lib/Lib.c.o(.text);\n"
                f"    {build}/asm/USA/overlays/room_x/data/D_1.data.s.o(.data);\n"
                "}\n")
            objects = overlay_link_symbols.linked_objects(script, build)

        self.assertEqual(objects, [
            build / "asm/USA/overlays/room_x/data/D_1.data.s.o",
            build / "src/overlays/room_lib/Lib.c.o",
        ])


class ConfigWindowTests(unittest.TestCase):
    def test_segments_end_at_the_next_start_or_the_terminator(self):
        with tempfile.TemporaryDirectory() as tmp:
            cfg = pathlib.Path(tmp) / "room_x.yaml"
            cfg.write_text(
                "segments:\n"
                "  - name: room_x\n"
                "    type: code\n"
                "    start: 0x0\n"
                "    vram: 0x8018EFE8\n"
                "  - name: room_x_tail\n"
                "    type: code\n"
                "    start: 0x100\n"
                "    vram: 0x80190000\n"
                "  - [0x180]\n")
            windows = overlay_link_symbols.config_windows(cfg)

        self.assertEqual(windows, [(0x8018EFE8, 0x8018F0E8), (0x80190000, 0x80190080)])


class AssignmentTests(unittest.TestCase):
    def test_reads_plain_provide_and_commented_assignments(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / "abi.txt"
            path.write_text(
                "/* owner: x\n * reason: y = 0x1; */\n"
                "D_80190000 = 0x80190000; /* room_x */\n"
                "PROVIDE(g_Thing = 0x80190010);\n"
                "// provide: g_Other = 0x80190020;\n")
            found = overlay_link_symbols.read_assignments(path)

        self.assertEqual(found, {"D_80190000": 0x80190000, "g_Thing": 0x80190010,
                                 "g_Other": 0x80190020})


class WindowOffsetTests(unittest.TestCase):
    def test_window_offsets_read_as_retail_addresses(self):
        table = overlay_link_symbols.overlay_windows.WindowTable(
            (("OVERLAY_WINDOW_A", 0x0), ("OVERLAY_WINDOW_ROOM", 0x832F0)), 0x8010BCF8, 0x801FE000)
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / "abi.txt"
            path.write_text("D_80190000 = OVERLAY_WINDOW_ROOM + 0x1018; /* room_x */\n"
                            "func_8010BD4C = OVERLAY_WINDOW_A + 0x54;\n")
            found = overlay_link_symbols.read_assignments(path, table)
            path.write_text("D_1 = OVERLAY_WINDOW_NOPE + 0x4;\n")
            with self.assertRaises(SystemExit):
                overlay_link_symbols.read_assignments(path, table)

        self.assertEqual(found, {"D_80190000": 0x80190000, "func_8010BD4C": 0x8010BD4C})


class RegionTests(unittest.TestCase):
    def test_hardware_and_kernel_are_outside_the_program(self):
        self.assertTrue(overlay_link_symbols.outside_program(0x1F801070))
        self.assertTrue(overlay_link_symbols.outside_program(0x80000004))
        self.assertFalse(overlay_link_symbols.outside_program(0x8009D254))


if __name__ == "__main__":
    unittest.main()
