"""Regression gates: the key-on/off steps stay free of register constraints."""
from pathlib import Path
import re
import unittest

from tools.tests.csource import find_definition

ROOT = Path(__file__).resolve().parents[2]


class AkaoVoiceKeyConstraintTests(unittest.TestCase):
    def test_redundant_constraints_are_absent(self):
        unit = find_definition('Akao_SetVoiceKeyOn').read_text()
        for name,limit,unpinned,first,last in (
            ('Akao_SetVoiceKeyOn',0,('flags','base_volume','value','next'),
             'void Akao_SetVoiceKeyOn(','void Spu_CopyVoiceToStereoSlot('),
            ('Akao_SetVoiceKeyOff',0,('voice','one','raw_pan','bias','pitch'),
             'void Akao_SetVoiceKeyOff(','void Akao_SetVoiceKeyOn('),
        ):
            with self.subTest(function=name):
                source = unit[unit.index(first):unit.index(last)]
                source = re.sub(r'/\*.*?\*/|//[^\n]*','',source,flags=re.S)
                self.assertNotRegex(source,r'asm\s+volatile')
                self.assertLessEqual(len(re.findall(r'asm\("\$\d+"\)',source)),limit)
                for variable in unpinned:
                    self.assertNotRegex(source,r'\b'+variable+r'\s+asm\s*\(')

    @unittest.skipUnless((ROOT/'assets/USA/main.exe').is_file() and
                         (ROOT/'build/USA/main.exe').is_file(), 'retail/build unavailable')
    def test_entire_functions_match_retail(self):
        # Full functions, including macro-expanded pan and LFO paths. These
        # byte gates are not an independent audio/hardware behavior model.
        retail = (ROOT/'assets/USA/main.exe').read_bytes()
        built = (ROOT/'build/USA/main.exe').read_bytes()
        for entry,size in ((0x80088344,0x63C),(0x80088980,0x4E4)):
            with self.subTest(entry=hex(entry)):
                offset = entry-0x8000F800
                self.assertEqual(len(retail[offset:offset+size]),size)
                self.assertEqual(built[offset:offset+size],retail[offset:offset+size])


if __name__ == '__main__': unittest.main()
