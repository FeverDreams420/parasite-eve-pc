"""Exact linked-byte regressions for sector DMA and the LIBDS file lookup."""
import hashlib
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]

# Retail TUs at 0x8007BF44 (bios_7.c, LIBCD BIOS part 7) and 0x80081414 (dsfile.c).
CASES = [('libcd/bios_7',
  720,
  'cf57c86e189e44e052836d51bc43c23062405ef42d40be9b66945cb8fc0ebcd2',
  'D_8009AFB4 = 0x8009afb4;\nD_8009AFB8 = 0x8009afb8;\nD_8009B260 = 0x8009b260;\nD_8009B27C = 0x8009b27c;\nD_8009B288 = 0x8009b288;\nD_8009B28C = 0x8009b28c;\nD_8009B2B0 = 0x8009b2b0;\nD_8009B2B4 = 0x8009b2b4;\nD_8009B2B8 = 0x8009b2b8;\nD_8009B2BC = 0x8009b2bc;\ng_CdRegDmaControl = 0x8009b2c0;\nD_800A3460 = 0x800a3460;\nD_800A3468 = 0x800a3468;\ng_CdReadyEvent = 0x8009b295;\ng_CdRegDmaControl = 0x8009b2c0;\ng_CdRegIndexBase = 0x8009b27c;\ng_CdRegRequest = 0x8009b28c;\ng_CdRegResponse = 0x8009b288;\ngetintr = 0x8007aab4;\nSECTIONS { .text 0x8007bf44 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }'),
 ('libds/dsfile',
  768,
  '0e47249fb6a552eda5b9ed324059dcf6a2f680f9d28a61de6a092dbf6c6ba57b',
  'DS_cachefile = 0x80081A7C;\nDS_newmedia = 0x80081714;\nDS_searchdir = 0x800819D8;\nD_8009AFC0 = 0x8009AFC0;\nD_800A36B8 = 0x800A36B8;\nDsShellOpen = 0x8007F7A8;\nprintf = 0x80071A74;\nputs = 0x80073C5C;\nstrncmp = 0x80071A04;\nSECTIONS { .text 0x80081414 : SUBALIGN(4) { *(.text .text.*) } .rodata 0x80011e6c : SUBALIGN(4) { *(.rodata) } .data 0x8009b6dc : SUBALIGN(4) { *(.data) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }')]


class SectorFileSearchTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("mipsel-none-elf-ld"), "MIPS binutils unavailable")
    def test_all_linked_bytes_match_retail(self):
        for name, size, digest, script in CASES:
            with self.subTest(function=name), tempfile.TemporaryDirectory() as directory:
                work = pathlib.Path(directory)
                obj, elf, data = work / "code.o", work / "code.elf", work / "code.bin"
                subprocess.run(["tools/scripts/cc.sh", f"src/main/psyq/{name}.c", str(obj)],
                               cwd=ROOT, check=True, capture_output=True)
                linker_script = work / "code.ld"
                linker_script.write_text(script)
                subprocess.run(["mipsel-none-elf-ld", "-T", str(linker_script),
                                str(obj), "-o", str(elf)], check=True)
                subprocess.run(["mipsel-none-elf-objcopy", "-O", "binary",
                                "--only-section=.text", str(elf), str(data)], check=True)
                code = data.read_bytes()
                self.assertEqual(len(code), size)
                self.assertEqual(hashlib.sha256(code).hexdigest(), digest)
