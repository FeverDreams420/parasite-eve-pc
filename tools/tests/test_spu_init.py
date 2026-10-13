import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import host_unit  # noqa: E402


class SpuInitTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_cold_warm_init_and_reverb_reset_order(self):
        # Exercise _SpuInit with a checked SpuStart stub; the complete TU is
        # covered separately by the linked-byte lifecycle regression.
        source = host_unit("_SpuInit")
        harness = source + r'''
#include <assert.h>
unsigned short _spu_voice_centerNote[24];
SpuReverbState g_SpuReverbState;
int _spu_rev_flag, _spu_rev_reserve_wa, D_8009B46C;
u32 _spu_rev_offsetaddr;
int _spu_AllocBlockNum, _spu_AllocLastNum, _spu_trans_mode;
SPU_MALLOC *_spu_memList;
static SPU_MALLOC memListSentinel;
int _spu_transMode, D_8009B388, D_8009B3B4, D_8009B3B0, D_8009B3E8;
static int phase, expectedMode;
static int *late[] = {&_spu_AllocBlockNum, &_spu_AllocLastNum, &_spu_trans_mode,
    &_spu_transMode, &D_8009B388, &D_8009B3B4, &D_8009B3B0, &D_8009B3E8};
int ResetCallback(void) { assert(phase == 0); phase = 1; return 0; }
int _spu_init(int mode) { assert(phase == 1 && mode == expectedMode); phase = 2; return 0; }
void SpuStart(void) {
    int i;
    assert(phase == 2);
    for (i = 0; i < 24; ++i)
        assert(_spu_voice_centerNote[i] == (expectedMode ? 0x100 + i : 0xC000));
    assert(_spu_rev_flag == 9 && g_SpuReverbState.mode == 9);
    phase = 3;
}
void _spu_FsetRXX(u32 reg, u32 area, u32 flag) {
    int i;
    assert(phase == 3 && reg == 0xD1 && area == 0xF800 && flag == 0);
    assert(_spu_rev_flag == 0 && _spu_rev_reserve_wa == 0 &&
           _spu_rev_offsetaddr == area);
    assert(g_SpuReverbState.mode == 0 && g_SpuReverbState.depth.left == 0);
    assert(g_SpuReverbState.depth.right == 0 && g_SpuReverbState.delay == 0);
    assert(g_SpuReverbState.feedback == 0);
    for (i = 0; i < 8; ++i) assert(*late[i] == 9);
    assert(_spu_memList == &memListSentinel);
    phase = 4;
}
int main(void) {
    int mode, i;
    for (mode = -1; mode <= 2; ++mode) {
        expectedMode = mode; phase = 0;
        for (i = 0; i < 24; ++i) _spu_voice_centerNote[i] = 0x100 + i;
        for (i = 0; i < 8; ++i) *late[i] = 9;
        _spu_memList = &memListSentinel;
        _spu_rev_flag = _spu_rev_reserve_wa = _spu_rev_offsetaddr = 9;
        D_8009B46C = 0xF800;
        g_SpuReverbState.mode = g_SpuReverbState.delay = g_SpuReverbState.feedback = 9;
        g_SpuReverbState.depth.left = g_SpuReverbState.depth.right = 9;
        _SpuInit(mode);
        assert(phase == 4 && D_8009B46C == 0xF800);
        for (i = 0; i < 8; ++i) assert(*late[i] == 0);
        assert(_spu_memList == 0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "spu-init-test"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
