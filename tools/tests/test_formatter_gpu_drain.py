"""Full linked retail bytes, including the formatter's owned data and padding."""
import hashlib
import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, linked_function_bytes  # noqa: E402

# Whole-unit section checks: the unit owns its .rodata/.data.
CASES = [('sprintf',
  '\n'
  'strlen = 0x80072314;\n'
  'memchr = 0x80072324;\n'
  'memmove = 0x80072334;\n'
  'SECTIONS {\n'
  ' .text 0x80071A84 : SUBALIGN(4) { *(.text) }\n'
  ' .rodata 0x8001161C : SUBALIGN(4) { *(.rodata) }\n'
  ' .data 0x80094528 : SUBALIGN(4) { *(.data) }\n'
  ' /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.gnu.attributes) }\n'
  '}\n',
  [('.text', 2192, '3f21dbc67784752f7ff40a3661fa4c650b3efa4c427f4be5c35d2889ff6ec866'),
   ('.rodata', 220, '90be5217bf0b4b3cff6e499901737f5f655eaa5c5e70acb5f2a2500e94adb6c9'),
   ('.data', 12, '15ec7bf0b50732b49f8228e07d24365338f9e3ab994b00af08e5a3bffe55fd8b')])]

# Function ranges inside merged units: the unit is linked so the function
# lands at its retail address, then the retail-sized range is hashed.
RANGES = [('Gpu_SetDisplayBuffer', 0x80076EE4, 608,
           'g_GpuDmaChcrPtr = 0x80095860;\n'
           'SetIntrMask = 0x80073E10;\n'
           'g_GpuDmaQueueHead = 0x80095874;\n'
           'g_GpuDmaQueueTail = 0x80095878;\n'
           'g_GpuSavedIntrMaskDrain = 0x80095880;\n'
           'g_GpuSystemState = 0x8009574C;\n'
           'DMACallback = 0x80073CF4;\n'
           'g_GpuDmaRegisterWindow = 0x80095854;\n'
           'g_GpuDmaQueue = 0x800BD030;\n',
           'a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c')]


class FormatterGpuDrainTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("mipsel-none-elf-ld"), "MIPS binutils unavailable")
    def test_all_linked_sections_match_retail(self):
        for name, layout, sections in CASES:
            with self.subTest(function=name), tempfile.TemporaryDirectory() as directory:
                work = pathlib.Path(directory)
                obj, elf = work / "code.o", work / "code.elf"
                subprocess.run(["tools/scripts/cc.sh", str(find_definition(name)), str(obj)],
                               cwd=ROOT, check=True, capture_output=True)
                script = work / "code.ld"
                script.write_text(layout)
                subprocess.run(["mipsel-none-elf-ld", "-T", str(script), str(obj),
                                "-o", str(elf)], check=True)
                for section, size, digest in sections:
                    with self.subTest(section=section):
                        data = work / "section.bin"
                        subprocess.run(["mipsel-none-elf-objcopy", "-O", "binary",
                                        f"--only-section={section}", str(elf), str(data)],
                                       check=True)
                        code = data.read_bytes()
                        self.assertEqual(len(code), size)
                        self.assertEqual(hashlib.sha256(code).hexdigest(), digest)
