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


class Room350EffectRadialTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_burst_allocation_random_vectors_and_configuration(self):
        source = host_unit("RoomM350_DropBurstEmitter", under=ROOM)
        harness = '#include <assert.h>\n#include <stddef.h>\n#include <string.h>\n' + source + r'''
RenderEffectParameters g_FieldFxSpriteParams;
#define D_800F336A g_FieldFxSpriteParams.parameter02
#define D_800F336C g_FieldFxSpriteParams.palette
#define D_800F336E g_FieldFxSpriteParams.parameter06
#define D_800F3370 g_FieldFxSpriteParams.tpage
#define D_800F3372 g_FieldFxSpriteParams.parameter0A
#define D_800F3374 g_FieldFxSpriteParams.depth
#define D_800F3376 g_FieldFxSpriteParams.extent_x
#define D_800F3378 g_FieldFxSpriteParams.extent_y
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char g_RoomM350DropDone, g_RoomM350DropBurstCount;
unsigned short g_FieldFxTpageSlot2, g_FieldFxTpageTable[65536];
static RoomM350Emitter emitters[2];
static int pools[2], allocations, draws, rotations, randomCalls, setups;
static int failAt, requestAfterAllocation, setupResult, randomValues[8];
static Vector effects[2];
_Static_assert(offsetof(RoomM350Emitter, pool) == 8, "pool offset");
_Static_assert(sizeof(Matrix) == 32, "matrix size");
_Static_assert(offsetof(Matrix, position) == 20, "translation offset");
_Static_assert(sizeof(Vector) == 8, "vector size");
int RoomM350_DropBurstCallback(int event, short *object) { return 0; }
int FieldFx_InitCallbackList(char *pool, int a, int b,
                  FieldFxCallbackListCallback callback) {
    assert(setups++ == 0 && allocations == 0 && pool == &pools[0]);
    assert(a == 8 && b == 16 && (void *)callback == (void *)RoomM350_DropBurstCallback);
    return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *pool) {
    assert(setups == 0 && allocations < 2);
    assert(pool == &pools[allocations & 1]);
    /* Mutations probe reload order, not the real allocator's side effects. */
    g_FieldFxActiveTaskSlot = (FieldFxTaskSlot *)&emitters[1];
    g_RoomM350DropBurstCount = requestAfterAllocation;
    g_RoomM350DropDone = 255;
    allocations++;
    if (allocations == failAt) return 0;
    return (short *)&effects[allocations - 1];
}
int Inv_ScrambleGrid(void) {
    assert(randomCalls < 8);
    return randomValues[randomCalls++];
}
GteMatrix *RotMatrixYXZ(GteShortVector *vector, GteMatrix *matrix) {
    assert(rotations == draws && randomCalls == draws * 4 + 2);
    assert(vector->x == (short)(randomValues[draws * 4] * 16));
    assert(vector->y == (short)(randomValues[draws * 4 + 1] * 16));
    assert(vector->z == 0);
    /* The unused pad is not initialized; it must survive the second phase. */
    vector->pad = 1234;
    memset(matrix, 0x5A, sizeof(*matrix));
    rotations++;
    return matrix;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *matrix, const GteShortVector *vector,
                              GteShortVector *effect) {
    unsigned char expectedMatrix[32];
    int combined = randomValues[draws * 4 + 2] * 256 | randomValues[draws * 4 + 3];
    memset(expectedMatrix, 0x5A, sizeof(expectedMatrix));
    assert(memcmp(matrix, expectedMatrix, sizeof(expectedMatrix)) == 0);
    assert(rotations == draws + 1 && randomCalls == (draws + 1) * 4);
    assert(vector->x == 0 && vector->y == 0 && vector->z == combined % 64 + 256);
    assert(vector->pad == 1234 && effect == &effects[draws]);
    effect->x = 100 + draws; effect->y = -200; effect->z = vector->z;
    draws++;
    return effect;
}
static void reset(void) {
    setups = allocations = draws = rotations = randomCalls = 0;
    g_FieldFxActiveTaskSlot = (FieldFxTaskSlot *)&emitters[0];
}
int main(void) {
    const int values[] = {0, 1, 63, 64, 127, 128, 254, 255};
    const int flags[] = {0, 1, 128, 255};
    const unsigned short indices[] = {0, 1, 255, 32768, 65535};
    int seed, stop, request, failure, mutation, i, event;
    emitters[0].pool = &pools[0]; emitters[1].pool = &pools[1];
    for (setupResult = -2; setupResult <= 2; ++setupResult) {
        reset();
        assert(RoomM350_DropBurstEmitter(0) == setupResult && setups == 1);
    }
    for (seed = 0; seed < 64; ++seed)
    for (stop = 0; stop < 4; ++stop) for (request = 0; request < 4; ++request)
    for (failure = 0; failure < 3; ++failure) for (mutation = 0; mutation < 4; ++mutation) {
        int active = !flags[stop] && flags[request];
        int expectedDraws = !active ? 0 : failure == 1 ? 0 : failure == 2 ? 1 : 2;
        reset(); failAt = failure; requestAfterAllocation = flags[mutation];
        g_RoomM350DropDone = flags[stop]; g_RoomM350DropBurstCount = flags[request];
        for (i = 0; i < 8; ++i) randomValues[i] = values[(seed + i * (seed / 8 + 1)) % 8];
        assert(RoomM350_DropBurstEmitter(1) == (flags[stop] ? 2 : 0));
        assert(allocations == (active ? failure == 1 ? 1 : 2 : 0));
        assert(draws == expectedDraws && rotations == draws && randomCalls == draws * 4);
        assert(g_RoomM350DropBurstCount == (active ? (unsigned char)(requestAfterAllocation - 1) : flags[request]));
    }
    for (i = 0; i < 5; ++i) {
        reset(); g_FieldFxTpageSlot2 = indices[i]; g_FieldFxTpageTable[indices[i]] = 65535 - i;
        assert(RoomM350_DropBurstEmitter(2) == 0 && allocations == 0 && setups == 0);
        assert(D_800F3372 == 0 && g_FieldFxSpriteParams.parameter00 == 16 && D_800F336A == 1);
        assert(D_800F3376 == 16 && D_800F3378 == 16 && D_800F336C == 2);
        assert(D_800F336E == 0 && D_800F3374 == 0 && D_800F3370 == 65535 - i);
    }
    for (event = -3; event <= 4; ++event) {
        if (event >= 0 && event <= 2) continue;
        reset(); g_RoomM350DropBurstCount = 255;
        assert(RoomM350_DropBurstEmitter(event) == 0 && g_RoomM350DropBurstCount == 255);
        assert(allocations == 0 && draws == 0 && setups == 0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-radial"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
