import pathlib
import shutil
import subprocess
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]


class SpuCommonLayoutTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_retail_field_offsets(self):
        # Psy-Q SpuCommonAttr (SpuSetCommonAttr argument) as used by AKAO.
        offsets = {
            "mask": 0x00, "mvol.left": 0x04, "mvol.right": 0x06,
            "mvolmode.left": 0x08, "mvolmode.right": 0x0A,
            "mvolx.left": 0x0C, "mvolx.right": 0x0E,
            "cd.volume.left": 0x10, "cd.volume.right": 0x12,
            "cd.reverb": 0x14, "cd.mix": 0x18,
            "ext.volume.left": 0x1C, "ext.volume.right": 0x1E,
            "ext.reverb": 0x20, "ext.mix": 0x24,
        }
        source = ('#include <stddef.h>\n#include "pe1/akao/spu_common.h"\n'
                  '#include "pe1/psyq_spu_reverb.h"\n')
        source += '_Static_assert(sizeof(SpuCommonAttr) == 0x28, "size");\n'
        source += (
            '_Static_assert(sizeof(SpuReverbRegisterAttrs) == 0x44, "reverb size");\n'
            '_Static_assert(offsetof(SpuReverbRegisterAttrs, mask) == 0, "mask");\n'
            '_Static_assert(offsetof(SpuReverbRegisterAttrs, regs) == 4, "regs");\n'
            '_Static_assert(offsetof(SpuReverbRegisterAttrs, regs[31]) == 0x42, '
            '"last reverb register");\n'
        )
        for field, offset in offsets.items():
            source += (
                f'_Static_assert(offsetof(SpuCommonAttr, {field}) == '
                f'{offset}, "{field}");\n'
            )
        result = subprocess.run(
            ["cc", "-std=c11", "-Werror", "-fsyntax-only", "-x", "c",
             "-I", str(ROOT / "include"), "-"],
            input=source, text=True, capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()
