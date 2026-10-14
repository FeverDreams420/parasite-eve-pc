#include "pe1/akao.h"
#include "pe1/akao_send.h"
#include "pe1/menu_sound.h"
#include "pe1/scene_assets.h"

int Akao_SendTableCommand(void *arg0, int arg1, int arg2, int arg3, int arg4) {
    void *entry = Asset_FindTable2CByU16Key(arg0, arg1);
    int ret;

    if (entry != 0) {
        ret = Akao_Cmd_24(entry, arg2, arg3, arg4);
    } else {
        ret = -1;
    }
    return ret;
}

void Menu_PlayConfirmSound(void) {
    if (g_AkaoBgmHandle[0] != 0) {
        Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44C, 0x100, 0x80, 0x7F);
    }
}

void Menu_PlayCancelSound(void) {
    if (g_AkaoBgmHandle[0] != 0) {
        Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44D, 0x100, 0x80, 0x7F);
    }
}

void Menu_PlayMoveSound(void) {
    if (g_AkaoBgmHandle[0] != 0) {
        Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44E, 0x100, 0x80, 0x7F);
    }
}

void Menu_PlayErrorSound(void) {
    if (g_AkaoBgmHandle[0] != 0) {
        Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44F, 0x100, 0x80, 0x7F);
    }
}
