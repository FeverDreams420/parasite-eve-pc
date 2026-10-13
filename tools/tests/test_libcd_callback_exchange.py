"""Retail-free exact-byte checks for LIBCD callback exchange, including relocations."""
import hashlib
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]

# Entire sys_2 TU at 0x8007A3CC (SYS.OBJ part 2, CdFlush to CdDataSync).
# CdSyncCallback and CdReadyCallback sit after CdReady.
CASES = [('sys_2',
  1380,
  '2f727289bc6dce6fc794dc321cc61b2c97dce1a5db4c3b1ad2d047f67860bdfc',
  'CD_cw = 0x8007b558;\nCD_datasync = 0x8007bddc;\nCD_debug = 0x8009afc0;\nCD_flush = 0x8007b9ec;\nCD_getsector = 0x8007bf44;\nCD_getsector2 = 0x8007c044;\nCD_ready = 0x8007b290;\nCD_sync = 0x8007b010;\nCD_vol = 0x8007b964;\nDMACallback = 0x80073cf4;\nD_8009AFB4 = 0x8009afb4;\nD_8009AFB8 = 0x8009afb8;\nD_8009AFC4 = 0x8009afc4;\ng_CdNoneString = 0x800119cc;\ng_CdCmdNameTable = 0x8009afdc;\ng_CdIntrStringTable = 0x8009b05c;\nSECTIONS { .text 0x8007a3cc : SUBALIGN(4) { *(.text .text.*) } .data 0x8009af2c : SUBALIGN(4) { *(.data .data.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }')]


class LibcdCallbackExchangeTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("mipsel-none-elf-ld"), "MIPS binutils unavailable")
    def test_all_linked_bytes_match_retail(self):
        for name, size, digest, script in CASES:
            with self.subTest(function=name), tempfile.TemporaryDirectory() as directory:
                work = pathlib.Path(directory)
                obj, elf, data = work / "code.o", work / "code.elf", work / "code.bin"
                subprocess.run(["tools/scripts/cc.sh", f"src/main/psyq/libcd/{name}.c", str(obj)],
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
