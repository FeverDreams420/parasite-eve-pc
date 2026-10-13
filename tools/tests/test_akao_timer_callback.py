"""Regression gates for timer-history scalar declarations (one pin remains)."""
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import find_definition, function_text  # noqa: E402


class AkaoTimerCallbackTests(unittest.TestCase):
    def test_scalar_history_declarations(self):
        unit = re.sub(r'/\*.*?\*/|//[^\n]*', '', find_definition('Akao_TimerCallback').read_text(), flags=re.S)
        source = re.sub(r'/\*.*?\*/|//[^\n]*', '', function_text('Akao_TimerCallback'), flags=re.S)
        self.assertNotIn('__asm__',source)
        self.assertNotIn('char _[16]',unit)
        header = (ROOT / 'include/pe1/akao/globals.h').read_text()
        for name in ('g_AkaoTimerDeltaHist3','g_AkaoTimerDeltaHist2','g_AkaoTimerDeltaHist1',
                     'g_AkaoTimerDeltaHist0','g_AkaoTimerDeltaSum'):
            self.assertIn(name, source)
            self.assertIn('extern s32 '+name+';',header)
            self.assertNotRegex(unit, r'\b'+name+r'\s*__asm__')
        # Partial cleanup, not a claim that the remaining pin is required.
        self.assertLessEqual(len(re.findall(r'\basm\s*\(',source)),1)

    @unittest.skipUnless((ROOT/'assets/USA/main.exe').is_file() and
                         (ROOT/'build/USA/main.exe').is_file(), 'retail/build unavailable')
    def test_retail_text_is_unchanged(self):
        offset,size = 0x8008E23C-0x8000F800,0xA0
        retail = (ROOT/'assets/USA/main.exe').read_bytes()[offset:offset+size]
        built = (ROOT/'build/USA/main.exe').read_bytes()[offset:offset+size]
        self.assertEqual(len(retail),size)
        self.assertEqual(built,retail)


if __name__ == '__main__':
    unittest.main()
