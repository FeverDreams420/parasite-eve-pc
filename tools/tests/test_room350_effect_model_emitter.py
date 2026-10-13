import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROOM = 'src/overlays/room_m350_cruiser'
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, host_unit, psx_compile  # noqa: E402
FUNCTION = "RoomM350_BodyRingEmitter"


class Room350EffectModelEmitterTests(unittest.TestCase):
    @unittest.skipUnless(
        (ROOT / "tools/old-gcc/cc1").is_file() and shutil.which("mipsel-none-elf-as"),
        "native PSX compiler/assembler unavailable",
    )
    def test_target_structure_offsets(self):
        unit = find_definition(FUNCTION, ROOM)
        checks = r'''
#define OFFSET(type, field) ((unsigned long)&((type *)0)->field)
typedef char particle_size[sizeof(RoomM350SineParticle) == 24 ? 1 : -1];
typedef char particle_controls[OFFSET(RoomM350SineParticle, controls) == 8 ? 1 : -1];
typedef char particle_amplitude[OFFSET(RoomM350SineParticle, amplitude) == 20 ? 1 : -1];
typedef char state_size[sizeof(EmissionState) == 4 ? 1 : -1];
typedef char model_count[OFFSET(RenderObjectHeader, part_count) == 2 ? 1 : -1];
typedef char transform_size[sizeof(RenderMatrix) == 32 ? 1 : -1];
typedef char transform_position[OFFSET(RenderMatrix, translation) == 20 ? 1 : -1];
typedef char instance_model[OFFSET(FieldActor, render_object.header) == 0x1B4 ? 1 : -1];
typedef char instance_transforms[OFFSET(FieldActor, render_object.matrices) == 0x238 ? 1 : -1];
typedef char actor_instance[OFFSET(RoomM350EffectActor, instance) == 8 ? 1 : -1];
typedef char emitter_pool[OFFSET(RoomM350EffectEmitter, pool) == 8 ? 1 : -1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(unit.read_text() + checks,
                                 pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_transform_selection_timer_and_initialization(self):
        source = host_unit(FUNCTION, under=ROOM)
        harness = '#include <assert.h>\n#include <stddef.h>\n#include <stdint.h>\n#include <string.h>\n' + source + r'''
typedef RoomM350SineParticle Particle;
typedef RenderMatrix Transform;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
FieldFxObjectPrefix *g_FieldFxOwner;
unsigned char g_RoomM350BodyDone;
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
static RoomM350EffectEmitter emitter;
static RoomM350EffectActor actors[2];
static FieldActor instances[2];
static RenderObjectHeader models[2];
static Transform transforms[2][256];
static EmissionState state;
static Particle effect, expected;
static int pool, setups, allocations, tableRandoms, controlRandoms, failAllocation, setupResult;
static int tableRandom, controlRandom, countAfterRandom;
_Static_assert(sizeof(Particle) == 24 && sizeof(EmissionState) == 4, "payload sizes");
_Static_assert(sizeof(Transform) == 32 && offsetof(Transform, translation) == 20, "transform layout");
/* FieldActor contains 64-bit pointers on this host. Its PSX offsets are tested separately. */
static int signedHalf(uint32_t value) {
    value &= 65535;
    return value < 32768 ? (int)value : (int)value - 65536;
}
static int selectedIndex(void) {
    int product = (tableRandom & 255) * ((int)models[1].part_count - 1);
    int quotient = product >= 0 ? product / 256 : -((-product) / 256);
    return quotient + 1;
}
int RoomM350_BodyRingCallback(int event, Particle *particle) { return 0; }
int FieldFx_InitCallbackList(char *p, int stride, int count,
                  FieldFxCallbackListCallback callback) {
    assert(setups++ == 0 && allocations == 0 && p == &pool);
    assert(stride == 24 && count == 8 && (void *)callback == (void *)RoomM350_BodyRingCallback);
    assert(state.count == 0 && state.delay == 0);
    return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(allocations++ == 0 && setups == 0 && p == &pool && state.delay == -1);
    return failAllocation ? 0 : &effect;
}
int Inv_ScrambleGrid(void) {
    assert(tableRandoms++ == 0 && allocations == 1 && controlRandoms == 0);
    /* Probe post-helper reloads, without asserting these are the real helper's effects. */
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[1];
    state.count = countAfterRandom;
    return tableRandom;
}
int rand(void) {
    int index = selectedIndex();
    assert(controlRandoms++ == 0 && tableRandoms == 1);
    assert(index >= 1 && index <= 255);
    assert(effect.position.x == signedHalf(transforms[1][index].translation[0]));
    assert(effect.position.y == signedHalf(transforms[1][index].translation[1]));
    assert(effect.position.z == signedHalf(transforms[1][index].translation[2]));
    assert(effect.brightness == 128 && effect.amplitude == signedHalf((countAfterRandom + 1) * 2048));
    assert(state.count == signedHalf((uint32_t)countAfterRandom + 1));
    assert(state.delay == (tableRandom & 7) + 12);
    state.count = -123; state.delay = -456; g_RoomM350BodyDone = 165;
    return controlRandom;
}
static void reset(void) {
    setups = allocations = tableRandoms = controlRandoms = 0;
    g_FieldFxActiveTaskSlot = (FieldFxTaskSlot *)&emitter;
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[0]; g_RoomM350BodyDone = 90;
    state.count = 0; state.delay = 0;
    memset(&effect, 0x5A, sizeof(effect));
}
static void verifySpawn(void) {
    int index = selectedIndex();
    int high = ((uint32_t)controlRandom >> 8) & 255;
    memset(&expected, 0x5A, sizeof(expected));
    expected.position.x = signedHalf(transforms[1][index].translation[0]);
    expected.position.y = signedHalf(transforms[1][index].translation[1]);
    expected.position.z = signedHalf(transforms[1][index].translation[2]);
    expected.brightness = 128;
    expected.amplitude = signedHalf((countAfterRandom + 1) * 2048);
    expected.controls[0] = (controlRandom & 255) + 512;
    expected.controls[1] = high + ((controlRandom & 2) ? 768 : -1024);
    expected.controls[2] = 0; expected.controls[3] = 0;
    assert(memcmp(&effect, &expected, sizeof(effect)) == 0);
    assert(allocations == 1 && tableRandoms == 1 && controlRandoms == 1);
    assert(state.count == -123 && state.delay == -456 && g_RoomM350BodyDone == 165);
}
int main(void) {
    const int counts[] = {-32768, -1, 0, 1, 23, 24, 32767};
    const int delays[] = {-32768, -1, 0, 1, 32767};
    const int controls[] = {0, 1, 2, 3, 255, 256, 32767, INT32_MIN, -1};
    const unsigned short indices[] = {0, 1, 255, 32768, 65535};
    int i, j, k, m, failure, value, event;
    emitter.pool = &pool;
    for (i = 0; i < 2; ++i) {
        actors[i].instance = (RoomM350EffectInstance *)&instances[i];
        instances[i].render_object.header = &models[i];
        instances[i].render_object.matrices = transforms[i]; models[i].part_count = 255;
        for (j = 0; j < 256; ++j) for (k = 0; k < 3; ++k)
            transforms[i][j].translation[k] = (i ? -1 : 1) * (j * 1001 + k * 45678 + 12345);
    }
    for (setupResult = -2; setupResult <= 2; ++setupResult) {
        reset(); state.count = 123; state.delay = -123;
        assert(RoomM350_BodyRingEmitter(0, &state) == setupResult && setups == 1 && allocations == 0);
    }
    /* Every delay bit pattern: a failed emission leaves -1, not a new delay. */
    for (value = -32768; value <= 32767; ++value) {
        reset(); state.delay = value; failAllocation = 1; expected = effect;
        assert(RoomM350_BodyRingEmitter(1, &state) == 0);
        assert(state.delay == signedHalf((uint32_t)value - 1) && state.count == 0);
        assert(allocations == (value == 0) && tableRandoms == 0 && controlRandoms == 0);
        assert(memcmp(&effect, &expected, sizeof(effect)) == 0 && g_RoomM350BodyDone == 90);
    }
    for (i = 0; i < 7; ++i) for (j = 0; j < 5; ++j) for (failure = 0; failure < 2; ++failure) {
        int active = counts[i] < 24 && delays[j] == 0;
        reset(); state.count = counts[i]; state.delay = delays[j]; failAllocation = failure;
        countAfterRandom = counts[i]; tableRandom = 255; controlRandom = -1; expected = effect;
        assert(RoomM350_BodyRingEmitter(1, &state) == (counts[i] >= 24 ? 2 : 0));
        assert(allocations == active);
        if (active && !failure) verifySpawn();
        else {
            assert(state.count == counts[i]);
            assert(state.delay == (counts[i] >= 24 ? delays[j] : signedHalf((uint32_t)delays[j] - 1)));
            assert(g_RoomM350BodyDone == (counts[i] >= 24 ? 1 : 90));
            assert(memcmp(&effect, &expected, sizeof(effect)) == 0);
        }
    }
    /* Exhaust every byte/count pair, including count0's signed division case. */
    for (i = 0; i < 256; ++i) for (j = 0; j < 256; ++j) for (k = 0; k < 9; ++k) {
        reset(); failAllocation = 0; models[1].part_count = i;
        tableRandom = j | (k & 1 ? 0x7FFFFF00 : 0);
        controlRandom = controls[k]; countAfterRandom = counts[k % 7];
        assert(RoomM350_BodyRingEmitter(1, &state) == 0); verifySpawn();
    }
    for (m = 0; m < 5; ++m) {
        reset(); state.count = -7; state.delay = 123;
        g_FieldFxTpageSlot11 = indices[m]; g_FieldFxTpageTable[indices[m]] = 65535 - m;
        assert(RoomM350_BodyRingEmitter(2, &state) == 0 && allocations == 0 && setups == 0);
        assert(state.count == -7 && state.delay == 123 && g_RoomM350BodyDone == 90);
        assert(UNIT == 64 && D_800F336A == 4 && D_800F3376 == 64 && D_800F3378 == 64);
        assert(D_800F336C == 3 && D_800F336E == 1 && D_800F3372 == 5 && D_800F3374 == 0);
        assert(D_800F3370 == 65535 - m);
    }
    for (event = -3; event <= 4; ++event) {
        if (event >= 0 && event <= 2) continue;
        reset(); state.count = -7; state.delay = 123;
        assert(RoomM350_BodyRingEmitter(event, &state) == 0 && allocations == 0 && setups == 0);
        assert(state.count == -7 && state.delay == 123 && g_RoomM350BodyDone == 90);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-model-emitter"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fwrapv", "-fno-strict-aliasing",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
