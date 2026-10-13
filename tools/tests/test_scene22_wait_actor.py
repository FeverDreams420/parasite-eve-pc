import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

FUNCTION = 'SceneE22_PlayerGate'


class SceneWaitActor(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[OFF(WaitState,complete)==0x14 ? 1:-1];
typedef char b[OFF(WaitState,signaled)==0x16 ? 1:-1];
typedef char c[OFF(FieldActorState,flags)==0x4C ? 1:-1];
typedef char d[OFF(struct FieldActor,state)==0 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION) + r'''
WaitState state;
FieldActorState owner;
struct FieldActor actor = { &owner };
struct FieldFxTaskContext *g_FieldFxTaskContext = (struct FieldFxTaskContext *)&state;
struct FieldActor *g_PlayerEntity = &actor;
int g_FieldFxCurrentAge;
int main(void) {
    int modes[] = {INT_MIN, -1, 2, 3, INT_MAX};
    int timers[] = {INT_MIN, -1, 0, 99, 100, INT_MAX};
    unsigned int i;
    owner.flags = 0xFFFFFFFFu;
    state.complete = 7; state.signaled = 8;
    assert(SceneE22_PlayerGate(0) == 0);
    assert(!state.complete && !state.signaled);
    assert(owner.flags == 0xEFFFFFFFu);
    for (i = 0; i < sizeof(modes)/sizeof(modes[0]); ++i) {
        state.complete = 7; state.signaled = 8;
        assert(SceneE22_PlayerGate(modes[i]) == 0);
        assert(state.complete == 7 && state.signaled == 8);
        assert(owner.flags == 0xEFFFFFFFu);
    }
    for (i = 0; i < sizeof(timers)/sizeof(timers[0]); ++i) {
        g_FieldFxCurrentAge = timers[i]; owner.flags = 0x123;
        state.complete = 7; state.signaled = 8;
        assert(SceneE22_PlayerGate(1) == (timers[i] >= 100));
        assert(state.complete == (timers[i] >= 100 ? 1 : 7));
        assert(state.signaled == (timers[i] >= 100 ? 0 : 8));
        assert(owner.flags == 0x123);
        owner.flags |= 0x10000000u;
        assert(SceneE22_PlayerGate(1) == 1);
        assert(state.complete == 1 && state.signaled == 1);
        assert(owner.flags == 0x10000123u);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=c99', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
