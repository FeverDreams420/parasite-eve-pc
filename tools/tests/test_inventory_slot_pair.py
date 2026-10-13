"""Exact linked-byte regressions for contiguous-slot search and slot compaction."""
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
CASES = [('item/active_list',
  316,
  'c9d79218d158c686643796e7641610c0660987624d9e6e5eb192617e2f91379c',
  'g_InvSelectionBitWords = 0x8009D064;\n'
  'g_InvSelectionBits = 0x8009D058;\n'
  'g_InvSlotLimit = 0x8009D050;\n'
  'g_InvItemPtr = 0x8009D048;\n'
  'g_AyaInventoryItems = 0x800C0E48;\n'
  'Inv_GetAyaSlotLimit = 0x80052F70;\n'
  'g_InvSelectionBitStorage = 0x8009D05C;\n'
  '_gp = 0x8009CD70;\n'
  'SECTIONS { .text 0x8005382c : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) '
  '*(.mdebug) } }'),
 ('item/Inv_CompactActiveListSlots',
  340,
  '92e03cc5341f56da9b4f4785622a9aff53e901a49b0303791fca286ba255f95b',
  'g_InvSlotLimit = 0x8009D050;\n'
  'g_InvItemPtr = 0x8009D048;\n'
  'g_InvPendingCompactCount = 0x8009D06C;\n'
  'g_InventoryRuntime = 0x800C0E20;\n'
  'Inv_GetBonusSlotCount = 0x80051E58;\n'
  'g_InvBaseCapacity = 0x800C0E0C;\n'
  '_gp = 0x8009CD70;\n'
  'SECTIONS { .text 0x80054cf8 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) '
  '*(.mdebug) } }')]


class InventorySlotPairTests(unittest.TestCase):
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
