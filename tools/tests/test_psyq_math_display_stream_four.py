"""Full linked-byte regression for four Psy-Q LIBMATH/LIBGPU/LIBCD functions."""
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
CASES = [('libmath/divdf3',
  816,
  '3b46f2ee4ed2d17e27f2c2e19afe53b7fa403a7333b3e9b633dd2551bfccd101',
  '_dbl_shift = 0x80072EA4;\n'
  '_mainasu = 0x80073554;\n'
  '_add_mant_d = 0x80072DF4;\n'
  '_dbl_shift_us = 0x80073294;\n'
  '_err_math = 0x800739C4;\n'
  'SECTIONS { .text 0x80072f64 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) '
  '*(.mdebug) } }'),
 ('libmath/adddf3',
  884,
  '3060f098b67be7758d285c025aca8d2dfc4db7a0b07501b9c929bdb914f11835',
  '_mainasu = 0x80073554;\n'
  '_dbl_shift = 0x80072EA4;\n'
  '_add_mant_d = 0x80072DF4;\n'
  '_err_math = 0x800739C4;\n'
  'SECTIONS { .text 0x80072a74 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) '
  '*(.mdebug) } }'),
 ('libgpu/PutDispEnv',
  1272,
  'c8dc390773e463aadf9158ed73f3cb4467a6c8b2af8f853c3950512ef0577ca1',
  'g_GraphDebug = 0x8009574E;\n'
  'g_PutDispEnvFormat = 0x80011970;\n'
  'g_GpuDebugPrintf = 0x80095748;\n'
  'g_GpuCallbacks = 0x80095744;\n'
  'GetVideoMode = 0x80074A28;\n'
  'D_8009574F = 0x8009574F;\n'
  'D_800957C0 = 0x800957C0;\n'
  'g_GpuDotClockDividers = 0x80095848;\n'
  'g_GpuHRangeTable = 0x80095820;\n'
  'D_80095822 = 0x80095822;\n'
  'g_GpuActiveDispEnv = 0x800957B8;\n'
  'memcpy = 0x80071A34;\n'
  'SECTIONS { .text 0x800755f0 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) '
  '*(.mdebug) } }'),
 ('libcd/StCdInterrupt',
  2332,
  '7c4ca8096a8921313e5b9a5f036f2b9341d656633afed2132b62af8d40aa8c74',
  'D_800B89F4 = 0x800B89F4;\n'
  'D_800A801C = 0x800A801C;\n'
  'g_CdStIntrMdecDmaChcrPtr = 0x8009B34C;\n'
  'g_CdStreamMemorySource = 0x800C0DB8;\n'
  'g_CdStreamReady = 0x800B0CD0;\n'
  'g_CdStreamSectorSlot = 0x800BCD7C;\n'
  'g_CdStIntrState = 0x8009B374;\n'
  'CdReady = 0x8007A488;\n'
  'g_CdStreamRingWriteSlot = 0x800BE998;\n'
  'D_800C0DC8 = 0x800C0DC8;\n'
  'g_StCurrentHeader = 0x800A34A0;\n'
  'g_CdStIntrIndexRegPtr = 0x8009B32C;\n'
  'g_CdStIntrRequestRegPtr = 0x8009B338;\n'
  'g_CdStIntrDelayRegPtr = 0x8009B33C;\n'
  'g_CdStIntrComDelayRegPtr = 0x8009B340;\n'
  'D_800A8020 = 0x800A8020;\n'
  'g_CdStIntrDataFifoPtr = 0x8009B334;\n'
  'mem2mem = 0x8007CE80;\n'
  'dma_execute = 0x8007CEAC;\n'
  'g_CdStIntrDmaChcrPtr = 0x8009B35C;\n'
  'D_800C0DC0 = 0x800C0DC0;\n'
  'g_CdStreamStartFrame = 0x800B6918;\n'
  'g_CdStreamExpectedTypeId = 0x800B8620;\n'
  'g_StFrameSectorIndex = 0x800A8018;\n'
  'g_StCurrentFrameNumber = 0x800A5D54;\n'
  'D_800BE9E4 = 0x800BE9E4;\n'
  'init_ring_status = 0x8007C444;\n'
  'D_800C0DBC = 0x800C0DBC;\n'
  'g_StrEndCallback = 0x800B0CCC;\n'
  'D_800C20C4 = 0x800C20C4;\n'
  'g_CdStreamSectorPayloadPtr = 0x800C0DC4;\n'
  'D_800B6914 = 0x800B6914;\n'
  'data_ready_callback = 0x8007C214;\n'
  'SECTIONS { .text 0x8007c564 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) '
  '*(.mdebug) } }')]


class PsyqMathDisplayStreamFourTests(unittest.TestCase):
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
