"""Exact retail code and frame-presentation callback sequencing."""
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import function_closure, linked_function_bytes  # noqa: E402


class DrawPresentFrameTests(unittest.TestCase):
    def test_source_has_no_assembly(self):
        source = function_closure('Draw_PresentFrame')
        source = re.sub(r'/\*.*?\*/|//[^\n]*','',source,flags=re.S)
        self.assertNotRegex(source,r'\b(?:asm|__asm__|INCLUDE_ASM|CC_POSTPASS)\b')

    @unittest.skipUnless(shutil.which('mipsel-none-elf-ld') and
                         (ROOT/'build/USA/main.elf').is_file() and
                         (ROOT/'assets/USA/main.exe').is_file(), 'retail/toolchain unavailable')
    def test_retail_bytes_and_callback_model(self):
        try:
            from unicorn import Uc, UC_ARCH_MIPS, UC_MODE_MIPS32, UC_MODE_LITTLE_ENDIAN, UC_HOOK_CODE
            from unicorn import mips_const as R
        except ImportError:
            self.skipTest('unicorn unavailable')
        symbols = {}
        for line in subprocess.check_output(['mipsel-none-elf-nm',str(ROOT/'build/USA/main.elf')],text=True).splitlines():
            fields = line.split()
            if len(fields)==3: symbols[fields[2]] = int(fields[0],16)
        entry, stop, stack = 0x8005E788,0x80010000,0x801F0000
        offset = entry-0x8000F800
        retail = (ROOT/'assets/USA/main.exe').read_bytes()[offset:offset+200]
        callbacks = ('VSync','DrawSync','ResetGraph','PutDrawEnv','PutDispEnv','LoadImage','DrawOTag')
        data = ('g_DrawPresentEnabled','g_DrawPresentImage','g_DrawBufferIndex','g_DrawCurrentFrameBuffer','g_DrawOrderingTableBase','_gp')
        with tempfile.TemporaryDirectory() as directory:
            compiled = linked_function_bytes(
                'Draw_PresentFrame', entry, 200,
                ''.join(f'{name} = 0x{symbols[name]:X};\n' for name in callbacks+data),
                Path(directory))
        self.assertEqual(compiled,retail)
        for mode in (-2147483648,-2,-1,0,1,2,8,2147483647):
            for enabled in (0,1,-1):
                for image in (0,0x80130000):
                    for index in (-1,0,1):
                        for mutate in (False,True):
                            draw_env = 0x80111000 if mutate else 0x80110000
                            disp_env = 0x8011205C if mutate else 0x8011005C
                            final_image = (0 if image else 0x80130040) if mutate else image
                            final_index = int(index==0) if mutate else index
                            ot = 0x80141000 if mutate and final_image else 0x80140000
                            expected = []
                            if enabled:
                                expected = [('VSync',1),('DrawSync',0),('VSync',0 if mode==1 else mode&0xFFFFFFFF),
                                            ('ResetGraph',1),('PutDrawEnv',draw_env),('PutDispEnv',disp_env)]
                                if final_image: expected.append(('LoadImage',(0,235 if final_index else 11,320,204),final_image))
                                expected.append(('DrawOTag',ot+0x3FFC))
                            for body in (retail,compiled):
                                m = Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN)
                                m.mem_map(0,0x200000)
                                def put(address,data): m.mem_write(address&0x1FFFFFFF,bytes(data))
                                def word(name,value): put(symbols[name],struct.pack('<I',value&0xFFFFFFFF))
                                put(entry,body)
                                for name in callbacks: put(symbols[name],struct.pack('<III',0,0x03E00008,0))
                                for name,value in (('g_DrawPresentEnabled',enabled),('g_DrawPresentImage',image),('g_DrawBufferIndex',index),('g_DrawCurrentFrameBuffer',0x80110000),('g_DrawOrderingTableBase',0x80140000)):
                                    word(name,value)
                                m.reg_write(R.UC_MIPS_REG_A0,mode&0xFFFFFFFF)
                                m.reg_write(R.UC_MIPS_REG_RA,stop)
                                m.reg_write(R.UC_MIPS_REG_SP,stack)
                                m.reg_write(R.UC_MIPS_REG_GP,symbols['_gp'])
                                for i in range(8): m.reg_write(getattr(R,f'UC_MIPS_REG_S{i}'),0xABCD0000+i)
                                events = []
                                external = {symbols[name]+4:name for name in callbacks}
                                def hook(machine,address,size,data):
                                    name = external.get(address)
                                    if name is None: return
                                    a0,a1 = (machine.reg_read(getattr(R,f'UC_MIPS_REG_A{i}')) for i in range(2))
                                    if name=='LoadImage':
                                        rect = struct.unpack('<hhhh',machine.mem_read(a0&0x1FFFFFFF,8))
                                        events.append((name,rect,a1))
                                    else: events.append((name,a0))
                                    if mutate:
                                        if name=='VSync': word('g_DrawPresentEnabled',0)
                                        elif name=='ResetGraph': word('g_DrawCurrentFrameBuffer',draw_env)
                                        elif name=='PutDrawEnv': word('g_DrawCurrentFrameBuffer',disp_env-0x5C)
                                        elif name=='PutDispEnv':
                                            word('g_DrawPresentImage',final_image)
                                            word('g_DrawBufferIndex',final_index)
                                        elif name=='LoadImage': word('g_DrawOrderingTableBase',ot)
                                    for reg in ('V0','V1','A0','A1','A2','A3','T0','T1','T2','T3','T4','T5','T6','T7','T8','T9'):
                                        machine.reg_write(getattr(R,'UC_MIPS_REG_'+reg),0xDEADCAFE)
                                m.hook_add(UC_HOOK_CODE,hook)
                                m.emu_start(entry,stop,count=1000)
                                self.assertEqual(events,expected,(mode,enabled,image,index,mutate))
                                self.assertEqual(m.reg_read(R.UC_MIPS_REG_PC),stop)
                                self.assertEqual(m.reg_read(R.UC_MIPS_REG_SP),stack)
                                self.assertEqual(m.reg_read(R.UC_MIPS_REG_GP),symbols['_gp'])
                                for i in range(8): self.assertEqual(m.reg_read(getattr(R,f'UC_MIPS_REG_S{i}')),0xABCD0000+i)


if __name__ == '__main__':
    unittest.main()
