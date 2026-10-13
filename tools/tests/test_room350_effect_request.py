import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROOM = 'src/overlays/room_m350_cruiser'
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import host_unit  # noqa: E402


class Room350EffectRequestTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_request_lifecycle_and_render_configuration(self):
        # The fall callback is reduced to a prototype; the test stubs it.
        source = host_unit("RoomM350_DropHitEmitter", under=ROOM)
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char g_RoomM350DropDone, D_8019A857;
short D_8019A864[3];
#define D_8019A866 D_8019A864[1]
#define D_8019A868 D_8019A864[2]
unsigned short g_FieldFxTpageSlots[12], g_FieldFxTpageTable[65536];
#define g_FieldFxTpageSlot11 g_FieldFxTpageSlots[11]
/* The retail halfword names are fields of the sprite parameter block; the
 * signed ones are read through signed views as the old globals were. */
RenderEffectParameters g_FieldFxSpriteParams;
#define SIGNED(field) (*(short *)&g_FieldFxSpriteParams.field)
#define UNIT SIGNED(parameter00)
#define D_800F336A SIGNED(parameter02)
#define D_800F336C g_FieldFxSpriteParams.palette
#define D_800F336E SIGNED(parameter06)
#define D_800F3370 g_FieldFxSpriteParams.tpage
#define D_800F3372 SIGNED(parameter0A)
#define D_800F3374 g_FieldFxSpriteParams.depth
#define D_800F3376 SIGNED(extent_x)
#define D_800F3378 SIGNED(extent_y)
static RoomM350Emitter emitter;
static int poolToken, setups, allocations, failAllocation, setupResult;
static short effect[4], xValue, yValue, zValue;
int RoomM350_DropHitCallback(int event, short *point) { return 0; }
int FieldFx_InitCallbackList(char *pool, int a, int b,
                  FieldFxCallbackListCallback callback) {
    assert(setups++ == 0 && allocations == 0 && pool == &poolToken);
    assert(a == 8 && b == 4 && (void *)callback == (void *)RoomM350_DropHitCallback);
    return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *pool) {
    assert(allocations++ == 0 && setups == 0 && pool == &poolToken);
    assert(D_8019A857 != 0 && g_RoomM350DropDone == 0);
    /* Stub mutations verify that coordinates are fetched after allocation. */
    D_8019A864[0] = xValue; D_8019A866 = yValue; D_8019A868 = zValue;
    return failAllocation ? 0 : effect;
}
int main(void) {
    const unsigned char flags[] = {0, 1, 128, 255};
    const short values[] = {-32768, -33, -1, 0, 32, 32767};
    const unsigned short indices[] = {0, 1, 255, 32768, 65535};
    short before[4];
    int event, stop, request, fail, x, y, z, i, ret;
    emitter.pool = &poolToken; g_FieldFxActiveTaskSlot = (FieldFxTaskSlot *)&emitter;
    for (ret = -2; ret <= 2; ++ret) {
        setups = allocations = 0; setupResult = ret;
        D_8019A857 = 128; g_RoomM350DropDone = 255;
        assert(RoomM350_DropHitEmitter(0) == ret && setups == 1 && allocations == 0);
        assert(D_8019A857 == 128 && g_RoomM350DropDone == 255);
    }
    for (stop = 0; stop < 4; ++stop) for (request = 0; request < 4; ++request)
    for (fail = 0; fail < 2; ++fail)
    for (x = 0; x < 6; ++x) for (y = 0; y < 6; ++y) for (z = 0; z < 6; ++z) {
        int allocate = flags[stop] == 0 && flags[request] != 0;
        int emitted = allocate && !fail;
        g_RoomM350DropDone = flags[stop]; D_8019A857 = flags[request];
        D_8019A864[0] = 123; D_8019A866 = 456; D_8019A868 = 789;
        xValue = values[x]; yValue = values[y]; zValue = values[z];
        memset(effect, 0x5A, sizeof(effect)); memcpy(before, effect, sizeof(effect));
        setups = allocations = 0; failAllocation = fail;
        assert(RoomM350_DropHitEmitter(1) == (flags[stop] ? 2 : 0));
        assert(allocations == allocate && setups == 0 && g_RoomM350DropDone == flags[stop]);
        assert(D_8019A857 == (emitted ? 0 : flags[request]));
        if (emitted) {
            assert(effect[0] == xValue && effect[1] == yValue && effect[2] == zValue && effect[3] == 0);
        } else assert(memcmp(effect, before, sizeof(effect)) == 0);
    }
    for (i = 0; i < 65536; ++i) g_FieldFxTpageTable[i] = (unsigned short)(i * 13 + 17);
    for (i = 0; i < 5; ++i) {
        g_FieldFxTpageSlot11 = indices[i]; setups = allocations = 0;
        D_8019A857 = 128; g_RoomM350DropDone = 255;
        UNIT = D_800F336A = D_800F336E = D_800F3372 = D_800F3374 = -7;
        D_800F336C = D_800F3370 = 65535; D_800F3376 = D_800F3378 = -7;
        assert(RoomM350_DropHitEmitter(2) == 0 && setups == 0 && allocations == 0);
        assert(UNIT == 32 && D_800F336A == 2 && D_800F336C == 3 && D_800F336E == 1);
        assert(D_800F3376 == 32 && D_800F3378 == 32 && D_800F3372 == 0 && D_800F3374 == 0);
        assert(D_800F3370 == g_FieldFxTpageTable[indices[i]]);
        assert(D_8019A857 == 128 && g_RoomM350DropDone == 255);
    }
    for (event = -2; event <= 4; ++event) {
        if (event >= 0 && event <= 2) continue;
        setups = allocations = 0;
        assert(RoomM350_DropHitEmitter(event) == 0 && setups == 0 && allocations == 0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-request"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
