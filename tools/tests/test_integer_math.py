"""Exact linked-byte regressions for the integer conversion and wide multiplication."""
import hashlib
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]

# Entire linked text ranges at their retail addresses.
CASES = [('psyq/libmath/fltsidf',
  256,
  'e573de21ab0c5cc4ee09974e395737ca6cf3330c3cecc77d21b259f6365b46ba',
  '_dbl_shift_us = 0x80073294;\n'
  'SECTIONS { .text 0x80073454 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) '
  '*(.mdebug) } }'),
 ('psyq/libmath/muldf3',
  1024,
  'e1aebadc811ba84da2cc8868a33f9df18ff73f5ac4d1d92d2bce95f710aa1c6a',
  '_add_mant_d = 0x80072df4;\n_dbl_shift_us = 0x80073294;\n_err_math = 0x800739c4;\nSECTIONS { .text 0x800735c4 : SUBALIGN(4) { *(.text .text.*) } /DISCARD/ : { *(.reginfo) *(.mdebug) } }')]


class IntegerMathTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("mipsel-none-elf-ld"), "MIPS binutils unavailable")
    def test_all_linked_bytes_match_retail(self):
        for name, size, digest, script in CASES:
            with self.subTest(function=name), tempfile.TemporaryDirectory() as directory:
                work = pathlib.Path(directory)
                obj, elf, data = work / "code.o", work / "code.elf", work / "code.bin"
                subprocess.run(["tools/scripts/cc.sh", f"src/main/{name}.c", str(obj)],
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
