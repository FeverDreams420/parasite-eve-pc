"""Full linked-byte regressions for Psy-Q LIBPAD/LIBDS functions."""
import hashlib
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]

# Entire linked text ranges at their retail addresses.
CASES = [('libpad/padseqd',
  1140,
  'e080b43d7fdd01895057432b2eb8509d80c259ecb79f25aa77b5711f3411b504',
  'g_MemCardObjResetFn = 0x8009B728;\n_padCmdParaMode = 0x80083E50;\n_padRecvAtLoadInfo = 0x80083644;\n_padSendAtLoadInfo = 0x800835C0;\ng_MemCardIsTransferActiveFn = 0x8009B740;\ng_MemCardResponseHandler = 0x8009B744;\ng_MemCardStateDispatchFn = 0x8009B73C;\nSECTIONS { .text 0x80084b44 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }'),
 ('libpad/padcmd',
  2388,
  'f4d9851b3c04b09b55295b3653fb9b46c24182d46125ce755235a0d4ac168e3e',
  'g_MemCardObjResetFn = 0x8009B728;\ng_MemCardIsTransferActiveFn = 0x8009B740;\nD_800A5AD0 = 0x800A5AD0;\nfunc_80083C20 = 0x80083C20;\nfunc_80083C3C = 0x80083C3C;\ng_MemCardIsTransferActiveFn = 0x8009B740;\nSECTIONS { .text 0x800835a4 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }'),
 ('libds/dssys_2',
  1664,
  'c75008ee4feeec03ea466c3a4598009a2b0c8d457a5bc434ea83b805770809d3',
  'DS_cw = 0x8007fb44;\nDS_system_status = 0x8007fbf0;\nD_800A3540 = 0x800a3540;\nD_800A3600 = 0x800a3600;\nD_800A3604 = 0x800a3604;\nD_800A3608 = 0x800a3608;\ng_CdDsReadIndex = 0x800a3604;\ng_CdDsReadQueue = 0x800a3540;\ng_CdDsReadQueueState = 0x800a3600;\ng_CdPendingReadCount = 0x800a3608;\ng_CdQueuedCmdSlots = 0x800a3510;\ng_DsReadCallbackCursor = 0x800a3690;\ng_DsReadCallbackSlots = 0x800a3610;\ng_DsReadCallbackState = 0x800b8ab0;\nrescpy = 0x80080998;\nSECTIONS { .text 0x8007e594 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }')]


class PsyqPadDsFourTests(unittest.TestCase):
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
