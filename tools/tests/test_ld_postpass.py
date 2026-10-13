import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.scripts import ld_postpass as lp
from tools.scripts import shift_test as st
from tools.scripts.shift_audit import BUILD, LINKERS, tool

SCRIPT = """\
SECTIONS
{
    main_ROM_START = __romPos;
    .main 0x80010000 : AT(main_ROM_START) SUBALIGN(4)
    {
        FILL(0x00000000);
        main_DATA_START = .;
        build/a.o(.data*);
        main_main_sbss_OFFSET = .;
        build/b.o(.data*);
        main_main_scommon_OFFSET = .;
        build/c.o(.data*);
        . = ALIGN(., 4);
        main_DATA_END = .;
        main_DATA_SIZE = ABSOLUTE(main_DATA_END - main_DATA_START);
    }
    main_bss_VRAM = ADDR(.main_bss);
    .main_bss (NOLOAD) : SUBALIGN(4)
    {
        FILL(0x00000000);
        main_BSS_START = .;
        build/a.o(.bss*);
        build/b.o(.bss*);
        . = ALIGN(., 4);
        main_BSS_END = .;
        main_BSS_SIZE = ABSOLUTE(main_BSS_END - main_BSS_START);
    }
    __romPos += SIZEOF(.main);
    .field_engine_bss (NOLOAD) : SUBALIGN(4)
    {
        field_engine_BSS_SIZE = ABSOLUTE(field_engine_BSS_END - field_engine_BSS_START);
    }
    __romPos += SIZEOF(.field_engine);
    field_engine_ROM_END = __romPos;
    field_engine_VRAM_END = .;

    overlay_windows_ROM_START = __romPos;
    overlay_windows_VRAM = ADDR(.overlay_windows);
    .overlay_windows field_engine_VRAM_END : AT(overlay_windows_ROM_START) SUBALIGN(4)
    {
        build/w.o(.data*);
    }
    __romPos += SIZEOF(.overlay_windows);
    overlay_windows_ROM_END = __romPos;
    overlay_windows_VRAM_END = .;
}
"""

HEADER = """\
.ascii "PS-X EXE"      /* Magic number */
.word 0x80072534       /* Initial PC */
.word 0x80010000       /* .text start */
.word 0x001EE000       /* .text size */
.word 0x801FFFF0       /* Initial $sp/r29 & $fp/r30 base */
.word 0x00000000       /* Initial $sp/r29 & $fp/r30 offset */
"""


class RewriteTests(unittest.TestCase):
    def test_bss_moves_into_main_with_catch_alls_and_startup_symbols(self):
        out = lp.rewrite(SCRIPT)
        self.assertNotIn("NOLOAD) : SUBALIGN(4)\n    {\n        FILL(0x00000000);\n        main_BSS_START", out)
        main = out[out.index(".main 0x80010000"):out.index("__romPos += SIZEOF(.main);")]
        order = ["*(.sdata .sdata.*)", "main_main_sbss_OFFSET", "_edata = .;", "__bss_start = .;",
                 "*(.sbss .sbss.*)", "*(.scommon)", "main_main_scommon_OFFSET", "main_DATA_END",
                 "main_BSS_START = .;", "build/a.o(.bss*);", "build/b.o(.bss*);", "*(COMMON)",
                 "main_BSS_END = .;", "_end = .;"]
        positions = [main.index(s) for s in order]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("main_bss_VRAM = main_BSS_START;", out)
        self.assertIn("ASSERT(SIZEOF(.field_engine_bss) == 0,", out)

    def test_overlay_windows_follow_the_engine_with_the_table_and_stack_limit(self):
        out = lp.rewrite(SCRIPT)
        self.assertIn(".overlay_windows field_engine_VRAM_END : AT(overlay_windows_ROM_START)", out)
        windows = out[out.index("    overlay_windows_VRAM_END = .;"):]
        order = ["INCLUDE linkers/USA/overlay_windows.txt",
                 "PSX_EXE_STACK_TOP = 0x801FFFF0;",
                 "PSX_EXE_TEXT_SIZE = ABSOLUTE(overlay_windows_VRAM_END - main_VRAM);",
                 "ASSERT(overlay_windows_VRAM_END <= PSX_EXE_STACK_TOP,",
                 "ASSERT(overlay_windows_ROM_END - main_ROM_START == PSX_EXE_TEXT_SIZE,",
                 "PSX_EXE_HEADROOM = ABSOLUTE(PSX_EXE_STACK_TOP - overlay_windows_VRAM_END);"]
        positions = [windows.index(s) for s in order]
        self.assertEqual(positions, sorted(positions))
        self.assertNotIn("0x8010BCF8", out)

    def test_header_takes_entry_size_and_stack_from_the_link(self):
        out = lp.rewrite_header(HEADER)
        self.assertIn(".word __SN_ENTRY_POINT       /* Initial PC */", out)
        self.assertIn(".word PSX_EXE_TEXT_SIZE       /* .text size */", out)
        self.assertIn(".word PSX_EXE_STACK_TOP       /* Initial $sp/r29 & $fp/r30 base */", out)
        self.assertEqual(lp.rewrite_header(out), out)
        with self.assertRaises(lp.PostPassError):
            lp.rewrite_header(HEADER.replace("/* .text size */", "/* size */"))

    def test_constructor_tables_open_main(self):
        out = lp.rewrite(SCRIPT)
        opening = "    {\n        FILL(0x00000000);\n        /* SN constructor"
        main = out[out.index(".main 0x80010000"):]
        self.assertTrue(main[main.index("\n") + 1:].startswith(opening))
        self.assertLess(main.index("__sn_ctor_table = .;"), main.index("main_DATA_START"))
        self.assertLess(main.index("__sn_dtor_table = .;"), main.index("main_DATA_START"))

    def test_idempotent(self):
        once = lp.rewrite(SCRIPT)
        self.assertEqual(lp.rewrite(once), once)

    def test_unexpected_shapes_fail(self):
        with self.assertRaises(lp.PostPassError):
            lp.rewrite(SCRIPT.replace("        main_main_scommon_OFFSET = .;\n", ""))
        with self.assertRaises(lp.PostPassError):
            lp.rewrite(SCRIPT.replace("build/b.o(.bss*);", "build/b.o(.sbss*);"))
        with self.assertRaises(lp.PostPassError):
            lp.rewrite(SCRIPT.replace("    __romPos += SIZEOF(.main);\n", ""))
        with self.assertRaises(lp.PostPassError):  # the window block at a fixed address
            lp.rewrite(SCRIPT.replace(".overlay_windows field_engine_VRAM_END :", ".overlay_windows 0x8010BCF8 :"))
        with self.assertRaises(lp.PostPassError):
            lp.rewrite(SCRIPT.replace("    overlay_windows_VRAM_END = .;\n", ""))
        with self.assertRaises(lp.PostPassError):
            lp.rewrite(SCRIPT.replace("    field_engine_VRAM_END = .;\n", ""))


# A unit that adds small data, small and large COMMON and a gp-relative load
# from an executable (not data) section, the shapes MASPSX -G8 units emit.
GROWTH_PROBE = """\
    .set noreorder
    .section .sdata, "aw"
probe_sdata:
    .word 1
    .section .sbss, "aw", @nobits
    .align 2
probe_sbss:
    .space 4
    .comm probe_small, 4
    .comm probe_big, 64
    .section .sdata, "aw"
    lw $2, %gp_rel(probe_small)($28)
    lw $2, %gp_rel(probe_sbss)($28)
"""


WORK_AREA = "build/USA/asm/USA/main/data/engine/boot_work_area.data.s.o(.data*);"
WORK_AREA_SIZE = 0x18800
WINDOW = 0x8010BCF8
HEADROOM = 0x1FF0  # retail image end 0x801FE000, initial $sp 0x801FFFF0


def small_work_area(size: int) -> str:
    """A stand-in for Boot_InitMemoryLayout's work area, SIZE bytes long."""
    return ("    .section .data, \"aw\"\n    .globl g_BootWorkArea\n"
            f"g_BootWorkArea:\n    .space 0x{size:X}\n")


@unittest.skipUnless((BUILD / "main.map").exists() and (LINKERS / "main.ld").exists(),
                     "needs `make build`")
class BuiltLayoutTests(unittest.TestCase):
    def run_link(self, tmp: Path, extra: list[Path], script: Path | None = None):
        out = tmp / "main.elf"
        cmd = [tool("mipsel-none-elf-ld"), "-EL", "-T", str(script or LINKERS / "main.ld"),
               "-T", str(LINKERS / "undefined_syms_manual.txt"),
               "-T", str(LINKERS / "undefined_addr_aliases.main.txt"),
               "-o", str(out), *map(str, extra)]
        return subprocess.run(cmd, cwd=st.ROOT, capture_output=True, text=True), out

    def link(self, tmp: Path, extra: list[Path], script: Path | None = None):
        res, out = self.run_link(tmp, extra, script)
        self.assertEqual(res.returncode, 0, res.stderr[-2000:])
        nm = subprocess.run([tool("mipsel-none-elf-nm"), str(out)], capture_output=True,
                            text=True, check=True).stdout
        return {p[2]: int(p[0], 16) for p in (l.split() for l in nm.splitlines()) if len(p) == 3}

    def assemble(self, tmp: Path, name: str, source: str) -> Path:
        (tmp / f"{name}.s").write_text(source)
        subprocess.run([tool("mipsel-none-elf-as"), "-EL", "-G8", "-march=r3000",
                        "-o", str(tmp / f"{name}.o"), str(tmp / f"{name}.s")], check=True)
        return tmp / f"{name}.o"

    def with_work_area(self, tmp: Path, size: int) -> Path:
        """main.ld with the work area replaced by one of SIZE bytes."""
        obj = self.assemble(tmp, "work_area", small_work_area(size))
        text = (LINKERS / "main.ld").read_text()
        self.assertEqual(text.count(WORK_AREA), 1)
        script = tmp / "main.ld"
        script.write_text(text.replace(WORK_AREA, f"{obj}(.data*);"))
        return script

    def test_padded_relink_links(self):
        with tempfile.TemporaryDirectory(prefix="shift_pad_") as tmp:
            m = st.measure(0x10, Path(tmp))
        self.assertTrue(m.base_identical)
        self.assertEqual(m.pinned, [])
        self.assertEqual(m.headroom, HEADROOM)
        self.assertIsNone(m.plain_errors)  # nothing is fixed: the windows move too
        self.assertIsNone(m.shifted_error)
        self.assertEqual(m.counts["unexplained_difference"], 0)
        self.assertEqual(m.counts["unexplained_relocated"], 0)

    def test_growth_past_the_stack_fails(self):
        big = ("    .section .data, \"aw\"\n    .globl probe_big_table\n"
               f"probe_big_table:\n    .space 0x{HEADROOM + 4:X}\n")
        with tempfile.TemporaryDirectory(prefix="ld_growth_") as tmp:
            tmp = Path(tmp)
            obj = self.assemble(tmp, "big", big)
            text = (LINKERS / "main.ld").read_text().replace(
                "        *(COMMON)\n", f"        *(COMMON)\n        {obj}(.data*);\n", 1)
            (tmp / "main.ld").write_text(text)
            res, _ = self.run_link(tmp, [], tmp / "main.ld")
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("runs into the initial stack", res.stderr)

    def test_new_globals_link_and_everything_after_main_moves(self):
        growth = 4 + 8 + 4 + 4 + 64  # sdata word, two loads, sbss, small, big
        with tempfile.TemporaryDirectory(prefix="ld_growth_") as tmp:
            tmp = Path(tmp)
            base = self.link(tmp, [])
            grown = self.link(tmp, [self.assemble(tmp, "probe", GROWTH_PROBE)])
            shorter = self.link(tmp, [], self.with_work_area(tmp, WORK_AREA_SIZE - 0x100))
        self.assertEqual(base["_end"], base["main_VRAM_END"])
        self.assertEqual(base["__bss_start"], base["_edata"])
        # small data stays in the gp window, before BSS or with the small pool
        self.assertLess(grown["probe_sdata"], grown["_edata"])
        for name in ("probe_sbss", "probe_small"):
            self.assertGreaterEqual(grown[name], grown["__bss_start"])
            self.assertLess(grown[name], grown["main_main_scommon_OFFSET"])
        self.assertGreaterEqual(grown["probe_big"], grown["main_BSS_START"])
        self.assertLess(grown["probe_big"], grown["_end"])
        self.assertEqual(grown["_end"] - base["_end"], growth)
        # the field engine, the work area and the window block follow main in
        # the file and in memory, as one unit
        for syms in (base, grown, shorter):
            self.assertEqual(syms["field_engine_VRAM"], syms["_end"])
            self.assertEqual(syms["field_engine_ROM_START"],
                             syms["field_engine_VRAM"] - 0x80010000 + syms["main_ROM_START"])
            self.assertEqual(syms["overlay_windows_VRAM"], syms["field_engine_VRAM_END"])
            self.assertEqual(syms["overlay_windows_ROM_START"],
                             syms["overlay_windows_VRAM"] - 0x80010000 + syms["main_ROM_START"])
            self.assertEqual(syms["PSX_EXE_TEXT_SIZE"], syms["overlay_windows_VRAM_END"] - 0x80010000)
            self.assertEqual(syms["OVERLAY_WINDOW_ROOM"] - syms["overlay_windows_VRAM"], 0x832F0)
        for name in ("field_engine_VRAM", "FieldFx_TurnToward", "g_FieldFxSlotTable", "g_BootWorkArea",
                     "overlay_windows_VRAM", "OVERLAY_WINDOW_MDEC_DIAG", "OVERLAY_WINDOW_ROOM",
                     "OVERLAY_WINDOW_TASK_PROGRAM", "g_StrFileDirArea", "g_PmTaskProgramArea",
                     "D_801F1F38", "overlay_windows_VRAM_END"):
            self.assertEqual(grown[name] - base[name], growth, name)
            self.assertEqual(shorter[name] - base[name], -0x100 if name not in (
                "field_engine_VRAM", "FieldFx_TurnToward", "g_FieldFxSlotTable", "g_BootWorkArea") else 0,
                name)
        self.assertEqual(base["overlay_windows_VRAM"], WINDOW)
        self.assertEqual(base["PSX_EXE_TEXT_SIZE"], 0x1EE000)
        self.assertEqual(base["PSX_EXE_HEADROOM"], HEADROOM)
        self.assertEqual(grown["PSX_EXE_HEADROOM"], HEADROOM - growth)
        self.assertEqual(shorter["PSX_EXE_HEADROOM"], HEADROOM + 0x100)


if __name__ == "__main__":
    unittest.main()
