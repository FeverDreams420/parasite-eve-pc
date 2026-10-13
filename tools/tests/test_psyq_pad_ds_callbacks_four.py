"""Full linked-byte regression for four Psy-Q LIBPAD/LIBDS functions."""
import hashlib
import pathlib
import shutil
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import linked_range_bytes  # noqa: E402

# Entire linked text ranges at their retail addresses.
CASES = [('libpad/padportd',
  1628,
  'bb07a5439818d8e0df4141a1e8b246b51e769fb4ab6d663a746d17589216bb02',
  'g_PadDmaCompleteFn = 0x8009b724;\nD_8009B72C = 0x8009b72c;\nD_8009B730 = 0x8009b730;\nD_8009B734 = 0x8009b734;\nD_8009B748 = 0x8009b748;\nD_8009B764 = 0x8009b764;\nD_8009B768 = 0x8009b768;\nD_8009B76C = 0x8009b76c;\nD_8009B77C = 0x8009b77c;\nD_8009B7BC = 0x8009b7bc;\nD_800A5AE0 = 0x800a5ae0;\nD_800A5B28 = 0x800a5b28;\nD_800A5B70 = 0x800a5b70;\n_dirFailAuto = 0x80084eb0;\n_padInitDirSeq = 0x80084b44;\n_padInitSioMode = 0x80082e00;\n_padSetVsyncParam = 0x80082adc;\nbzero = 0x80071a24;\ng_MemCardCommandByte = 0x8009b770;\ng_MemCardObjArray = 0x8009b758;\ng_MemCardObjLookupFn = 0x8009b738;\ng_MemCardObjResetFn = 0x8009b728;\ng_MemCardPort2Present = 0x8009b778;\ng_MemCardServiceReady = 0x8009b75c;\nSECTIONS { .text 0x800844e4 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }'),
 ('libds/dssys1_vsync',
  720,
  '7df21a00b56e81e4d4a811bd38215db6ff65f4246173802e2e0beb262734fabd',
  'g_CdRomCmdTimeout = 0x8009B598;\nLIBDS_DSSYS_1_text_774 = 0x800800F4;\ng_DsSyncResultCountdown = 0x8009B594;\ng_CdReadProgress = 0x8009B6A4;\nLIBDS_DSSYS_1_text_368 = 0x8007FCFC;\ng_DsPollCallback = 0x800A36A0;\ng_DsReadSysEnabled = 0x8009B554;\ng_DsReadStatusBlock = 0x8009B574;\ng_CdRomCommandEventValue = 0x8009B570;\nSECTIONS { .text 0x8007fe24 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }'),
 ('libds/dsready_2',
  1180,
  '5a4b8d9b13a0a3c718960571cbd9d3e24b54a5c9721e897dbf8b7a13e1d4dac1',
  'DS_lastmode = 0x8007fc18;\nDS_lastpos = 0x8007fc28;\nD_8009B708 = 0x8009b708;\nDsCommand = 0x8007ee84;\nDsDataCallback = 0x800824f0;\nDsGetSector = 0x80080ae4;\nDsPacket = 0x8007f0c8;\nDsPosToInt = 0x80080c48;\nDsQueueLen = 0x8007f778;\nDsReadyCallback = 0x800824c8;\nDsStartCallback = 0x800824dc;\nfunc_8007FC44 = 0x8007fc44;\nfunc_8008227C = 0x8008227c;\ng_DsAsyncReadState = 0x8009b6ec;\ng_DsReadBusy = 0x8009b70c;\nSECTIONS { .text 0x80081e70 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }'),
 ('libpad/padif',
  1504,
  '338fd1ade835fb0bc2491c596f668f5f176d0410b4b93296693aca93b2d2f1ac',
  'g_PadDmaCompleteFn = 0x8009b724;\nD_8009B72C = 0x8009b72c;\nD_8009B730 = 0x8009b730;\ng_MemCardResponseHandler = 0x8009b744;\nD_8009B748 = 0x8009b748;\nD_8009B750 = 0x8009b750;\nD_8009B754 = 0x8009b754;\ng_MemCardObjArray = 0x8009b758;\nD_8009B760 = 0x8009b760;\nD_8009B764 = 0x8009b764;\ng_MemCardCommandByte = 0x8009b770;\nD_8009B77C = 0x8009b77c;\ng_MemCardWriteBlockCount = 0x8009b79c;\nD_8009B7A0 = 0x8009b7a0;\nMemCard_WaitReadyForTransfer = 0x800834e8;\nMemCard_WaitStatusBit2 = 0x80083578;\nMemCard_WriteByte = 0x800832b4;\n_padSioRW = 0x800830dc;\ng_MemCardCommandByte = 0x8009b770;\ng_MemCardDispatchResult = 0x8009b7a4;\ng_MemCardPort1Present = 0x8009b774;\ng_MemCardStateDispatchFn = 0x8009b73c;\ng_MemCardWriteBlockCount = 0x8009b79c;\nsetRC2wait = 0x80084fc4;\nSECTIONS { .text 0x80083f04 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }')]


class PsyqPadDsCallbacksFourTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("mipsel-none-elf-ld"), "MIPS binutils unavailable")
    def test_all_linked_bytes_match_retail(self):
        for name, size, digest, script in CASES:
            with self.subTest(function=name), tempfile.TemporaryDirectory() as directory:
                work = pathlib.Path(directory)
                symbols, sections = script.split("SECTIONS", 1)
                address = int(sections.split()[2], 16)
                # Units churn; link whichever current units cover the range.
                code = linked_range_bytes(address, size, symbols, work)
                self.assertEqual(len(code), size)
                self.assertEqual(hashlib.sha256(code).hexdigest(), digest)
