import contextlib
import io
import pathlib
import tempfile
import unittest
from unittest.mock import patch

from tools.scripts import try_drop_pins


# Lines whose pin match used to swallow every asm except the last one. The
# shapes come from real units (libpad, render object helpers, the scene_e20
# hover orb); a fixture is used because the live pins keep being removed.
FIXTURE = '''int f(int a, int b, int c) {
    register int x asm("$7"), y asm("$6");
    register int p asm("$20"), q asm("$21"), r asm("$22");
    register short *u asm("$12"), *v asm("$13"), *w asm("$14");
    register int single asm("$16");
    return a + b + c + x + y + p + q + r + *u + *v + *w + single;
}
'''
CLAUSE_LINES = (
    (2, ('asm("$7")', 'asm("$6")')),
    (3, ('asm("$20")', 'asm("$21")', 'asm("$22")')),
    (4, ('asm("$12")', 'asm("$13")', 'asm("$14")')),
    (5, ('asm("$16")',)),
)


class _Baseline(object):
    def unlink(self):
        pass


class TryDropPinsClauseTests(unittest.TestCase):
    def test_main_tries_every_asm_clause_on_the_line(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "pins.c"
            path.write_text(FIXTURE)
            attempted = []

            def try_edit(src, text, candidate, baseline):
                attempted.append(candidate)
                return False

            with patch.object(try_drop_pins, "snapshot", return_value=_Baseline()), \
                 patch.object(try_drop_pins, "try_edit", side_effect=try_edit), \
                 patch.object(try_drop_pins, "compile_one", return_value=True), \
                 contextlib.redirect_stdout(io.StringIO()):
                try_drop_pins.main([str(path)])

            self.assertEqual(path.read_text(), FIXTURE)

        original = FIXTURE.splitlines()
        for lineno, expected in CLAUSE_LINES:
            with self.subTest(line=lineno):
                removed = []
                for candidate in attempted:
                    after = candidate.splitlines()[lineno - 1]
                    if original[lineno - 1] == after:
                        continue
                    gone = [clause for clause in expected if clause not in after]
                    kept = [clause for clause in expected if clause in after]
                    self.assertEqual(len(gone), 1)
                    self.assertEqual(kept, [clause for clause in expected if clause != gone[0]])
                    removed.append(gone[0])
                self.assertEqual(tuple(removed), expected)

    def test_kept_removal_is_written_back(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "pins.c"
            path.write_text(FIXTURE)

            def try_edit(src, text, candidate, baseline):
                # Only the single pin is redundant; every other one is load-bearing.
                return 'asm("$16")' in text and candidate == text.replace(' asm("$16")', '')

            with patch.object(try_drop_pins, "snapshot", return_value=_Baseline()), \
                 patch.object(try_drop_pins, "try_edit", side_effect=try_edit), \
                 patch.object(try_drop_pins, "compile_one", return_value=True), \
                 contextlib.redirect_stdout(io.StringIO()):
                try_drop_pins.main([str(path)])

            self.assertEqual(path.read_text(),
                             FIXTURE.replace(' asm("$16")', ''))


if __name__ == "__main__":
    unittest.main()
