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


class Room350AnimationRepeatTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_repeat_and_completion_transitions(self):
        source = host_unit("RoomM350_AreaController", under=ROOM)
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
short g_RoomM350AreaRepeats;
unsigned char g_RoomM350AreaDone, g_RoomM350AreaPlayerInside, g_RoomM350AreaEntered;
int main(void) {
    const int events[] = {-1, 0, 1, 2};
    const int frames[] = {-32768, -1, 0, 2, 3, 20, 21, 22, 23, 255, 32767};
    const int lengths[] = {0, 1, 22, 24, 255};
    const int counts[] = {-32768, -1, 0, 1, 4};
    const int states[] = {0, 1, 2, 4};
    Animation animation, before;
    Animation *slots[3] = {0, 0, &animation};
    Actor actor;
    unsigned char actorState;
    int e, a, s, n, f, l, c;
    g_FieldFxOwner = (FieldFxObjectPrefix *)slots;
    memset(&actor, 0, sizeof(actor));
    actor.state = &actorState;
    for (e = 0; e < 4; ++e) for (a = 0; a < 2; ++a)
    for (s = 0; s < 4; ++s) for (n = 8; n <= 9; ++n)
    for (f = 0; f < 11; ++f) for (l = 0; l < 5; ++l)
    for (c = 0; c < 5; ++c) {
        int event = events[e], expectedState = states[s];
        int expectedCount = counts[c], expectedFlag = 0xA5, result = 0;
        memset(&animation, 0xCC, sizeof(animation));
        animation.actor = a ? &actor : 0;
        animation.animation = n; animation.frameCount = lengths[l];
        animation.frame.halves[0] = 1234; animation.frame.halves[1] = frames[f];
        memcpy(&before, &animation, sizeof(before));
        actorState = states[s]; g_RoomM350AreaRepeats = counts[c];
        g_RoomM350AreaDone = 0xA5; g_RoomM350AreaPlayerInside = 0x5A; g_RoomM350AreaEntered = 0xC3;
        if (event == 0) { expectedCount = 4; expectedFlag = 0; }
        if (event == 1) {
            if (a && expectedState == 1) expectedState = 2;
            if (n == 9) {
                if (counts[c] > 0) {
                    if (frames[f] >= 22) {
                        before.frame.word = 0x30000;
                        --expectedCount;
                    }
                } else if (frames[f] >= lengths[l] - 1) {
                    if (a) expectedState = 4;
                    expectedFlag = result = 1;
                }
            }
        }
        assert(RoomM350_AreaController(event) == result);
        assert(memcmp(&animation, &before, sizeof(animation)) == 0);
        assert(actorState == expectedState && g_RoomM350AreaRepeats == expectedCount);
        assert(g_RoomM350AreaDone == expectedFlag);
        assert(g_RoomM350AreaPlayerInside == (event == 0 ? 0 : 0x5A));
        assert(g_RoomM350AreaEntered == (event == 0 ? 0 : 0xC3));
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-animation-repeat"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
