#include "pe1/room_m256_swing.h"
#include "pe1/pm.h"
#include "../room_lib/room_lib.h"

void RoomM256_SwingNop0(void) {
}



void RoomM256_SwingInit(PmSlotHeader *raw) {
    RoomM256Swing *arg0=(void *)raw;
    arg0->cycle.callback = RoomM256_SwingAwaitMode;
    arg0->cycle.started = 0;
    arg0->cycle.signal = 0;
    arg0->cycle.segment = 0;
    arg0->cycle.angles[0] = 0;
    arg0->cycle.angles[3] = 0;
}

int RoomM256_SwingConfigure(PmSlotHeader *raw, int arg1, int arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5) {
    RoomM256Swing *arg0=(void *)raw;
    switch ((unsigned int)arg2) {
    case 0x19:
        if (arg1 != 1) {
            return 0;
        }
        arg0->cycle.signal = (int *)arg3;
        *(int *)arg3 = arg1;
        break;
    case 0x100:
        arg0->cycle.angles[1] = arg3;
        arg0->cycle.angles[2] = arg4;
        break;
    case 0x101:
        arg0->cycle.durations[0] = arg3;
        arg0->cycle.durations[1] = arg4;
        arg0->cycle.durations[2] = arg5;
        break;
    default:
        return 0;
    }
    return 0;
}

int RoomM256_SwingNop3(PmSlotHeader *unused) {
    return 0;
}

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM256Swing,cycle.callback)==PE1_OFFSETOF(RoomObj,callback),room_m256_native_callback_layout);
PmCommand g_RoomM256SwingClass={RoomM256_SwingNop0,RoomM256_SwingInit,RoomM256_SwingConfigure,RoomM256_SwingNop3,RoomLib_UpdateCallback,RoomM256_SwingRelease,RoomM256_SwingNop6};
