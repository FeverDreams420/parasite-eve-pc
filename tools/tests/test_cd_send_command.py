import pathlib
import re
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import host_unit  # noqa: E402


class CdSendCommandTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_command_state_transitions(self):
        source = host_unit("LIBDS_DSSYS_1_text_368")
        # The sender reaches the command window back through its timeout word
        # (CDROM_EVENT_STATE_FROM_TIMEOUT), so on the host that word must be
        # the readyResult field of the same object, as it is in retail RAM.
        source, count = re.subn(r'extern\s+int\s+g_CdRomCmdTimeout\b[^;]*;',
            "#define g_CdRomCmdTimeout "
            "(*(int *)&g_CdRomEventCommandState.command.read.readyResult)", source)
        self.assertEqual(count, 1)
        harness = r'''
#include <assert.h>
#include <stdint.h>
#define memcpy __builtin_memcpy
#define memcmp __builtin_memcmp
#define memset __builtin_memset
''' + source + r'''
CdRomEventCommandState g_CdRomEventCommandState;
#define state g_CdRomEventCommandState
int g_CdRomCmdLongTimeoutTable[256];
static int commandValue, statusValue, hasParam, outcome, flushes, copies, sends;
static unsigned char parameters[4] = {0x12, 0x34, 0x56, 0x78};
void CD_flush(void) {
    assert(flushes++ == 0 && copies == 0 && sends == 0);
    assert(state.pendingCommand == 0xCC);
}
void parcpy(void *dst, const void *src) {
    assert(flushes == 1 && copies++ == 0 && sends == 0);
    assert(dst == state.pendingParamBytes && src == parameters);
    assert(state.pendingCommand == commandValue);
    memcpy(dst, src, 4);
}
int CD_cw(int command, void *param, u8 *a, int b) {
    int rewritten = (commandValue == 7 && statusValue == 1) ||
                    (commandValue == 8 && statusValue != 1);
    assert(flushes == 1 && copies == hasParam && sends++ == 0);
    assert(command == (rewritten ? 1 : commandValue));
    assert(a == 0 && b == 1);
    assert(param == ((!rewritten && hasParam) ? state.pendingParamBytes : 0));
    assert(state.command.read.readyResult ==
           (g_CdRomCmdLongTimeoutTable[commandValue] ? 960 : 30));
    assert(state.command.retryAttempts == 0);
    assert(state.command.read.lastCommand == 0xCC);
    if (hasParam) assert(memcmp(state.pendingParamBytes, parameters, 4) == 0);
    /* The sender's post-call path must reload the command and clear both
       timeout words on failure, even when the low-level call changed them. */
    state.pendingCommand = 0x5A;
    state.command.read.readyResult = 111;
    state.command.retryAttempts = 222;
    return outcome;
}
int main(void) {
    const int outcomes[] = {0, 1, -1, 256};
    int c, s, p, t, o;
    for (c = 0; c < 256; ++c) for (s = 0; s < 3; ++s)
    for (p = 0; p < 2; ++p) for (t = 0; t < 2; ++t)
    for (o = 0; o < 4; ++o) {
        int result;
        memset(&state, 0xCC, sizeof(state));
        commandValue = c; statusValue = s; hasParam = p; outcome = outcomes[o];
        flushes = copies = sends = 0;
        state.command.read.eventFlags.bit1 = s;
        g_CdRomCmdLongTimeoutTable[c] = t ? -7 : 0;
        result = LIBDS_DSSYS_1_text_368(c, p ? parameters : 0);
        assert(result == (outcome == 0));
        assert(flushes == 1 && copies == p && sends == 1);
        assert(state.command.read.lastCommand == (outcome ? 0xCC : 0x5A));
        assert(state.command.read.readyResult == (outcome ? 0 : 111));
        assert(state.command.retryAttempts == (outcome ? 0 : 222));
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "cd-send-command"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
