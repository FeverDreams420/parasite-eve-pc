/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* Psy-Q LIBGPU SYS.OBJ, part 3 of 11: GetDispEnv, GetODE, SetTexWindow,
 * SetDrawArea, SetDrawOffset, SetPriority, SetDrawStp, SetDrawMode,
 * SetDrawEnv and the private Gpu_SetDrawEnvBack. */

#include "common.h"
#include "pe1/draw_area.h"
#include "pe1/gpu_callbacks.h"
#include "pe1/gpu_command_builders.h"
#include "pe1/gpu_state.h"
#include "pe1/psyq_gpu.h"

#include "sys_3_externs.h"

void *GetDispEnv(void *arg0) {
    void *(*fn)(void *, const void *, unsigned int);

    fn = memcpy;
    fn(arg0, &g_GpuActiveDispEnv, sizeof(DISPENV));
    return arg0;
}

int GetODE(void) {
    return g_GpuCallbacks->callback() < 0;
}

typedef signed short s16_1;

typedef struct {
    s16_1 x;
    s16_1 y;
} Point;

void SetTexWindow(GpuCmdPacket *arg0, int arg1) {
    arg0->u0.head.code = 2;
    arg0->field4 = Gpu_BuildTexWindowCmd((GpuTextureWindowRectBytes *)arg1);
    arg0->field8 = 0;
}

void SetDrawArea(GpuCmdPacket *arg0, DrawAreaRect *arg1) {
    arg0->u0.head.code = 2;
    arg0->field4 = Gpu_BuildDrawAreaTopLeftCmd(arg1->x, arg1->y);
    arg0->field8 = Gpu_BuildDrawAreaBottomRightCmd((s16)(arg1->x + arg1->w - 1), (s16)(arg1->y + arg1->h - 1));
}

void SetDrawOffset(GpuCmdPacket *arg0, Point *arg1) {
    arg0->u0.head.code = 2;
    arg0->field4 = Gpu_BuildDrawOffsetCmd(arg1->x, arg1->y);
    arg0->field8 = 0;
}

void SetPriority(GpuCmdPacket *arg0, int arg1, int arg2) {
    arg0->u0.head.code = 2;
    arg0->field4 = (arg1 ? 0xE6000002 : 0xE6000000) | (arg2 != 0);
    arg0->field8 = 0;
}

void SetDrawStp(GpuCmdPacket *arg0, int arg1) {
    arg0->u0.head.code = 2;
    arg0->field4 = arg1 ? 0xE6000001 : 0xE6000000;
    arg0->field8 = 0;
}

void SetDrawMode(GpuCmdPacket *arg0, int arg1, int arg2, int arg3, int arg4) {
    arg0->u0.head.code = 2;
    arg0->field4 = Gpu_BuildDrawModeCmd(arg1, arg2, arg3 & 0xFFFF);
    arg0->field8 = Gpu_BuildTexWindowCmd((GpuTextureWindowRectBytes *)arg4);
}

void SetDrawEnv(DR_ENV *packet, DRAWENV *env)
{
    RECT rect;
    unsigned int *words = (unsigned int *)packet;
    int n;
    words[1] = Gpu_BuildDrawAreaTopLeftCmd(env->clip.x, env->clip.y);
    words[2] = Gpu_BuildDrawAreaBottomRightCmd(
        (short)(env->clip.w + env->clip.x - 1), (short)(env->clip.y + env->clip.h - 1));
    words[3] = Gpu_BuildDrawOffsetCmd(env->ofs[0], env->ofs[1]);
    words[4] = Gpu_BuildDrawModeCmd(env->dfe, env->dtd, env->tpage);
    words[5] = Gpu_BuildTexWindowCmd((GpuTextureWindowRectBytes *)&env->tw);
    words[6] = 0xE6000000;
    n = 7;
    if (env->isbg) {
        rect.x = env->clip.x;
        rect.y = env->clip.y;
        rect.w = env->clip.w;
        rect.h = env->clip.h;
        rect.w = rect.w < 0 ? 0 : rect.w > g_GpuSystemState.width - 1 ? g_GpuSystemState.width - 1 : rect.w;
        rect.h = rect.h < 0 ? 0 : rect.h > g_GpuSystemState.height - 1 ? g_GpuSystemState.height - 1 : rect.h;
        rect.x -= env->ofs[0];
        rect.y -= env->ofs[1];
        words[n++] = 0x60000000 | (env->b0 << 16) | (env->g0 << 8) | env->r0;
        words[n++] = *(unsigned int *)&rect.x;
        words[n++] = *(unsigned int *)&rect.w;
    }
    ((unsigned char *)words)[3] = n - 1;
}

void Gpu_SetDrawEnvBack(DR_ENV *packet, DRAWENV *env)
{
    RECT rect;
    unsigned int *words = (unsigned int *)packet;
    int n;
    words[1] = Gpu_BuildDrawAreaTopLeftCmd(env->clip.x, env->clip.y);
    words[2] = Gpu_BuildDrawAreaBottomRightCmd(
        (short)(env->clip.w + env->clip.x - 1), (short)(env->clip.y + env->clip.h - 1));
    words[3] = Gpu_BuildDrawOffsetCmd(env->ofs[0], env->ofs[1]);
    words[4] = Gpu_BuildDrawModeCmd(env->dfe, env->dtd, env->tpage);
    words[5] = Gpu_BuildTexWindowCmd((GpuTextureWindowRectBytes *)&env->tw);
    words[6] = 0xE6000000;
    n = 7;
    if (env->isbg) {
        rect.x = env->clip.x;
        rect.y = env->clip.y;
        rect.w = env->clip.w;
        rect.h = env->clip.h;
        rect.w = rect.w < 0 ? 0 : rect.w > g_GpuSystemState.width - 1 ? g_GpuSystemState.width - 1 : rect.w;
        rect.h = rect.h < 0 ? 0 : rect.h > g_GpuSystemState.height - 1 ? g_GpuSystemState.height - 1 : rect.h;
        if ((rect.x & 63) || (rect.w & 63)) {
            rect.x -= env->ofs[0];
            rect.y -= env->ofs[1];
            words[n++] = 0x60000000 | (env->b0 << 16) | (env->g0 << 8) | env->r0;
            words[n++] = *(unsigned int *)&rect.x;
            words[n++] = *(unsigned int *)&rect.w;
        } else {
            words[n++] = 0x02000000 | (env->b0 << 16) | (env->g0 << 8) | env->r0;
            words[n++] = *(unsigned int *)&rect.x;
            words[n++] = *(unsigned int *)&rect.w;
        }
    }
    ((unsigned char *)words)[3] = n - 1;
}
