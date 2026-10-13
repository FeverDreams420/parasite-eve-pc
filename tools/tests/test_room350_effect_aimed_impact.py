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


class Room350EffectAimedImpactTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_spawn_angles_layout_and_helper_capture_order(self):
        source = host_unit("RoomM350_DropEmitter", under=ROOM)
        harness = '#include <assert.h>\n#include <stddef.h>\n#include <stdint.h>\n#include <string.h>\n' + source + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
FieldFxObjectPrefix *g_FieldFxOwner;
Player *g_PlayerEntity;
unsigned char g_RoomM350DropDone, D_8019A855;
short g_RoomM350DropLungeTimer;
static RoomM350Emitter emitter;
static Actor actors[3];
static Instance instances[3];
static FieldActorState owners[3];
/* Each instance's render matrices; the impact starts at matrix 7. */
static Transform transforms[3][8];
static Player players[2];
#define ORIGIN(i) transforms[i][7].translation
#define PLAYER_POSITION(i) players[i].render_object.model_matrix.translation
static Particle particle;
static int pool, setups, allocations, roots, angles, randoms, rotations, applications, sounds;
static int setupResult, failAllocation, rng[2], angleResult;
static unsigned short yawAfterRandom;
static Vector *rotationInput;
static Matrix *rotationMatrix;
_Static_assert(sizeof(Particle) == 16, "particle size");
_Static_assert(offsetof(Particle, x) == 8, "particle position");
_Static_assert(offsetof(Particle, stopped) == 14, "particle stop flag");
_Static_assert(sizeof(Matrix) == 32, "matrix size");
_Static_assert(offsetof(Matrix, translation) == 20, "matrix translation");
_Static_assert(offsetof(RoomM350Emitter, pool) == 8, "emitter pool");
_Static_assert(offsetof(Actor, instance) == 8, "actor instance");
/* FieldActor holds pointers, so its PSX offsets are asserted by the MIPS
 * build (pe1/field_actor.h), not on this 64-bit host. */
static int signedByte(int x) { return x < 128 ? x : x - 256; }
static int signedWord(uint32_t x) {
    return x <= INT32_MAX ? (int)x : (int)((int64_t)x - 4294967296LL);
}
static short signedHalf(uint32_t x) {
    x &= 65535;
    return x < 32768 ? (short)x : (short)((int)x - 65536);
}
int RoomM350_DropCallback(int event, Particle *effect) { return 0; }
int FieldFx_InitCallbackList(char *p, int size, int count,
                  FieldFxCallbackListCallback callback) {
    assert(setups++ == 0 && allocations == 0 && p == &pool);
    assert(size == 16 && count == 4 && (void *)callback == (void *)RoomM350_DropCallback);
    return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(allocations++ == 0 && setups == 0 && p == &pool);
    /* Mutations probe captured versus reloaded values, not real helper side effects. */
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[1];
    g_PlayerEntity = &players[1];
    D_8019A855 = 165;
    g_RoomM350DropDone = 255;
    return failAllocation ? 0 : &particle;
}
int Math_IntSqrt(int value) {
    uint32_t dx = (uint32_t)ORIGIN(1)[0] - (uint32_t)PLAYER_POSITION(1)[0];
    uint32_t dz = (uint32_t)ORIGIN(1)[2] - (uint32_t)PLAYER_POSITION(1)[2];
    uint32_t sum = (uint32_t)((uint64_t)dx * dx) + (uint32_t)((uint64_t)dz * dz);
    assert(roots++ == 0 && allocations == 1 && value == signedWord(sum));
    assert(particle.x == signedHalf(ORIGIN(1)[0]));
    assert(particle.y == signedHalf(ORIGIN(1)[1]));
    assert(particle.z == signedHalf(ORIGIN(1)[2]));
    assert(particle.stopped == 0 && particle.pad == 0x5A);
    assert(particle.velocity.pad == 0x5A5A);
    return 12345;
}
int ratan2(int y, int horizontal) {
    uint32_t dy = (uint32_t)ORIGIN(1)[1] - (uint32_t)PLAYER_POSITION(1)[1];
    assert(angles++ == 0 && roots == 1 && y == signedWord(dy) && horizontal == 12345);
    return angleResult;
}
int Inv_ScrambleGrid(void) {
    assert(angles == 1 && randoms < 2);
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[2];
    instances[1].rot_y = yawAfterRandom;
    return rng[randoms++];
}
GteMatrix *RotMatrixYXZ(GteShortVector *v, GteMatrix *m) {
    unsigned pitch = ((uint32_t)angleResult + signedByte(rng[0]) / 2) & 4095;
    unsigned yaw = yawAfterRandom + signedByte(rng[1]) + 2048;
    if (pitch < 0xD00) pitch = 0xD00;
    assert(rotations++ == 0 && randoms == 2);
    assert(v->x == pitch && v->y == signedHalf(yaw) && v->z == 0);
    rotationInput = v; rotationMatrix = (Matrix *)m;
    /* The unused pad must survive reuse of the same vector in the next phase. */
    v->pad = 1234;
    memset(m, 0x3C, sizeof(*m));
    return m;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *m, const GteShortVector *v,
                              GteShortVector *out) {
    unsigned char expected[32];
    memset(expected, 0x3C, sizeof(expected));
    assert(applications++ == 0 && rotations == 1 && (const void *)m == rotationMatrix && v == rotationInput);
    assert(memcmp(m, expected, sizeof(expected)) == 0);
    assert(v->x == 0 && v->y == 0 && v->z == 128 && v->pad == 1234);
    assert(out == &particle.velocity && D_8019A855 == 165);
    out->x = -3; out->y = 4; out->z = 5;
    particle.x = -32768; particle.y = 32767; particle.z = -123;
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[2];
    return out;
}
int Asset_Find08w(int id, int mode, int x, int y, int z) {
    assert(sounds++ == 0 && applications == 1 && id == 0x60A);
    assert(mode == owners[2].progress && x == -32768 && y == 32767 && z == -123);
    assert(D_8019A855 == 0 && g_RoomM350DropLungeTimer == -333);
    D_8019A855 = 77;
    g_RoomM350DropLungeTimer = -1;
    return -123;
}
static void reset(void) {
    setups = allocations = roots = angles = randoms = rotations = applications = sounds = 0;
    g_FieldFxActiveTaskSlot = (FieldFxTaskSlot *)&emitter;
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[0]; g_PlayerEntity = &players[0];
    g_RoomM350DropDone = 0; D_8019A855 = 1; g_RoomM350DropLungeTimer = -333;
    instances[1].rot_y = 17;
    memset(&particle, 0x5A, sizeof(particle));
}
int main(void) {
    const int flags[] = {0, 1, 128, 255};
    const int pitches[] = {0, 1, 0xCFF, 0xD00, 0xFFF, -1, INT32_MIN, INT32_MAX};
    const int coordinates[] = {0, 1, -1, 32767, -32768, 100000, INT32_MIN, INT32_MAX};
    int i, j, k, stop, request, failure, event;
    emitter.pool = &pool;
    for (i = 0; i < 3; ++i) {
        actors[i].instance = (struct RoomM350Instance *)&instances[i];
        instances[i].state = &owners[i];
        instances[i].render_object.matrices = transforms[i];
        owners[i].progress = -1000 - i;
        instances[i].rot_y = 555;
    }
    for (setupResult = -2; setupResult <= 2; ++setupResult) {
        reset(); assert(RoomM350_DropEmitter(0) == setupResult && setups == 1 && allocations == 0);
    }
    for (stop = 0; stop < 4; ++stop) for (request = 0; request < 4; ++request)
    for (failure = 0; failure < 2; ++failure) {
        int active = !flags[stop] && flags[request];
        int spawned = active && !failure;
        reset(); g_RoomM350DropDone = flags[stop]; D_8019A855 = flags[request]; failAllocation = failure;
        assert(RoomM350_DropEmitter(1) == (flags[stop] ? 2 : 0));
        assert(allocations == active && sounds == spawned && roots == spawned);
        assert(D_8019A855 == (spawned ? 77 : active ? 165 : flags[request]));
        assert(g_RoomM350DropLungeTimer == (spawned ? 8 : -333));
        if (!spawned) {
            unsigned char untouched[16]; memset(untouched, 0x5A, sizeof(untouched));
            assert(memcmp(&particle, untouched, sizeof(particle)) == 0);
        }
    }
    /* Exhaust every pair of RNG bytes across wrap/clamp and coordinate boundaries. */
    for (k = 0; k < 8; ++k) for (i = 0; i < 256; ++i) for (j = 0; j < 256; ++j) {
        int axis;
        reset(); failAllocation = 0; rng[0] = i; rng[1] = j; angleResult = pitches[k];
        yawAfterRandom = (unsigned short)(k * 8191 + j);
        for (axis = 0; axis < 3; ++axis) {
            ORIGIN(1)[axis] = coordinates[(k + axis) % 8];
            PLAYER_POSITION(1)[axis] = coordinates[(k + axis + 3) % 8];
        }
        assert(RoomM350_DropEmitter(1) == 0);
        assert(allocations == 1 && randoms == 2 && roots == 1 && angles == 1);
        assert(rotations == 1 && applications == 1 && sounds == 1);
        assert(D_8019A855 == 77 && g_RoomM350DropLungeTimer == 8 && particle.stopped == 0);
        assert(particle.velocity.x == -3 && particle.velocity.y == 4 && particle.velocity.z == 5);
        assert(particle.velocity.pad == 0x5A5A && particle.pad == 0x5A);
    }
    for (event = -3; event <= 4; ++event) {
        if (event == 0 || event == 1) continue;
        reset(); assert(RoomM350_DropEmitter(event) == 0 && setups == 0 && allocations == 0);
        assert(D_8019A855 == 1 && g_RoomM350DropLungeTimer == -333);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-aimed-impact"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fwrapv", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
