/*
 * The scene script's argument entry: stores its three arguments for the
 * room's effects and returns them. Thirty-five rooms and scenes link it as
 * the function after their last effect module (the orbit flare, the homing
 * flash or the sound burst); the words are the room's
 * (g_RoomScriptArgs, pe1/room_module.h).
 */

#include "pe1/room_module.h"

int g_RoomScriptArgs[3];

intptr_t RoomLib_SetArgs3(int unused, intptr_t a, intptr_t b, intptr_t c) {
    (void)unused;
    int *args = g_RoomScriptArgs;

    *args = (s32)(u32)a;
    g_RoomScriptArgs[1] = (s32)(u32)b;
    g_RoomScriptArgs[2] = (s32)(u32)c;
    return (intptr_t)args;
}
