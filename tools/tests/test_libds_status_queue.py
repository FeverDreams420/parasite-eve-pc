"""Retail-free exact-byte checks for two LIBDS routines, including relocations."""
import hashlib
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]

# Retail instruction streams at 0x800809E0 and 0x8007E594 respectively.
CASES = [('dssys_1_10',
  228,
  '9cf0a1928cb92874823ad99513c1a840740865820bfea90e256b13288e76a292',
  'g_DsStatusHeaderText = 0x80011D74;\nD_80011D94 = 0x80011D94;\nD_80011DB0 = 0x80011DB0;\nD_80011DD8 = 0x80011DD8;\nD_80011DF8 = 0x80011DF8;\nD_80011E0C = 0x80011E0C;\nD_80011E3C = 0x80011E3C;\nD_80011E44 = 0x80011E44;\nD_80011E4C = 0x80011E4C;\ng_DsReadStatusBlock = 0x8009B574;\ng_DsSyncCallback = 0x800A36A4;\ng_DsReadyCallback = 0x800A36A8;\nER_active = 0x800822AC;\ng_DsPollCallback = 0x800A36A0;\nprintf = 0x80071A74;\nSECTIONS { .text 0x800809e0 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }'),
 ('dssys_2',
  1664,
  'c75008ee4feeec03ea466c3a4598009a2b0c8d457a5bc434ea83b805770809d3',
  'DS_cw = 0x8007fb44;\nDS_system_status = 0x8007fbf0;\nD_800A3540 = 0x800a3540;\nD_800A3600 = 0x800a3600;\nD_800A3604 = 0x800a3604;\nD_800A3608 = 0x800a3608;\ng_CdDsReadIndex = 0x800a3604;\ng_CdDsReadQueue = 0x800a3540;\ng_CdDsReadQueueState = 0x800a3600;\ng_CdPendingReadCount = 0x800a3608;\ng_CdQueuedCmdSlots = 0x800a3510;\ng_DsReadCallbackCursor = 0x800a3690;\ng_DsReadCallbackSlots = 0x800a3610;\ng_DsReadCallbackState = 0x800b8ab0;\nrescpy = 0x80080998;\nSECTIONS { .text 0x8007e594 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }')]


class LibdsStatusQueueTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("mipsel-none-elf-ld"), "MIPS binutils unavailable")
    def test_all_linked_bytes_match_retail(self):
        for name, size, digest, script in CASES:
            with self.subTest(function=name), tempfile.TemporaryDirectory() as directory:
                work = pathlib.Path(directory)
                obj, elf, data = work / "code.o", work / "code.elf", work / "code.bin"
                subprocess.run(["tools/scripts/cc.sh", f"src/main/psyq/libds/{name}.c", str(obj)],
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
