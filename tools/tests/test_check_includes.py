import pathlib
import tempfile
import unittest

from tools.scripts import check_includes


def errors(name, text):
    with tempfile.TemporaryDirectory() as tmp:
        path = pathlib.Path(tmp) / name
        (pathlib.Path(tmp) / "local.h").write_text("#ifndef L\n#define L\n#endif\n")
        path.write_text(text)
        return check_includes.errors_for(path, text)


def fixed(name, text):
    with tempfile.TemporaryDirectory() as tmp:
        path = pathlib.Path(tmp) / name
        (pathlib.Path(tmp) / "local.h").write_text("#ifndef L\n#define L\n#endif\n")
        return check_includes.fix(path, text)[0]


class CheckIncludesTests(unittest.TestCase):
    def test_tidy_unit_passes(self):
        text = ('/* CC1_FLAGS: -G8 */\n\n#include "common.h"\n#include "pe1/gte.h"\n'
                '#include "m2c_macros.h"\n\n#include "local.h"\n\nint x;\n')
        self.assertEqual(errors("a.c", text), [])

    def test_include_after_code_needs_a_reason(self):
        late = '#include "common.h"\n\nint x;\n#include "pe1/gte.h"\n'
        self.assertTrue(any("after code" in e for e in errors("a.c", late)))
        marked = ('#include "common.h"\n\nint x;\n'
                  '/* Late include: x must stay small. */\n#include "pe1/gte.h"\n')
        self.assertEqual(errors("a.c", marked), [])

    def test_templates_may_repeat_anywhere(self):
        text = '#include "common.h"\n\nint x;\n#include "t.inc"\n#include "t.inc"\n'
        self.assertEqual(errors("a.c", text), [])

    def test_duplicate_through_another_spelling(self):
        text = '#include "common.h"\n#include <common.h>\n'
        found = errors("a.c", text)
        self.assertTrue(any("already included" in e for e in found))
        self.assertTrue(any('spell it #include "common.h"' in e for e in found))

    def test_header_needs_a_guard(self):
        self.assertIn("header without an include guard",
                      errors("a.h", '#include "common.h"\nint x;\n'))
        self.assertEqual(errors("a.h", '#ifndef A_H\n#define A_H\n#include "common.h"\n#endif\n'), [])

    def test_order_and_fix(self):
        text = ('#include "local.h"\n#include "pe1/gte.h"\n#include "common.h"\n\n'
                'int x;\n#include "pe1/gte.h"\n\nint y;\n')
        self.assertTrue(any("not in order" in e for e in errors("a.c", text)))
        self.assertEqual(fixed("a.c", text),
                         '#include "common.h"\n#include "pe1/gte.h"\n\n#include "local.h"\n\n'
                         'int x;\n\nint y;\n')


if __name__ == "__main__":
    unittest.main()
